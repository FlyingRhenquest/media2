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

#include <fr/media2/PacketReader.h>

namespace fr {
  namespace media2 {

    PacketReader::PacketReader(const std::string &filename) :
      filename(filename),
      stateSender{PacketReaderStateMachine::Sender{this}},
      state{stateSender} {
      state.process_event(PacketReaderStateMachine::open{});
    }

    PacketReader::~PacketReader() {
      close();
      if (processingThread.joinable()) {
        processingThread.join();
      }
    }

    bool PacketReader::open() {
      bool retval = false;
      retval = openFormat() && setupStreams();
      if (!retval) {
        sendEvent(PacketReaderStateMachine::open_error{});
      }
      return retval;
    }

    void PacketReader::close() {
      signals.shutdown();
      std::lock_guard<std::mutex> lock(streamMutex);
      join();
      avformat_close_input(&formatContext);
      streams.clear();
      audioStreams.clear();
      videoStreams.clear();
    }

    void PacketReader::shutdown() {
      close();
    }

    void PacketReader::reset() {
      signals.reset();
      close();
    }

    void PacketReader::unpause() {
      paused.notify_all();
    }

    void PacketReader::join() {
      if (processingThread.joinable()) {
        processingThread.join();
      }
    }

    void PacketReader::process() {
      using namespace boost::sml;
      if (state.is("opened"_s)) {
        sendEvent(PacketReaderStateMachine::play{});
      } else {
        processingThread = std::thread([this](){ processPrivately(); });
      }
    }

    void PacketReader::error(const std::string &msg) {
      signals.error(msg);
    }

    void PacketReader::eof() {
      signals.eof();
    }

    bool PacketReader::openFormat() {
      bool retval = false;
      // Parameters 3 and 4 are input format and a dictionary for options.
      // At some point I need to set it up so the user can set both if they
      // want to.
      int apiRet = avformat_open_input(&formatContext, filename.c_str(), nullptr, nullptr);
      std::string err{"Error opening "};
      err.append(filename);
      if (0 > apiRet) {
        state.process_event(PacketReaderStateMachine::open_error{err});
      } else {
        retval = true;
      }
      return retval;
    }

    bool PacketReader::setupStreams() {
      bool foundStreams = false;
      int apiRet = avformat_find_stream_info(formatContext, nullptr);
      if (0 > apiRet) {
        std::string err{"Could not find any streams in "};
        err.append(filename);
        close();
        state.process_event(PacketReaderStateMachine::open_error{err});
      } else {
        for(int i = 0; i < formatContext->nb_streams; ++i) {
          // TODO: Build create static functions for these classes
          // so if the type ever changes, I don't need to change it
          // here too.
          auto stream = std::make_shared<Stream>();
          auto data = std::make_shared<StreamData>(filename);
          stream->data = data;
          data->stream = formatContext->streams[i];
          data->time_base = data->stream->time_base;
          avcodec_parameters_copy(data->parameters, formatContext->streams[i]->codecpar);
          data->codec = (AVCodec*) avcodec_find_decoder(formatContext->streams[i]->codecpar->codec_id);
          if (!data->codec) {
            stream->data.reset();
          } else {
            stream->data->mediaType = data->codec->type;
            AVCodecContext *ctx = avcodec_alloc_context3(data->codec);
            if (nullptr == ctx) {
              stream->data.reset();
            } else {
              stream->data->setContext(&ctx);
              // Copy default parameters to context if there are any
              avcodec_parameters_to_context(stream->data->context.get(), formatContext->streams[i]->codecpar);
              if(avcodec_open2(data->context.get(), data->codec, nullptr) < 0) {
                stream->data.reset();
              } else {
                foundStreams = true;
                if (AVMEDIA_TYPE_VIDEO == stream->data->mediaType) {
                  videoStreams.push_back(stream);
                } else if (AVMEDIA_TYPE_AUDIO == stream->data->mediaType) {
                  audioStreams.push_back(stream);
                }
              }
              streams.push_back(stream);
            }
          }
        }
      }
      if (foundStreams) {
        state.process_event(PacketReaderStateMachine::open_success{});
      } else {
        std::string err{"Could not open any streams in "};
        err.append(filename);
        state.process_event(PacketReaderStateMachine::open_error{err});
      }
      return foundStreams;
    }

    void PacketReader::processPrivately() {
      // Will be destroyed when it goes out of scope
      auto packet = Packet::create();
      int apiRet = 0;
      state.process_event(PacketReaderStateMachine::play{});

      using namespace boost::sml;
      while(!state.is("done"_s)) {
        if (state.is("paused"_s)) {
          std::unique_lock<std::mutex> lock;
          paused.wait(lock, []{ return true;});
        }
        if (av_read_frame(formatContext, packet.get()) < 0) {
          state.process_event(PacketReaderStateMachine::eof{});
        } else {
          std::lock_guard<std::mutex> lock(streamMutex);
          int streamIndex = packet->stream_index;
          if (streams[streamIndex]->data.get() != nullptr) {
            streams[streamIndex]->forward(packet);
          }
          // Drop the previous packet buffer. Copies of
          // the packet that you make will continue to hold
          // that data.
          av_packet_unref(packet.get());
        }
      }
    }

  } // namespace media
} // namespace fr
