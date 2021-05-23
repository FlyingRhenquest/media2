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
 * Convert an AVFrame to an OpenCV Mat object. Provides a signal to get
 * the mat. This class is not designed to handle varying frame sizes
 * or formats. If you're converting frames from multiple streams,
 * create a Frame2Mat object for each different resolution and pixel
 * format.
 */

#pragma once

#include <boost/signals2.hpp>
#include <fr/media2/Frame.h>
#include <fr/media2/FrameSubscriber.h>
#include <fr/media2/Scaler.h>
#include <opencv2/imgproc.hpp>

namespace fr {
  namespace media2 {

    class Frame2Mat : public FrameSubscriber {
    public:
      // Frames signal. If we don't receive frames in BGR format
      // we'll create a scaler to convert them and subscribe IT
      // to this frames signal, then re-emit frames we receive
      // that are not in bgr format to this signal
      boost::signals2::signal<void(Frame::const_pointer,
	     StreamData::pointer)> frames;
      
      boost::signals2::signal<void(cv::Mat)> mats;
      virtual ~Frame2Mat() override;

    protected:
      void process(Frame::const_pointer frame, StreamData::pointer stream) override;
      // Process BGRA frames for the conversion to OpenCV Mats.
      void processBgr(Frame::const_pointer frame, StreamData::pointer stream);
    private:
      // OpenCV wants frames in BGRA pixel format,
      // so we'll just set up a scaler and handle that
      // if the frames we receive are not in that format.
      std::unique_ptr<Scaler> bgrScaler;

    };

  }
}
