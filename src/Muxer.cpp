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

  Muxer::Muxer(std::string filename, std::string format) : filename(filename), format(format) {
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
    auto stream = std::make_shared<StreamInfo>(this);
    stream->subscribe(to);
    streams.push_back(stream);
    stream->stream = avformat_new_stream(context, to->data->codec);
    if (nullptr == stream->stream) {
      throw std::runtime_error("Error creating new stream");
    }
    stream->stream->time_base = to->data->context->time_base;
    avcodec_parameters_copy(stream->stream->codecpar, to->data->parameters);
  }

  void Muxer::unsubscribe() {
    // Immediately disconnects all subscriptions to encoders
    // You shouldn't have to call this directly (Unless you really
    // want to.)
    for (auto stream : streams) {
      if (stream->subscription.connected()) {
	stream->subscription.disconnect();
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
 
  void Muxer::process(const Packet::pointer& packet, StreamData::pointer stream, StreamInfo* info) {
    // Write header if we haven't yet
    if (streamStart) {
      streamStart = false;
      std::cout << "Writing header" << std::endl;
      int avRet = avformat_write_header(context, nullptr);
      if (avRet < 0) {
	throw std::runtime_error("Could not write media header.");
      }
    }
    // Remap packet index to the output stream in this object
    packet->stream_index = info->stream->index;
    std::cout << "Writing frame" << std::endl;
    int avRet = av_interleaved_write_frame(context, packet.get());
    if (avRet < 0) {
      // This apparently doesn't always indicate the stream is done, so
      // I may want to continue processing despite this
      std::string err{"Error writing packet. RC = "};
      err.append(std::to_string(avRet));
      throw std::runtime_error(err);
    }
  }
}
