/**
 * Copyright 2021 Bruce Ide
 *
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
 * This is a registry that applications in the cluster can query for information
 * about jobs. The job object can be retrieved by JobID or by a stream ID.
 */

#pragma once

#define ZMQ_BUILD_DRAFT_API
#define ZMQ_CPP11
#define ZMQ_HAVE_POLLER

#include <atomic>
#include "Job.h"
#include <json_oarchive.hpp>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <zmq.hpp>

namespace fr::media2::demos {

  class JobRegistry {
  public:
    /**
     * Constructor takes the tcp address to listen on for queries by
     * job ID and the tcp address to listen on for queries by stream
     * ID.
     */
    JobRegistry(std::string jobAddress, std::string streamAddress);
    ~JobRegistry();

    // Add a job to the registry
    void add(std::shared_ptr<Job>);
    // Shuts down the registry
    void shutdown();

    // In addition to ZMQ query, you can also query the object
    // locally. These can return an empty shared ptr, so check for that.

    std::shared_ptr<Job> byJobId(std::string id);
    std::shared_ptr<Job> byStreamId(std::string id);

  private:
    std::unordered_map<std::string, std::shared_ptr<Job>> jobs;
    std::unordered_map<std::string, std::shared_ptr<Job>> streamIds;
    std::mutex jobsMutex;
    std::mutex byStreamMutex;
    std::thread jobThread;
    std::thread streamThread;
    std::atomic<bool> shutdownFlag;

    // Porcess by takes an address and a query function and processes requests
    // from zmq.
    void processBy(std::string address, std::function<std::shared_ptr<Job>(std::string id)> queryBy);
  };
  
}
