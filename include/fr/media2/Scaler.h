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
 * Rescales video frames. This can change the frame resolution or pixel
 * format. This object is not designed to handle changes to input
 * resolution. That gets set when the first frame is passsed into it.
 */

#pragma once

extern "C" {
#include <libavutil/imgutils.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <boost/signals2.hpp>
#include <fr/media2/Frame.h>
#include <fr/media2/FrameSubscriber.h>

namespace fr {
  namespace media2 {
    class Scaler : public FrameSubscriber {
    public:

      boost::signals2::signal<void(Frame::const_pointer,
	StreamData::pointer)> frames;
      
      // Pass desired width, height and pixel format
      // in. If you want the same pixel format as the
      // source, use AV_PIX_FMT_NONE. If you want the
      // same resolution as the source, set width and
      // height to -1.
      Scaler(int width, int height, AVPixelFormat fmt = AV_PIX_FMT_NONE);
      ~Scaler();
      Scaler(const Scaler &copy) = delete;
      Scaler operator=(const Scaler &copy) = delete;

    protected:

      void process(Frame::const_pointer frame,
		   StreamData::pointer stream) override;
      
    private:
      SwsContext *context = nullptr;
      Frame::pointer outputFrame = Frame::create();
      
    };
  }
}
