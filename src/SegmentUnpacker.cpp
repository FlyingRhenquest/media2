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

#include <fr/media2/SegmentUnpacker.h>

namespace fr::media2 {

  SegmentUnpacker::SegmentUnpacker(int nThreads, std::function<void(Stream::pointer)> setupStream) :
    setupStream{setupStream},
    cache{std::make_shared<StreamCache>()}
  {
    // Start thread pool
    for (int i = 0; i < nThreads; ++i) {
      workers.push_back(std::make_shared<std::thread>([this]{this->doSomeWork();}));
    }
  }
    
  SegmentUnpacker::SegmentUnpacker(int nThreads, std::shared_ptr<StreamCache> cache,
				   std::function<void(Stream::pointer)> setupStream) :
    setupStream{setupStream}, cache{cache} {
    // Start thread pool
    for (int i = 0; i < nThreads; ++i) {
      workers.push_back(std::make_shared<std::thread>([this]{this->doSomeWork();}));
    }
  }
  
  SegmentUnpacker::~SegmentUnpacker() { close(); }

  void SegmentUnpacker::close() {
    unsubscribe();
    shutdownFlag = true;
    while (!work.empty()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    for (auto thread : workers) {
      if (thread->joinable()) {
	thread->join();
      }
    }
    cache.reset();
    workers.clear();
  }
  
  void SegmentUnpacker::subscribe(ZmqSegmentSubscriber* source) {
    auto sub = source->receivedSegment.connect([this](std::stringstream &buffer, uuid_t /* notused */){
      this->receive(buffer);
    });
    subscriptions.push_back(sub);
  }

  void SegmentUnpacker::unsubscribe() {
    for (auto sub : subscriptions) {
      sub.disconnect();
    }
  }
  
  void SegmentUnpacker::receive(std::stringstream& buffer) {
    auto ptr = segFrom(buffer);
    receive(std::move(ptr));
  }

  void SegmentUnpacker::receive(std::unique_ptr<Segment> seg) {
    std::lock_guard<std::mutex> lock(workMutex);
    work.push_back(std::move(seg));
  }

  std::unique_ptr<Segment> SegmentUnpacker::segFrom(std::stringstream& buffer) {
    auto ptr = std::make_unique<Segment>();
    boost::archive::binary_iarchive ar(buffer);
    ar >> *ptr;
    return ptr;
  }

  void SegmentUnpacker::doSomeWork() {
    while (!shutdownFlag || work.size() > 0) {
      if (work.empty()) {
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
      } else {
	std::unique_ptr<Segment> seg;
	{
	  std::lock_guard<std::mutex> lock(workMutex);
	  // Doublecheck work empty now that it's locked
	  if (!work.empty()) {
	    seg = std::move(work.front());
	    work.pop_front();
	  }
	  // Reliquish lock ASAP
	}
	unpack(std::move(seg));	
      }
    }
  }
  
  void SegmentUnpacker::unpack(std::unique_ptr<Segment> seg) {
    auto stream = cache->get(seg->jobId);
    if (nullptr == stream.get()) {
      stream = cache->get(seg.get());
      setupStream(stream);
    }

    for (const Packet::pointer& packet : seg->packets) {
      stream->forward(packet);
    }
  }
  
}
