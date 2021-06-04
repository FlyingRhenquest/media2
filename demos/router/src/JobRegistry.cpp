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

#include "JobRegistry.h"
#include <boost/serialization/vector.hpp>
#include <iostream>

namespace fr::media2::demos {

  JobRegistry::JobRegistry(std::string jobAddress, std::string streamAddress) {
    // Launch worker threads
    jobThread = std::thread([this, jobAddress]{processBy(jobAddress, [this](std::string id){return this->byJobId(id);});});
    streamThread = std::thread([this, streamAddress]{processBy(streamAddress, [this](std::string id){return this->byStreamId(id);});});
  }

  JobRegistry::~JobRegistry() {
    shutdown();
    if (jobThread.joinable()) {
      jobThread.join();
    }
    if (streamThread.joinable()) {
      streamThread.join();
    }
    jobs.clear();
    streamIds.clear();
  }

  void JobRegistry::add(std::shared_ptr<Job> job) {
    { // Scope to drop mutex quickly
      std::lock_guard<std::mutex> lock(jobsMutex);
      // Neither map should contain dups, so I'm not going to bother
      // checking.
      jobs[job->jobId] = job;
    }
    {
      std::lock_guard<std::mutex> lock(byStreamMutex);
      for (auto stream : job->streamIds) {
	streamIds[stream] = job;
      }
    }
  }

  void JobRegistry::shutdown() {
    shutdownFlag = true;
  }

  std::shared_ptr<Job> JobRegistry::byJobId(std::string id) {
    std::shared_ptr<Job> ret;
    std::lock_guard<std::mutex> lock(jobsMutex);
    if (jobs.contains(id)) {
      ret = jobs[id];
    }
    return ret;
  }

  std::shared_ptr<Job> JobRegistry::byStreamId(std::string id) {
    std::shared_ptr<Job> ret;
    std::lock_guard<std::mutex> lock(byStreamMutex);
    if (streamIds.contains(id)) {
      ret = streamIds[id];
    }
    return ret;
  }

  void JobRegistry::processBy(std::string address, std::function<std::shared_ptr<Job>(std::string id)> queryBy) {
    std::shared_ptr<Job> defaultResponse = std::make_shared<Job>();
    zmq::context_t context;
    zmq::socket_t receiver(context, zmq::socket_type::rep);
    std::cout << "starting processBy handler on " << address << std::endl;
    receiver.bind(address);
    while(!shutdownFlag) {
      std::shared_ptr<Job> resp;
      zmq::message_t msg;
      auto req = receiver.recv(msg, zmq::recv_flags::none);
      std::string id = msg.to_string();
      resp = queryBy(id);
      if (nullptr == resp.get()) {
	// Not found. Send empty job buffer instead.
	resp = defaultResponse;
      }
      std::stringstream buffer;
      boost::archive::json_oarchive arch(buffer);
      arch << *resp;
      zmq::message_t outgoing(buffer.str());
      receiver.send(outgoing, zmq::send_flags::none);
    }
  }

}
