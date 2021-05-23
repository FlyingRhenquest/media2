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
 * Reads packets from a source. This can be anything that ffmpeg can
 * open (URL, rtmp stream, file, whatever.)
 *
 * When the object is created, it opens the stream in question and
 * adds all the streams in the file to a vector that it holds. These
 * Stream objects can be subscribed to by PacketSubscribers. When
 * PacketReader::process is run, it will start processing in a separate
 * thread. Each time it uses libav to read a packet from the stream,
 * it will be forwarded to the appropriate stream in the stream vector.
 *
 * Keep in mind packets are compressed data. They still need to be
 * decoded before you can do much with them, although you can tell
 * whether the packet has a key frame and a few other things based
 * on flags in the AVPacket.
 */

#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#include <boost/signals2.hpp>
#include <boost/sml.hpp>
#include <condition_variable>
#include <fr/media2/Packet.h>
#include <fr/media2/PacketReaderBase.h>
#include <fr/media2/Stream.h>
#include <fr/media2/StreamData.h>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace fr {
  namespace media2 {

    class PacketReader : public PacketReaderBase {
    public:

      PacketReader(const std::string& filename);
      PacketReader(const PacketReader& copy) = delete;
      virtual ~PacketReader() override;
      
      // Signals the user can subscribe to
      struct {
	// Fired with a message when an error occurs.
	boost::signals2::signal<void(const std::string &)> error;
	// Fired when the player hits an EOF
	boost::signals2::signal<void()> eof;
	// Fired when shutdown is called
	boost::signals2::signal<void()> shutdown;
	// Fired when reset is called
	boost::signals2::signal<void()> reset;
      } signals;

      // Defined in base
      PacketReaderStateMachine::Sender stateSender;
      // State starts in ready state
      boost::sml::sm<PacketReaderStateMachine::PlayerState,
	boost::sml::thread_safe<std::recursive_mutex>>
      state;
      // Name of the resource to open
      std::string filename;
      // Streams held here
      std::vector<Stream::pointer> streams;
      // If you want to interact with just audio or video streams,
      // these will be populated on open.
      std::vector<Stream::pointer> audioStreams;
      std::vector<Stream::pointer> videoStreams;

      // Send a StateMachine event to the state machine
      template <typename Event>
      void sendEvent(Event e) {state.process_event(e);}

      // If processing thread is joinable, join it.
      // You will have to call this if you want to
      // wait for processing to complete. If you
      // have an event loop or something, you won't
      // have to, and the thread will rejoin when
      // the object is destroyed or reopened.
      void join() override;

      // Kicks off processing. You need to call this
      // before your callbacks will receive any
      // packets. Create the reader, subscribe your
      // objects to the streams, then call process.
      // *Does not block*.
      void process() override;
      
    protected:
      std::thread processingThread;
      AVFormatContext *formatContext = nullptr;
      std::mutex pauseMutex;
      std::mutex streamMutex;
      std::condition_variable paused;
      
      // First thing to do in opening the media source. This
      // object owns the format.
      // TODO: Provide methods to supply input format
      // and an avdictionary for this function to use.
      bool openFormat();
      // Opens codecs and sets up the stream vectors
      bool setupStreams();
      // Processes the file in processingThread.
      void processPrivately();
      
      // Open kicks off opening. This is handled automatically
      // so that the streams can be set up before you try
      // to use them.
      bool open() override;
      
      // Closes all the things. 
      void close() override;
      
      // Reset the stream to the beginning and re-open
      void reset() override;
      
      // Unpauses stream processing (Sending the
      // StateMachine::pause event pauses processing.)
      void unpause() override;
      
      // Shut down the reader thread
      void shutdown() override;

      // Send error to subscribers
      void error(const std::string &msg) override;

      // Send eof to subscribers
      void eof() override;

      void initState();

    };
  }
}
