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
 * Breaks a stream up into segments. Video streams have to be an iFrame
 * and all subsequent frames to the next iFrame. If you want smaller
 * segments, set the stream gop_size smaller. Audio segments should
 * just be a fixed number of packets.
 *
 */

#pragma once

#include <boost/signals2.hpp>
#include <fr/media2/Segment.h>
#include <fr/media2/PacketSubscriber.h>
#include <mutex>

namespace fr::media2 {

  // Subscribe the Segmenter to ONE (1) stream. If you try to subscribe one
  // segmenter to more than one stream, you're going to have a bad time.
  
  class Segmenter : public PacketSubscriber {
  public:
    // Constructor takes number of frames to include in a segment,
    // but this will be ignored for video data.
    Segmenter(size_t nframes = 250);
    virtual ~Segmenter() override;

    // Boost signal for segments
    boost::signals2::signal<void(const Segment::pointer &segment, StreamData::pointer stream)> segments;

    // Send the current segment now. Might be handy when you hit EOF.
    void flush();

  protected:

    // Generated when the segmenter is created and assigned to the segments
    // it generates
    uuid_t jobId;
    // nframes for audio data
    size_t nframes;
    // Number of frames in current segment.
    size_t currentFrames = 0l;
    Segment::pointer currentSegment;
    std::mutex currentSegmentMutex;
    StreamData::pointer stream;
    
    void process(const Packet::pointer& packet, StreamData::pointer stream) override;
  };
  
}
