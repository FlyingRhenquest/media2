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

#include <fr/media2/Frame.h>

namespace fr {
  namespace media2 {

    void Frame::destroy(AVFrame *frame) {
      av_frame_free(&frame);
    }
    
    Frame::pointer Frame::create() {
      return Frame::pointer(av_frame_alloc(), &Frame::destroy);
    }

    Frame::pointer create(int width,
			 int height,
			 AVPixelFormat fmt,
			 int align) {
      Frame::pointer retval{Frame::create()};
      retval->width = width;
      retval->height = height;
      retval->format = fmt;
      av_frame_get_buffer(retval.get(), align);
      return retval;
    }

    Frame::pointer create(int64_t layout, AVSampleFormat format, int rate,
			  int align) {
      Frame::pointer retval{Frame::create()};
      retval->channel_layout = layout;
      retval->format = format;
      retval->sample_rate = rate;
      av_frame_get_buffer(retval.get(), align);
      return retval;
    }
    
    Frame::pointer clone(Frame::const_pointer copy) {
      return Frame::pointer(av_frame_clone(copy.get()), &Frame::destroy);
    }
  }
}
  
