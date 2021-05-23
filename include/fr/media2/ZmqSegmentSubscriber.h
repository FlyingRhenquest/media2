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
 * Subscribes to a ZmqSegmentPublisher. This object is not very smart, it
 * just receives segments and notifiers listeners that it has a buffer.
 * It's up to subscribers to do something with the message.
 *
 * You can receive more than one stream with this object.
 */

#pragma once

#define ZMQ_BUILD_DRAFT_API
#define ZMQ_CPP11
#define ZMQ_HAVE_POLLER

#include <atomic>
#include <boost/signals2.hpp>
#include <sstream>
#include <string>
#include <thread>
#include <uuid.h>
#include <zmq.hpp>
#include <zmq_addon.hpp>

namespace fr::media2 {

  class ZmqSegmentSubscriber {
  public:
    ZmqSegmentSubscriber(std::string listenAddress);
    ~ZmqSegmentSubscriber();

    // Kicks off a thread that runs until close gets called or the object
    // is destroyed.
    void process();
    // You don't necessarily ever need to call close/join -- you can just let
    // this run and receive segments forever, if you want to.
    
    // Shuts down this object
    void close();
    // Rejoins the working thread
    void join();

    // Subscribe to this signal to receive segments.
    boost::signals2::signal<void(std::stringstream&, uuid_t)> receivedSegment;

  protected:
    std::atomic<bool> shutdownPlox = false;
    std::stringstream buffer;
    zmq::context_t context;
    zmq::socket_t socket;
    std::thread processingThread;
  
    void processPrivately();
    
  };
  
}
