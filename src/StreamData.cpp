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

#include <fr/media2/StreamData.h>
#include <fr/media2/Segment.h>

namespace fr {
  namespace media2 {

    StreamData::StreamData(const std::string &filename) : filename(filename) {}

    StreamData::StreamData(Segment* seg, const std::string &filename) : filename(filename) {
      avcodec_parameters_copy(parameters, &seg->parameters);
      codec = (AVCodec*) avcodec_find_decoder(parameters->codec_id);
      time_base = seg->time_base;
      if (nullptr == codec) {
	throw std::runtime_error("Unknown codec id: " + std::to_string((int) parameters->codec_id));
      }
      AVCodecContext *ctx = avcodec_alloc_context3(codec);
      if (nullptr == ctx) {
	throw std::runtime_error("Could not create codec context");
      }
      ctx->time_base = time_base;
      avcodec_parameters_to_context(ctx, parameters);
      setContext(&ctx);
    }
    
    StreamData::~StreamData() {
      if (nullptr != parameters) {
	avcodec_parameters_free(&parameters);
	parameters = nullptr;
      }
    }
    
    void StreamData::destroyContext(AVCodecContext* ctx) {
      avcodec_free_context(&ctx);
    }

    void StreamData::setContext(AVCodecContext** ctx) {
      // Copy ctx pointer in there
      context = ContextPointer(*ctx, &StreamData::destroyContext);
      // Set ctx you sent us to null
      *ctx = nullptr;
      // This is fine...
    }
    
  }
}
