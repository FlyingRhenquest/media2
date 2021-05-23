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
 * Decodes packets from a stream. Emits uncompressed AVFrames.
 *
 * Decoding does not use a thread; it runs in the thread of its
 * notifier. If you don't want to block while reading the source
 * and want to buffer packets to feed to the decoder, you can
 * write another PacketSubscriber that buffers them and feeds
 * them to this decoder in its own thread.
 *
 * As with packets, don't expect the frames you get to be valid
 * once your callback returns to the Decoder. If you need to keep
 * the frame around, you'll need to copy it.
 */

#pragma once

#include <boost/signals2.hpp>
#include <fr/media2/PacketSubscriber.h>
#include <fr/media2/Frame.h>
#include <fr/media2/FrameSource.h>

namespace fr {

  namespace media2 {

    class Decoder : public PacketSubscriber, public FrameSource {

    protected:

      Frame::pointer workingFrame = Frame::create();
      
      void process(const Packet::pointer& packet,
		   StreamData::pointer stream) override;

    public:
      Decoder() = default;
      virtual ~Decoder() = default;

      void subscribeCallback(Stream::pointer to) override;

    };
    
  }
}
