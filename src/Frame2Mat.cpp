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

#include <fr/media2/Frame2Mat.h>

namespace fr {
  namespace media2 {

    void process(Frame::const_pointer frame, StreamData::pointer stream) {
      if (AV_PIX_FMT_BGR24 != frame->format) {
	if (nullptr == bgrScaler.get()) {
	  bgrScaler = std::make_unique<Scaler>(frame->width, frame->height, AV_PIX_FMT_BGR24);
	  bgrScaler->subscribe(this);
	}
	// Request conversion to correct pixformat
	frames(frame, stream);
      } else {
	// Forward BGR frames for mat conversion
	processBgr(frame, stream);
      }
    }

    void processBgr(Frame::const_pointer frame, StreamData::pointer stream) {
      cv::Mat rawMat(frame->width, frame->height, CV_8UC3, frame->data[0],
		     frame->linesize[0]);
      cv::Mat copied;
      rawMat.copyTo(copied);
      mats(copied);
    }
    
  }
}
