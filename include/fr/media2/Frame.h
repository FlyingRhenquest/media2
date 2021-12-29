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
 * Some convenience functions for frames.
 */

#pragma once

extern "C" {
#include <libavutil/frame.h>
}

#include <memory>

namespace fr {

  namespace media2 {

    class Frame {
    public:

      static void destroy(AVFrame*);
      using pointer = std::unique_ptr<AVFrame, decltype(&Frame::destroy)>;
      using const_pointer = const pointer &;

      // create creates a frame. The empty version creates a bufferless
      // frame which can be used for reading. You can also create one
      // with buffers by specifying the parameters required to allocate
      // a frame buffer. This would be more handy for copying existing
      // frames and for creating your own frames to inject into the
      // data stream.
      static pointer create();

      // Create a video frame of a specified size and pixel format
      // You can also specify the alignment if you want to, but
      // ffmpeg is generally pretty good about figuring it out
      static pointer create(int width,
			    int height,
			    AVPixelFormat fmt,
			    int align = 0);

      // Create an audio frame with the specified channel layout,
      // sample format and bit rate
      static pointer create(int64_t layout, AVSampleFormat format,
			    int rate, int align = 0);

      // Clone frame. If you unref the old one, it's basically a copy.
      static pointer clone(const_pointer copy);
    };
  }
}
