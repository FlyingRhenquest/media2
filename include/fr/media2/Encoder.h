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
 * Encoder takes uncompressed video and audio frames and turns them into
 * compressed packets in the codec of your choice.
 *
 * Encoder itself is not particularly smart. If you need your image
 * data in a different pixel format or at a different size, you'll
 * need to use a Scaler to change your frames before the encoder
 * receives them. If you need your audio data in a different format
 * or sample rate, you'll need to use a Resampler to make that
 * happen. In most of those cases, it's really a good idea to
 * adjust the packet timestamps yourself prior to encoding.
 * I'll write an object to do that at some point but haven't
 * yet.
 *
 * The encoder doesn't do IO operations. You can take the packets
 * and write them yourself or you can subscribe a Muxer to one
 * or more encoders.
 *
 * Each Encoder should only be responsible for encoding one
 * stream, either video or audio data, definitely not both.
 * If you subscribe an Encoder to two things, you're gonna have
 * a bad time.
 *
 * Encoders will determine many of the parameters they're encoding
 * from the data in the first frame they receive. This information
 * can't change after that point. If you need to change encoding
 * parameters mid-stream, make a new encoder. And maybe some new
 * objects to handle that transition seamlessly.
 */

#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}

#include <fr/media2/Packet.h>
#include <fr/media2/FrameSubscriber.h>
#include <fr/media2/Stream.h>
#include <fr/media2/StreamData.h>
#include <stdexcept>
#include <string>

namespace fr::media2 {

  class Encoder : public FrameSubscriber {
  public:
    // If codecName is empty, try to guess the codec
    // when we receive the first packet.
    Encoder(std::string codecName = "");
    virtual ~Encoder() override;
    Encoder(const Encoder &copy) = delete;
    Encoder operator=(const Encoder &copy) = delete;

    void subscribeCallback(FrameSource *source) override;
    void setTimeBase(AVRational base);
    
    // Stream for this encoder. PacketSubscribers can subscribe to this
    Stream::pointer stream = std::make_shared<Stream>();

  protected:

    void process(Frame::const_pointer frame,
		 StreamData::pointer stream) override;

    // Working packet to write to. This is sent in the packets signal.
    // Once you return from that signal, the packet you received will
    // no longer be valid. So either processes it in the signal callback
    // or copy and store it for later processing. Packet::copy should
    // just move the memory, so it should be reasonably safe to do that
    // (Might make for a lot of allocs, though, if we have to keep
    // reallocating the workingPacket buffers.)
    Packet::pointer workingPacket = Packet::create();
  };
  
}
