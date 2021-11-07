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
 * Receives incoming segments and stores them to disk. There are two
 * components to this; a receiver receives the segment buffer and
 * job ID and stores it for later processing. A threadpool takes each
 * segment, decompresses it to retrieve some data from the segment
 * and then stores the segment buffer to the directories made by the
 * router.
 */

#pragma once

#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/json.hpp>
#include <Job.h> // From router demo
#include <atomic>
#include <fr/media2.h>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <thread>
#include <uuid.h>
#include <memory>
#include <mutex>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <zmq.hpp>

namespace fr::media2::demos {

  class Storage {
  public:
    Storage(std::string storageServiceAddress, std::string queryServiceAddress, int nthreads = 4);
    ~Storage();

    void shutdown();
    void join();
    
    // Add another segment to process
    void add(std::stringstream& buffer, uuid_t streamId);
    
  private:

    struct Tasks {
      std::shared_ptr<std::stringstream> buffer;
      uuid_t id;
      std::shared_ptr<Job> metadata;
    };

    std::string queryServiceAddress;
      
    std::atomic<bool> shutdownRequest;
    
    std::thread receiverThread;
    std::vector<std::thread> storagePool;
    
    // Store jobs in cache so I don't have to look them up
    // for every segment.
    std::unordered_map<std::string, std::shared_ptr<Job>> jobCache;
    std::deque<std::unique_ptr<Tasks>> tasks;
    std::mutex taskMutex;
    std::mutex cacheMutex;

    std::shared_ptr<::fr::media2::ZmqSegmentSubscriber> subscriber;

    // Gets set false by workers if tasks are empty
    // Gets set true by receiverThread when more work
    // arrives.
    bool moreWork;

    // Worker threads can wait on this
    std::condition_variable workNotification;

    // Queries zmq service for ID
    std::shared_ptr<Job> jobServiceLookup(std::string id);
    
    // Get stringified uuid
    std::string idStr(uuid_t id);
    // Query job by stream ID
    std::shared_ptr<Job> queryJob(uuid_t id);
    // Run receiver thread
    void receive(std::string storageServiceAddress); 
    // Run by worker threads
    void process();
    
  };

}
