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
 * This objects handles incoming jobs. It sets up a directory and stores the job
 * metadata in json format in the directory.
 *
 */

#pragma once

#define ZMQ_BUILD_DRAFT_API
#define ZMQ_CPP11
#define ZMQ_HAVE_POLLER

#include <atomic>
#include <boost/signals2.hpp>
#include "Job.h"
#include "JobRegistry.h"
#include <filesystem>
#include <memory>
#include <mutex>
#include <fstream>
#include <string>
#include <thread>
#include <uuid.h>
#include <vector>
#include <zmq.hpp>

namespace fr::media2::demos {

  /**
   * Starts a thread and listens for jobs from the client with zmq.
   * Client requests with Job structure. Server checks to insure
   * that the job ID is not already used and sends the job back as
   * a reply with the Job ID replaced if it is.
   *
   * This object isn't really geared toward a high-traffic situation.
   */
  
  class RouterJobHandler {
  public:
    RouterJobHandler(std::string listenAddress, std::shared_ptr<JobRegistry> registry);
    ~RouterJobHandler();

    // A signal to connect to be notified of jobs that get created
    boost::signals2::signal<void(std::shared_ptr<Job>)> receivedJob;

    void shutdown();
    
  protected:
    std::atomic<bool> shutdownRequest = false;
    std::thread processingThread;
    std::shared_ptr<JobRegistry> registry;
    void process(std::string);
  };
  
}
