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

#include <fr/media2/Scaler.h>

namespace fr {
  namespace media2 {

    Scaler::Scaler(int width, int height, AVPixelFormat fmt) {
      outputFrame->width = width;
      outputFrame->height = height;
      outputFrame->format = (int)fmt;
    }

    Scaler::~Scaler() {
      if (nullptr != context) {
	sws_freeContext(context);
      }
    }

    void Scaler::process(Frame::const_pointer frame,
			 StreamData::pointer stream) {
      if (nullptr == context) {
	if (outputFrame->height < 0 || outputFrame->width < 0) {
	  outputFrame->width = frame->width;
	  outputFrame->height = frame->height;
	}
	if (outputFrame->format == -1) {
	  outputFrame->format = frame->format;
	}
	context = sws_getCachedContext(context, frame->width,
	   frame->height, (AVPixelFormat) frame->format,
	   outputFrame->width, outputFrame->height,
	   (AVPixelFormat)outputFrame->format, SWS_BICUBIC, nullptr,
	   nullptr, nullptr);
	
      }
      sws_scale(context, frame->data, frame->linesize, 0,
		frame->height, outputFrame->data, outputFrame->linesize);
      frames(outputFrame, stream);
    }
    
  }
}
