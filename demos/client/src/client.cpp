/**
 * Copyright (C) Bruce Ide
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define ZMQ_BUILD_DRAFT_API
#define ZMQ_CPP11
#define ZMQ_HAVE_POLLER


#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <Job.h> // From router demo
#include <fr/media2.h>
#include <sstream>
#include <string>
#include <uuid.h>
#include <zmq.hpp>

int main(int argc, char *argv[]) {
  // Address to send RouterJobHandler notifications
  // Env: JOB_HANDLER_ADDRESS
  std::string jobHandlerAddress{"JOB_HANDLER_ADDRESS"};
  // Address to stream segments to
  // Env: INCOMING_SEGMENT_ADDRESS
  std::string incomingSegmentAddress{"INCOMING_SEGMENT_ADDRESS"};

  std::string errors;
  if (char *jha = getenv(jobHandlerAddress.c_str())) {
    jobHandlerAddress = jha;
  } else {
    errors.append(jobHandlerAddress);
  }
  if (char *isa = getenv(incomingSegmentAddress.c_str())) {
    incomingSegmentAddress = isa;
  } else {
    errors.append(errors.empty() ? incomingSegmentAddress : std::string{", "} + incomingSegmentAddress);
  }
  if (!errors.empty()) {
    std::string err("The following environment variables must be set up with zmq URIs prior to running this client: ");
    err.append(errors);
    throw std::runtime_error(errors);
  }
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " filename.mp4" << std::endl;
  } else {
    fr::media2::PacketReader reader(std::string{argv[1]});
    fr::media2::demos::Job jerb;
    jerb.filename = std::string{argv[1]};
    if (reader.videoStreams.size() > 0) {
      switch(reader.videoStreams[0]->data->parameters->width) {
      case 4096:
      case 3996:
	jerb.resolution = "4K";
	break;
      case 1920:
	jerb.resolution = "1080";
	break;
      case 1280:
	jerb.resolution = "720";
	break;
      default:
	jerb.resolution = "unknown";
	break;
      }
    } else {
      // Could just be an audio file or something
      jerb.resolution = "unknown";
    }
    // We're going to need some uuid strings for our stream
    // Retrospectively, I should have just written the router to take
    // a job ID when receiving streams and made a storage directory
    // for the stream if it didn't exist yet. Not gonna go back and
    // fix it for a demo, though.

    std::vector<std::string> uuids;

    for (int i = 0; i < reader.streams.size() + 1; ++i) {
      uuid_t jerbid;
      uuid_generate(jerbid);
      char uuidchars[40];
      uuid_unparse(jerbid, uuidchars);
      uuids.push_back(std::string{uuidchars});
    }
    // This is proposed job ID only, server can override it.
    jerb.jobId = uuids.back();
    uuids.pop_back();
    jerb.streamIds = uuids;

    std::stringstream buffer;
    {
      cereal::JSONOutputArchive ar(buffer);
      ar << jerb;
    }
    
    // OK! Let's do the server two-step!
    zmq::context_t context;
    zmq::socket_t jobSender(context, zmq::socket_type::req);

    zmq::message_t jobMsg(buffer.str());
    zmq::message_t jobResponse;
    jobSender.connect(jobHandlerAddress);
    jobSender.send(jobMsg, zmq::send_flags::none);
    auto resp = jobSender.recv(jobResponse, zmq::recv_flags::none);
    // Clear buffer
    std::stringstream respBuffer;
    respBuffer << jobResponse.to_string();
    {
      cereal::JSONInputArchive ar(buffer);
      ar >> jerb;
    }
    std::cout << "Sending " << jerb.filename << std::endl;
    std::cout << "Job ID: " << jerb.jobId << std::endl;
    std::cout << "Stream IDs: ";
    std::cout << "Json Jerb: " << buffer.str() << std::endl;
    for (auto id : jerb.streamIds) {
      std::cout << id << "  ";
    }
    std::cout << std::endl;

    // Set up segmenters and publishers
    std::vector<std::shared_ptr<fr::media2::Segmenter>> segmenters;
    std::vector<std::shared_ptr<fr::media2::ZmqSegmentPublisher>> publishers;
    for (auto stream : reader.streams) {
      // Get a uuid from our uuids
      std::string uuidstr = uuids.back();
      uuids.pop_back();
      uuid_t uuid;
      uuid_parse(uuidstr.c_str(), uuid);
      
      auto segmenter = std::make_shared<fr::media2::Segmenter>();
      auto publisher = std::make_shared<fr::media2::ZmqSegmentPublisher>(incomingSegmentAddress, uuid);
      segmenter->subscribe(stream);
      publisher->subscribe(segmenter.get());
      segmenters.push_back(segmenter);
      publishers.push_back(publisher);
    }
    // Send all packets
    reader.process();
    reader.join();

    // Flush segmenters
    for (auto segmenter : segmenters) {
      segmenter->flush();
    }
    // Clear segmenters and publishers
    segmenters.clear();
    publishers.clear();
    std::cout << "Sent." << std::endl;
  }
}
