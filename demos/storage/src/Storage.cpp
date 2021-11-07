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

#include <Storage.h>
#include <iostream>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace fr::media2::demos {

  Storage::Storage(std::string storageServiceAddress, std::string queryServiceAddress, int nthreads) :
    queryServiceAddress(queryServiceAddress) {

    std::cout << "Starting storage service on " << storageServiceAddress << std::endl;
    receiverThread = std::thread([this, storageServiceAddress]{ receive(storageServiceAddress); });
    for (int i = 0; i < nthreads; ++i) {
      storagePool.push_back(std::thread([this]{process();}));
    }
  }

  Storage::~Storage() {
    shutdown();
    // Wake workers if they're restin'
    workNotification.notify_all();
    join();
  }

  void Storage::shutdown() {
    shutdownRequest = true;
    if (nullptr != subscriber.get()) {
      subscriber->close();
      subscriber->join();
    }
  }

  void Storage::join() {
    if (receiverThread.joinable()) {
      receiverThread.join();
    }
    for (auto& worker : storagePool) {
      if (worker.joinable()) {
	worker.join();
      }
    }
    storagePool.clear();
  }

  void Storage::add(std::stringstream &buffer, uuid_t streamId) {
    auto task = std::make_unique<Tasks>();
    task->buffer = std::make_shared<std::stringstream>();
    task->buffer->swap(buffer);
    uuid_copy(task->id, streamId);
    
    std::lock_guard<std::mutex> lock(taskMutex);
    tasks.push_back(std::move(task));
    moreWork = true;
    workNotification.notify_all();
  }

  std::string Storage::idStr(uuid_t id) {
    char idchars[40];
    uuid_unparse(id, idchars);
    return std::string(idchars);
  }

  std::shared_ptr<Job> Storage::jobServiceLookup(std::string id) {
    std::cout << "Looking up " << id << std::endl;
    std::shared_ptr<Job> resp;
      // Query ZMQ
      zmq::context_t context;
      zmq::socket_t receiver(context, zmq::socket_type::req);
      receiver.connect(queryServiceAddress);
      std::stringstream buffer;
      zmq::message_t outgoing(id);
      zmq::message_t incoming;
      auto bytes = receiver.send(outgoing, zmq::send_flags::none);
      bytes = receiver.recv(incoming, zmq::recv_flags::none);
      buffer << incoming.to_string();
      std::cout << buffer.str() << std::endl;
      cereal::JSONInputArchive arch(buffer);
      arch >> resp;
      return resp;
  }
  
  std::shared_ptr<Job> Storage::queryJob(uuid_t uuid) {
    std::shared_ptr<Job> ret;
    std::string id = idStr(uuid);
    std::lock_guard<std::mutex> lock(cacheMutex);
    if (jobCache.contains(id)) {
      ret = jobCache[id];
    } else {
      ret = jobServiceLookup(id);
      jobCache[id] = ret;
    }
    return ret;
  }

  void Storage::receive(std::string storageServiceAddress) {
    subscriber = std::make_shared<ZmqSegmentSubscriber>(storageServiceAddress);
    subscriber->receivedSegment.connect([this](std::stringstream& buffer, uuid_t uuid) {
      this->add(buffer, uuid);
    });
    subscriber->process();
  }

  void Storage::process() {
    std::mutex cvWaiter; // Waits on workNotification

    // If we shut down, we want to process remaining tasks before
    // exiting.
    while(!shutdownRequest || !tasks.empty()) {
      std::unique_ptr<Tasks> task;
      {
	std::lock_guard<std::mutex> lock(taskMutex);
	if (!tasks.empty()) {
	  task = std::move(tasks.front());
	  tasks.pop_front();
	}
      }
      if (nullptr != task.get()) {
	// Get job for directory to store in
	while(nullptr == task->metadata.get()) {
	  task->metadata = queryJob(task->id);
	}
	if (!task->metadata->jobId.empty()) {
	  // I have to unpack the segment to get its
	  // timestamp (This seems like something I should
	  // send on the wire so I don't have to mess with that)
	  auto ptr = std::make_shared<Segment>();
	  boost::archive::binary_iarchive ar(*task->buffer);
	  ar >> (*ptr);
	  
	  std::filesystem::path writeDir = task->metadata->jobRoot() / idStr(task->id);
	  std::string filename{"/"};
	  filename.append(std::to_string(ptr->dts));
	  std::cout << "Writing " << writeDir.string() + filename << std::endl;
	  std::ofstream seg(writeDir.string() + filename);
	  seg << task->buffer->str();
	}
      } else {
	// No work ATM
	std::unique_lock<std::mutex> lock(cvWaiter);
	moreWork = false;
	workNotification.wait(lock, [this]{return moreWork;});
      }
    } 
     
  }

}

