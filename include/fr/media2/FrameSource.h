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
 * A frame source object. This provides a frames signal and a pointer
 * to some codec parameters.
 */

#pragma once

extern "C" {
#include <libavcodec/codec_par.h>
}

#include <boost/signals2.hpp>
#include <fr/media2/Frame.h>
#include <fr/media2/StreamData.h>

namespace fr::media2 {

  class FrameSource {
  public:
    // Stream in this signal is informational only and can be
    // null in anything that creates this kind of signal.
    boost::signals2::signal<void(Frame::const_pointer,
				 StreamData::pointer)> frames;

    AVCodecParameters *parameters = nullptr;
    AVRational time_base = {0,0};
    AVRational avg_frame_rate = {0,0};
    AVRational r_frame_rate = {0,0};
    FrameSource() {
      parameters = avcodec_parameters_alloc();
    }
    virtual ~FrameSource() {
      avcodec_parameters_free(&parameters);
    }
  };
}
