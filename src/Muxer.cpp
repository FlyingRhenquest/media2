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

#include <fr/media2/Muxer.h>
#include <iostream>

namespace fr::media2 {

  Muxer::Muxer(std::string filename, std::string format, long bufferMax) :
    filename(filename),
    format(format),
    bufferMax(bufferMax) {
    if (!format.empty()) {
      avformat_alloc_output_context2(&context, nullptr, format.c_str(), filename.c_str());
      if (nullptr == context) {
        std::string err("Error allocating output format context for ");
        err.append(filename);
        err.append(" with format specified as ");
        err.append(format);
        throw std::runtime_error(err);
      }
    } else {
      avformat_alloc_output_context2(&context, nullptr, nullptr, filename.c_str());
      if (nullptr == context) {
        std::string err("Error allocating output format context for ");
        err.append(filename);
        throw std::runtime_error(err);
      }
    }
    open();
  }

  Muxer::~Muxer() {
    unsubscribe();
    // Close, if we haven't already (This also clears stream info.)
    close();
    if (nullptr != context) {
      avformat_free_context(context);
      context = nullptr;
    }
  }

  void Muxer::subscribe(Stream::pointer to) {
    if (!streamStart) {
      // This is an ffmpeg thing -- if you try to add a stream after the
      // muxer has started writing, some stuff in the second stream will
      // not be initialized correctly and ffmpeg will crash.
      throw std::runtime_error("Can not add a new stream after muxer has written its header.");
    }
    auto info = std::make_shared<StreamInfo>(this);
    info->subscribe(to);
    streaminfo.push_back(info);
    info->stream = avformat_new_stream(context, to->data->codec);
    if (nullptr == info->stream) {
      throw std::runtime_error("Error creating new stream");
    }
    info->stream->time_base = to->data->time_base;
    info->stream->avg_frame_rate = to->data->avg_frame_rate;
    info->stream->r_frame_rate = to->data->r_frame_rate;
    avcodec_parameters_copy(info->stream->codecpar, to->data->parameters);
  }

  void Muxer::unsubscribe() {
    // Immediately disconnects all subscriptions to encoders
    // You shouldn't have to call this directly (Unless you really
    // want to.)
    for (auto info : streaminfo) {
      if (info->subscription.connected()) {
        info->subscription.disconnect();
      }
    }
  }

  void Muxer::open() {
    if (state != States::OPEN) {
      if (!(context->flags & AVFMT_NOFILE)) {
        int avRet = avio_open(&context->pb, filename.c_str(), AVIO_FLAG_WRITE);
        if (avRet < 0) {
          std::string err("Error opening ");
          err.append(filename);
          err.append(" rc = ");
          err.append(std::to_string(avRet));
          state = States::ERROR;
          throw(err);
        }
      }
      state = States::OPEN;
    }
  }

  void Muxer::close() {
    flush();
    if (state == States::OPEN) {
      if (! streamStart) {
        // Flush yaddda
        av_interleaved_write_frame(context, nullptr);
        // If streamStart is false, we never saw any packets
        // and trying to write a trailer will probably
        // segv
        av_write_trailer(context);
      }
      if (!(context->flags & AVFMT_NOFILE)) {
        avio_closep(&context->pb);
      }
      state = States::CLOSED;
      streamStart = true;
    }
  }

  void Muxer::process(const Packet::pointer& packet, StreamData::pointer stream) {
    throw std::runtime_error("Wrong process method called.");
  }

  void Muxer::write(Packet::pointer &packet) {
    // Write header if we haven't yet
    if (streamStart) {
      streamStart = false;
      int avRet = avformat_write_header(context, nullptr);
      if (avRet < 0) {
        throw std::runtime_error("Could not write media header.");
      }
    }
    int avRet = av_interleaved_write_frame(context, packet.get());
    if (avRet < 0) {
      std::string err{"Error writing packet. RC = "};
      err.append(std::to_string(avRet));
      throw std::runtime_error(err);
    }
  }

  void Muxer::flush() {
    std::lock_guard<std::mutex> lock(bufferMutex);
    while(!buffer.empty()) {
      Packet::pointer pkt = std::move(buffer.front());
      buffer.pop_front();
      write(pkt);
    }
  }

  void Muxer::process(const Packet::pointer& packet, StreamData::pointer stream, StreamInfo* info) {
    // Remap packet index to the output stream in this object
    try {
      copyTimingData(stream, info);
    } catch (std::runtime_error &e) {
      std::cerr << "Warning: Ignoring exception in muxer; output file will probably be incorrect." << std::endl;
      std::cerr << e.what() << std::endl;
    }
    packet->stream_index = info->stream->index;
    if (buffer.size() < bufferMax) {
      std::lock_guard<std::mutex> lock(bufferMutex);
      buffer.push_back(Packet::copy(packet));
    } else {
      // Start reading off the front
      Packet::pointer pkt = Packet::nullPacket();
      std::lock_guard<std::mutex> lock(bufferMutex);
      buffer.push_back(Packet::copy(packet));
      pkt = std::move(buffer.front());
      buffer.pop_front();
      write(pkt);
    }
  }

  void Muxer::copyTimingData(StreamData::pointer inputStream, StreamInfo* outputInfo) {
    // Check nulls
    if (outputInfo) {
      if (!outputInfo->stream) {
        throw std::runtime_error("outputInfo->stream is null");
      }
    } else {
      throw std::runtime_error("outputInfo is null");
    }
    if (inputStream.get()) {
      if (!inputStream->stream) {
        throw std::runtime_error("inputStream->stream is null");
      }
    } else {
      throw std::runtime_error("inputStream is null");
    }
    outputInfo->stream->time_base = inputStream->stream->time_base;
    outputInfo->stream->avg_frame_rate = inputStream->stream->avg_frame_rate;
    outputInfo->stream->r_frame_rate = inputStream->stream->r_frame_rate;
  }
}
