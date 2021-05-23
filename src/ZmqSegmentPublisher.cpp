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

  ZmqSegmentPublisher::~ZmqSegmentPublisher() {}

  void ZmqSegmentPublisher::process(const Segment::pointer& segment) {
    std::stringstream buffer;
    boost::archive::binary_oarchive ar(buffer);
    ar << *segment;
    zmq::multipart_t multimessage;
    // I don't want to have to deserialize my segment just to get the job ID,
    // so send it first.
    multimessage.pushmem(segment->jobId, sizeof(uuid_t));
    multimessage.pushstr(buffer.str());
    multimessage.send(publisher);
  }

  
}
