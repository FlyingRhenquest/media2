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

#include <fr/media2/Segmenter.h>
#include <uuid.h>

namespace fr::media2 {

  Segmenter::Segmenter(size_t nframes) : nframes(nframes) {
    uuid_generate(jobId);
  }

  Segmenter::~Segmenter() {
    flush();
  }

  // This will happily send an empty one, so you can just keep calling flush until
  // the segment you receive returns true when you call empty() on it.

  // If you call flush and there are still frames in your stream AND the next frame
  // is not an iframe, you're gonna have a bad time. Don't do that. I might drop an
  // assert into Segment at some point or something.
  void Segmenter::flush() {
    segments(currentSegment);
    currentSegment = currentSegment->next();
  }

  void Segmenter::process(const Packet::pointer &packet, StreamData::pointer stream) {
    if (nullptr == currentSegment.get()) {
      if (nullptr == stream->parameters) {
	// If you get one of these, I probably forgot to copy the parameters
	// pointer from the stream to the stream parameters somewhere.
	throw std::runtime_error("Stream is missing parameters.");
      }
      currentSegment = Segment::create(jobId, *stream->parameters);
      if (nullptr != stream->stream) {
	currentSegment->time_base = stream->stream->time_base;
      }

      // (video) Segment needs to start on an iframe, and the first frame of your
      // video has to be an iframe. If they're not, you're doing something
      // strange that I'm not going to support.
      if ((AVMEDIA_TYPE_VIDEO == stream->mediaType) && !Packet::containsIFrame(packet)) {
	throw std::runtime_error("First packet in stream is not an IFrame.");
      }
      currentSegment->append(packet);
      currentFrames++;
    } else {
      if (((AVMEDIA_TYPE_VIDEO == stream->mediaType) && Packet::containsIFrame(packet)) ||
	  ((AVMEDIA_TYPE_VIDEO != stream->mediaType) && (currentFrames >= nframes))) {
	segments(currentSegment);
	currentSegment = currentSegment->next();
	currentFrames = 0l;
      }
      currentSegment->append(packet);
      currentFrames++;
    }
  }
  
}
