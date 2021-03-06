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

#include <fr/media2/ZmqSegmentPublisher.h>
#include <zmq.h>
#include <zmq_addon.hpp>
#include <sstream>

namespace fr::media2 {

  ZmqSegmentPublisher::ZmqSegmentPublisher(std::string address) : publisher{context, zmq::socket_type::pub} {
    publisher.connect(address);
    uuid_clear(jobId);
  }

  ZmqSegmentPublisher::ZmqSegmentPublisher(std::string address, uuid_t id) : publisher{context, zmq::socket_type::pub} {
    publisher.connect(address);
    uuid_copy(jobId, id);
  }

  ZmqSegmentPublisher::~ZmqSegmentPublisher() {}

  void ZmqSegmentPublisher::process(const Segment::pointer& segment, StreamData::pointer stream) {
    std::stringstream buffer;
    boost::archive::binary_oarchive ar(buffer);
    ar << *segment;
    zmq::multipart_t multimessage;
    if (uuid_is_null(jobId)) {
      multimessage.addmem(segment->jobId, sizeof(uuid_t));
    } else {
      multimessage.addmem(jobId, sizeof(uuid_t));
    }
    multimessage.addmem(&stream->mediaType, sizeof(AVMediaType));
    multimessage.addmem(&stream->parameters->width, sizeof(int));
    multimessage.addmem(&stream->parameters->height, sizeof(int));
    multimessage.addstr(buffer.str());
    multimessage.send(publisher);
  }

  void ZmqSegmentPublisher::process(std::stringstream& buffer, uuid_t id, AVMediaType mt, int width, int height) {
    zmq::multipart_t multimessage;
    multimessage.addmem(id, sizeof(uuid_t));
    multimessage.addmem(&mt, sizeof(AVMediaType));
    multimessage.addmem(&width, sizeof(int));
    multimessage.addmem(&height, sizeof(int));
    multimessage.addstr(buffer.str());
    multimessage.send(publisher);
  }

  
}
