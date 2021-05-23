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

#include <fr/media2/Encoder.h>
#include <iostream>

namespace fr::media2 {

  Encoder::Encoder(std::string codecName) {
    stream->data = std::make_shared<StreamData>();
    stream->data->parameters = avcodec_parameters_alloc();
    // If codec is empty, try to figure it out when
    // we get the first packet
    if (!codecName.empty()) {
      stream->data->codec = (AVCodec*) avcodec_find_encoder_by_name(codecName.c_str());
      if (nullptr == stream->data->codec) {
	std::string err{"Could not find codec "};
	err.append(codecName);
	throw std::logic_error(err);
      }
    }
  }

  Encoder::~Encoder() {
    if (nullptr != stream->data->parameters) {
      avcodec_parameters_free(&stream->data->parameters);
      stream->data->parameters = nullptr;
    }
  }

  void Encoder::process(Frame::const_pointer frame,
			StreamData::pointer streamIn) {
    if (0 == stream->data->context->time_base.den) {
      stream->data->context->time_base = streamIn->stream->time_base;
    }
    int retval = avcodec_send_frame(stream->data->context.get(), frame.get());
    if (retval < 0) {
      std::string err{"Error encoding frame: "};
      err.append(std::to_string(retval));
      throw std::runtime_error(err);
    }
    while (retval >= 0) {
      retval = avcodec_receive_packet(stream->data->context.get(), workingPacket.get());
      if (retval == AVERROR(EAGAIN) || retval == AVERROR_EOF) {
	break;
      } else if (retval < 0) {
	std::string err{"Error encoding frame: "};
	err.append(std::to_string(retval));
	throw std::runtime_error(err);
      }
      std::cout << "Encoding packet" << std::endl;
      stream->packets(workingPacket, stream->data);
      av_packet_unref(workingPacket.get());
    }
  }

  void Encoder::subscribeCallback(FrameSource *source) {
    stream->data->codec = (AVCodec*) avcodec_find_encoder(source->parameters->codec_id);
    AVCodecContext *context = avcodec_alloc_context3(stream->data->codec);
    if (!context) {
      throw std::runtime_error("Could not find codec");
    }
    avcodec_parameters_copy(stream->data->parameters, source->parameters);
    if (AVMEDIA_TYPE_AUDIO == stream->data->parameters->codec_type) {
      context->sample_fmt = (AVSampleFormat) stream->data->parameters->format;
      context->sample_rate = stream->data->parameters->sample_rate;
      context->channels = stream->data->parameters->channels;
      context->channel_layout = stream->data->parameters->channel_layout;
      context->frame_size = stream->data->parameters->frame_size;
    } else if (AVMEDIA_TYPE_VIDEO == stream->data->parameters->codec_type) {
      context->pix_fmt = (AVPixelFormat) stream->data->parameters->format;
      context->width = stream->data->parameters->width;
      context->height = stream->data->parameters->height;
    }
    stream->data->mediaType = stream->data->parameters->codec_type;

    if (0 != source->time_base.den) {
      context->time_base = source->time_base;
    }

    stream->data->setContext(&context);
    
    int retval = avcodec_open2(stream->data->context.get(), stream->data->codec, nullptr);
    if (retval < 0) {
      std::string err("Could not open context for codec ");
      err.append(std::to_string(source->parameters->codec_id));
      throw std::runtime_error(err);
    }
  }

}
