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
 * Holds data I want to know about streams. This includes an AVStream
 * pointer, maybe a codec, codec context, media type and assiociated
 * filename/url of the container the stream originated from. All seems
 * like it'd be handy for debugging upstream components.
 *
 * A StreamData pointer will be forwarded with packet callbacks from
 * the stream class, along with the packet in question. The StreamData
 * pointer and any of its components are allowed to be null and should
 * be tested before use.
 *
 * StreamData pointers are shared pointers and will exist as long
 * as anything holds a pointer to that data.
 */

#pragma once

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_par.h>
}

#include <memory>
#include <string>

namespace fr {
  namespace media2 {

    class Segment;
    
    class StreamData {
    public:
      using pointer = std::shared_ptr<StreamData>;

      StreamData(const std::string& filename = "");
      // Create a StreamData from a Segment, which contains
      // all the information we need. You can include
      // filename if you want it set in the stream data,
      // but this will not actually open a file.
      StreamData(Segment*, const std::string& filename="");
      // You can copy the shared ptr though
      StreamData(const StreamData& toCopy) = delete;
      ~StreamData();
      
      // Set on creation by PacketReader
      const std::string filename;
      /**
       * Stream is managed by LibAV as part of the format context. I just
       * gather it here as it contains useful information that I might want
       * for further upstream. Always check to make sure it's not null.
       */
      AVStream *stream = nullptr;
      /**
       * Copy time_base from stream for reasons
       */
      AVRational time_base;
      /**
       * Codec parameters, always check to make sure it's not null.
       */
      AVCodecParameters *parameters = avcodec_parameters_alloc();
      /**
       * Also gather this here because I always have to go
       * looking for it and it's kind of a pain in the ass.
       */
      AVMediaType mediaType = AVMEDIA_TYPE_UNKNOWN;
      /**
       * Yeah I'm storing a codec pointer here too. You want the name
       * of the codec or to query its capabilities? I got you covered.
       * This pointer is also managed by LibAV.
       */
      AVCodec *codec = nullptr;

      /**
       * This class currently wants to own the working codec context.
       * So we'll set some things up for it...
       */

      /**
       * Function to deallocate it
       */

      static void destroyContext(AVCodecContext* ctx);

      /**
       * Type to manage it
       */
      using ContextPointer = std::unique_ptr<AVCodecContext, decltype(&StreamData::destroyContext)>;

      /**
       * Variable to hold it. This can be null. You'll want to check it
       * and to pass it to libav things that need codec context using
       * context.get().
       */

      ContextPointer context = ContextPointer{nullptr, &StreamData::destroyContext};

      /**
       * Method to set it and take ownership. The codec context
       * you pass it will be null on return.
       */

      void setContext(AVCodecContext** ctx);
      
    };
    
  }
}
