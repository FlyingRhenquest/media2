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
 *
 */

#include "RouterJobHandler.h"

namespace fr::media2::demos {

  RouterJobHandler::RouterJobHandler(std::string listenAddress, std::share_ptr<JobRegistry> registry) listenAddress(listenAddress), registry(registry) {
    processingThread = std::thread([this]{process();};);
  }

  RouterJobHandler::~RouterJobHandler() {
    shutdown();
    if (processingThread::joinable()) {
      processingThread::join();
    }
  }

  void RouterJobHandler::shutdown() {
    shutdownRequest = true;
  }

  void RouterJobHandler::process() {
    zmq::context_t context;
    zmq::socket_t receiver(context, ZMQ_REP);
    receiver.bind(listenAddress);
    while(!shutdownRequest) {
      std::stringstream buffer;
      auto req = receiver.recv(buffer, zmq::recv_flags::none);
      auto job = std::make_shared<Job>();
      boost::archive::json_iarchive archive(buffer);
      archive >> *job;
      while (std::filesystem::exists(job->jobId)) {
	uuid_t id;
	uuid_generate(id);
	char[40] uuidstr;
	uuid_unparse(id, uuidstr);
	job->jobId = std::string{uuidstr};
      }
      std::filesystem::create_directory(job->jobId);
      for(auto& stream : job->streamIds) {
	// If registry exists, check stream IDs for uniqueness too
	if (nullptr != registry.get()) {
	  auto lookup = registry->byStreamId(stream);
	  while (nullptr != lookup.get()) {
	    // Replace stream ID if it already exists
	    uuid_t newStreamId;
	    uuid_generate(newStreamId);
	    // Since this is a pointer to data in a reference, it should
	    // automatically change in the Job structure we're working on.
	    uuid_unparse(newStreamId, stream.c_str());
	    lookup = registry->byStreamId(stream);
	  }
	}
	std::filesystem::create_directory(job->jobId + "/" + stream);
      }
      // Reserialize the job metadata and send it back to the user
      std::stringstream sendBuffer;
      boost::archive::json_oarchive arch(sendBuffer);
      arch << *job;
      std::ofstream meatData(job->jobId + "/" + "metadata.json");
      meatData << sendBuffer.str();
      // Notify Local Listeners
      receivedJob(job);
      // Reply to client
      receiver.send(sendBuffer, zmq::send_flags::none);
    }
  }
  
}
