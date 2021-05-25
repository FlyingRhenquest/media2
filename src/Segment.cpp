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

#include <fr/media2/Segment.h>
#include <fr/media2/Packet.h>

namespace fr::media2 {

  Segment::Segment() {
    memset(jobId, '\0', sizeof(uuid_t));
    memset((void*)&parameters, '\0', sizeof(AVCodecParameters));
  }
  
  Segment::Segment(uuid_t jobId, const AVCodecParameters &parameters) : parameters(parameters) {
    memcpy(this->jobId, jobId, sizeof(uuid_t));
    if (parameters.extradata_size) {
      this->parameters.extradata = (uint8_t *) malloc(sizeof(uint8_t *) * parameters.extradata_size);
      memcpy(this->parameters.extradata, parameters.extradata, parameters.extradata_size);
    }
  }

  Segment::~Segment() {
    if (parameters.extradata) {
      free(parameters.extradata);
      parameters.extradata = nullptr;
    }
  }

  Segment::pointer Segment::create(uuid_t jobId, const AVCodecParameters &parameters) {
    return std::make_unique<Segment>(jobId, parameters);
  }

  Segment::pointer Segment::copy(const Segment::pointer& toCopy) {
    auto ret = Segment::create(toCopy->jobId, toCopy->parameters);
    ret->pts = toCopy->pts;
    ret->dts = toCopy->dts;
    ret->time_base = toCopy->time_base;
    for (const auto& packet : toCopy->packets) {
      ret->append(Packet::copy(packet));
    }
    return ret;
  }

  bool Segment::empty() {
    return packets.empty();
  }

  Segment::pointer Segment::next() {
    auto n = create(jobId, parameters);
    n->time_base = time_base;
    return n;
  }
  
  void Segment::append(const Packet::pointer& packet) {
    if (0 == packets.size()) {
      pts = packet->pts;
      dts = packet->dts;
    }
    packets.push_back(std::move(Packet::copy(packet)));
    npackets++;
  }
  
}
