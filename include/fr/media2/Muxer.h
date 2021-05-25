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
 * Muxes data from multiple encoders.
 *
 * This is a bit different from the others because this is the (or at least, a)
 * end of the line for packets. There's not a signal to subscribe to in this
 * object (at least for the time being.)
 *
 * This object can also handle subscribing to multiple packet sources.
 * Each new subscription will create an entry in StreamInfo with the
 * subscription information, so that we can drop the subscriptions
 * later.
 * 
 * This is a no-frills muxer, but there's a great deal of room to add
 * frills in other objects. We may also decide to build a more-frills
 * muxer at some point in the future.
 */

#pragma once

extern "C" {
#include <libavformat/avformat.h>
}

#include <boost/signals2.hpp>
#include <deque>
#include <fr/media2/PacketSubscriber.h>
#include <fr/media2/StreamData.h>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace fr::media2 {

  class Muxer : public PacketSubscriber {
  private:
    // Router and info object for streams this muxer owns
    class StreamInfo {
    public:
      using pointer = std::shared_ptr<StreamInfo>;
      boost::signals2::connection subscription;
      Muxer* owner = nullptr;
      // Output stream assigned by muxer
      AVStream *stream;
      bool firstFrame = true;

      StreamInfo(Muxer *owner) : owner(owner) {}

      ~StreamInfo() {
	subscription.disconnect();
      }

      void subscribe(Stream::pointer to) {
	subscription = to->packets.connect([this](const Packet::pointer& packet,
					      StreamData::pointer stream) {
	  this->process(packet, stream);
	});
      }
      
      // Actually handles the processing for incoming packets on this
      // stream. Forwards on to the process method above
      void process(const Packet::pointer &packet, StreamData::pointer stream) {
	if (nullptr == owner) {
	  throw std::runtime_error("Nullptr in muxer (This should not be possible.)");
	}
	if (firstFrame) {
	  // On first frame processed, copy stream info from
	  // incoming stream to owner's output stream
	  // stream indexes 
	}
	owner->process(packet, stream, this);
      }
    };

  public:
    std::vector<StreamInfo::pointer> streams;
    std::deque<Packet::pointer> buffer;

    // Constructor will attempt to determine the format from the filename.
    // Specify it directly if you really want to be sure
    Muxer(std::string filename, std::string format = "", long bufferMax = 300);
    Muxer(const Muxer& copy) = delete;
    virtual ~Muxer() override;
    Muxer operator=(const Muxer& copy) = delete;

    // Have to override subscribe to store StreamDatas for sources
    // I don't check for duplicates, so make sure you get your
    // subscriptions right!
    void subscribe(Stream::pointer to) override;
    // Also have to override unsubscribe for the same reason
    // (This unsubscribes ALL streams.)
    void unsubscribe() override;

    // Opens output for writing
    void open();
    // Close file -- this (should) flush buffers to wherever you're
    // writing, clears stream data, etc.
    void close();
    
    // Flushes packet buffer (Close calls this automatically)
    void flush();

    // Lets see if we can get away with cheesy state tracking, shouldn't
    // need to bust out SML for this

    enum class States {
      READY, OPEN, CLOSED, ERROR
    };

    States state = States::READY;

  protected:
    // This one doesn't actually do anything in this case
    void process(const Packet::pointer& packet, StreamData::pointer stream) override;
    void process(const Packet::pointer& packet, StreamData::pointer stream, StreamInfo* info);

  private:
    // ffmpeg format to write to.
    std::string format;
    // Filename to write to (Can be a RTMP URL or anything else
    // ffmpeg can write to.)
    std::string filename;
    // Output format context
    AVFormatContext *context = nullptr;
    // Keeps track of stream start so we can write the header in process
    bool streamStart = true;
    // Number of elements to retain in buffer.
    long bufferMax;
    std::mutex bufferMutex;
    // Write a packet to output
    void write(Packet::pointer &);
  };
}
