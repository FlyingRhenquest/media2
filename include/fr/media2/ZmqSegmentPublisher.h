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
 * Uses a zmq pub-sub to send serialized segment buffers somewhere.
 * I might eventually roll a more generic transport, since I'm just
 * sending byte buffers, so really just need send(pointer, size)
 * for this to work.
 */

#pragma once

#define ZMQ_BUILD_DRAFT_API
#define ZMQ_CPP11
#define ZMQ_HAVE_POLLER

#include <fr/media2/Stream.h>
#include <fr/media2/StreamData.h>
#include <fr/media2/SegmentSubscriber.h>
#include <string>
#include <uuid.h>
#include <zmq.hpp>

namespace fr::media2 {

  /**
   * You should create ONE segment sender per stream. If you try to funnel
   * two streams into a segment sender, you're gonna have a bad time.
   * This is more or less (ok, more) stateless and will just send
   * whatever you shove into it. You can send an empty segment to
   * indicate end of stream if you want to. That's probably what I'll
   * end up doing in my demo transport.
   *
   * Also, don't try to reuse the segment publisher for different streams,
   * just make a new one for each stream you want to send.
   */
  
  class ZmqSegmentPublisher : public SegmentSubscriber {
  public:
    ZmqSegmentPublisher(std::string address);
    // Forces publisher to use supplied uuid
    ZmqSegmentPublisher(std::string address, uuid_t id);
    ~ZmqSegmentPublisher();

    // This can also be called manually to send a segment
    void process(const Segment::pointer&, StreamData::pointer) override;
    // Or via stringstream/uuid
    void process(std::stringstream&, uuid_t, AVMediaType mt = AVMEDIA_TYPE_UNKNOWN, int width = 0, int height = 0);

    // Set/Reset UUID -- forces publisher to use this uuid
    void setUuid(uuid_t);
    
  protected:
    std::string remoteAddress;
    uuid_t jobId;
    zmq::context_t context;
    zmq::socket_t publisher;
  };

}
