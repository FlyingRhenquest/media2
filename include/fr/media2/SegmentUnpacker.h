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
 * Unpacks a segment. This will attempt to retrieve a stream from the stream
 * cache. If this fails, the stream subscriptions will have to be set up again.
 * This can handle segments from multiple sources. It will start a thread
 * for each stream it's handling. Oh, it can also subscribe to multiple
 * SegmentSubscribers.
 */

#pragma once

#include <atomic>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/signals2.hpp>
#include <chrono>
#include <deque>
#include <functional>
#include <fr/media2/Serialization.h>
#include <fr/media2/Segment.h>
#include <fr/media2/Stream.h>
#include <fr/media2/StreamCache.h>
#include <fr/media2/ZmqSegmentSubscriber.h>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <memory>
#include <vector>

namespace fr::media2 {

  class SegmentUnpacker {
  public:
    // Pass a lambda to set up subscriptions if we get a cache
    // miss.  
    SegmentUnpacker(int nThreads, std::function<void(Stream::pointer)>);
    // You can also pass in your own cache if you want to
    SegmentUnpacker(int nThreads, std::shared_ptr<StreamCache>, std::function<void(Stream::pointer)>);
    ~SegmentUnpacker();

    // Subscribe to a segment subscriber
    void subscribe(ZmqSegmentSubscriber*);
    void unsubscribe();

    // Shut down the unpacker -- unpacker will unsubscribe to new messages,
    // process any remaining work and shut down.
    void close();
    
    // receive a segment. You can manually feed serialzied segments to
    // this object this way too.
    void receive(std::stringstream &);
    // If you never serialized your segment or deserialized it elsewhere,
    // you can feed this object Segments too. This object takes
    // ownership of the pointer, so send a copy if you want to keep it.
    void receive(std::unique_ptr<Segment>);
  private:
    std::vector<std::shared_ptr<std::thread>> workers;
    std::mutex workMutex;
    
    std::atomic<bool> shutdownFlag = false;
    // Work queue
    std::deque<std::unique_ptr<Segment>> work;
    // If we get a cache miss, setupStream should be used to set
    // up subscribers to that stream. If it's not anything,
    // Segments will get unpacked but the packets will never
    // be forwarded anywhere.
    std::function<void(Stream::pointer stream)> setupStream;
    std::vector<boost::signals2::connection> subscriptions;
    std::shared_ptr<StreamCache> cache;

    std::unique_ptr<Segment> segFrom(std::stringstream&);
    // Runs in thread until done
    void doSomeWork();
    // Unpacks a segment
    void unpack(std::unique_ptr<Segment>);
  };
  
}
