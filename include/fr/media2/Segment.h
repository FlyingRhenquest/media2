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
 * Code to accumulate a "segment", which in this case is an iframe and
 * all the packets following that iframe, until the next iframe (for
 * video data) along with the codec parameters for the stream.
 *
 * If the Segment is holding audio data, you can set the number of
 * packets each segment should hold, since they don't have iframes.
 *
 * Each segment will expose the PTS and DTS of the iframe / first
 * packet in the segment for easy sorting and stream reassembly.
 *
 */

#pragma once

#include <fr/media2/Serialization.h>
#include <fr/media2/Packet.h>
#include <fr/media2/Stream.h>
#include <memory>
#include <uuid.h>
#include <vector>

namespace fr::media2 {

  class Segment {
  public:
    using pointer = std::unique_ptr<Segment>;

    Segment();
    Segment(uuid_t jobid, const AVCodecParameters &parameters);
    ~Segment();

    // Create a new segment (Idiom should be if you have a new stream,
    // create ONE new segment, and then use segment::next to get a new
    // one each time you hit a new iFrame.
    static pointer create(uuid_t jobid, const AVCodecParameters &parameters);

    // Copy a segment (This has to copy all the packets, too)
    static pointer copy(const pointer& toCopy);
    
    // Next gets a copy of jobid and parameters from the current segment
    pointer next();
    
    // Copies packet and appends it to vector
    // Packet must be copied, as the working packet will
    // get recycled as soon as append returns,
    // corrupting the memory in our buffer.
    void append(const Packet::pointer& packet);

    // Returns true if there are no packets in this segment.
    bool empty();
    
    // Each stream associated with a video should have its own job ID.
    // This ID should be applied to all Segments in that stream.
    uuid_t jobId;

    // PTS and DTS of first packet in the segment (DTS is probably usually
    // the value you're interested in.)
    int64_t pts;
    int64_t dts;

    AVCodecParameters parameters;
    size_t npackets = 0l;
    std::vector<Packet::pointer> packets;

  private:
    friend class boost::serialization::access;
    // Defining serilization for segments is now pretty easy since I've already
    // defined all its component types

    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
      ar << jobId;
      ar << pts;
      ar << dts;
      ar << parameters;
      ar << npackets;
      for (const Packet::pointer &packet : packets) {
	ar << *packet;
      }
    }

    template<class Archive>
    void load(Archive &ar, const unsigned int version) {
      ar >> jobId;
      ar >> pts;
      ar >> dts;
      ar >> parameters;
      size_t pkts;
      ar >> pkts;
      for (int i = 0; i < pkts; ++i) {
	Packet::pointer pkt = Packet::create();
	ar >> *pkt;
	append(pkt);
      }
    }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
      boost::serialization::split_member(ar, *this, version);
    }

  };

}

