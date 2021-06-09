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
#include <cereal/archives/json.hpp>
#include <zmq.hpp>
#include <iostream>

namespace fr::media2::demos {

  RouterJobHandler::RouterJobHandler(std::string listenAddress, std::shared_ptr<JobRegistry> registry) :
    registry(registry) {
    std::cout << "Starting job handler on " << listenAddress << std::endl;
    processingThread = std::thread([this, listenAddress]{process(listenAddress);});
  }

  RouterJobHandler::~RouterJobHandler() {
    shutdown();
    if (processingThread.joinable()) {
      processingThread.join();
    }
  }

  void RouterJobHandler::shutdown() {
    shutdownRequest = true;
  }

  void RouterJobHandler::process(std::string address) {
    zmq::context_t context;
    zmq::socket_t receiver(context, zmq::socket_type::rep);
    receiver.bind(address);
    while(!shutdownRequest) {
      std::stringstream buffer;
      zmq::message_t incomingMsg;
      auto rep = receiver.recv(incomingMsg, zmq::recv_flags::none);
      buffer << incomingMsg.to_string();
      Job jerb;
      {
	cereal::JSONInputArchive archive(buffer);
	archive >> jerb;
      }
      std::cout << "Received request for " << buffer.str() << std::endl;
      while (std::filesystem::exists(jerb.jobId)) {
	uuid_t id;
	uuid_generate(id);
	char uuidstr[40];
	uuid_unparse(id, uuidstr);
	jerb.jobId = std::string{uuidstr};
      }
      std::filesystem::current_path(std::filesystem::temp_directory_path());
      auto path = std::filesystem::current_path();
      auto jobPath = path / jerb.jobId;
      std::cout << "Creating " << jobPath.string() << std::endl;
      std::filesystem::create_directory(jobPath);
      for(auto& stream : jerb.streamIds) {
	// If registry exists, check stream IDs for uniqueness too
	if (nullptr != registry.get()) {
	  auto lookup = registry->byStreamId(stream);
	  while (nullptr != lookup.get()) {
	    // Replace stream ID if it already exists
	    uuid_t newStreamId;
	    uuid_generate(newStreamId);
	    char uuidstr[40];
	    uuid_unparse(newStreamId, uuidstr);
	    stream = uuidstr;
	    lookup = registry->byStreamId(stream);
	  }
	}
	auto streamPath = jobPath / stream;
	std::cout << "Creating " << streamPath.string() << std::endl;
	std::filesystem::create_directory(streamPath);
      }
      // Reserialize the job metadata and send it back to the user
      std::stringstream sendBuffer;
      {
	cereal::JSONOutputArchive arch(sendBuffer);
	arch << jerb;
      }
      std::ofstream meatData(jerb.jobId + "/" + "metadata.json");
      meatData << sendBuffer.str() << std::endl;
      // Notify Local Listeners
      auto tempJob = std::make_shared<Job>();
      (*tempJob) = jerb;
      std::cout << "Signalling " << tempJob->jobId << std::endl;
      receivedJob(tempJob); 
      // Reply to client
      zmq::message_t outgoingMsg(sendBuffer.str());
      receiver.send(outgoingMsg, zmq::send_flags::none);
    }
  }
  
}
