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
 * An audio resampler. Can convert bitrates, formats et al.
 */

extern "C" {
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>  
}



#pragma once

namespace fr {
  namespace media2 {

    class Resampler : public FrameSubscriber {
    public:
      // Resampler outputs a frame that a frame subscriber can
      // subscribe to, via boost signal.
      boost::signals2::signal<void(Frame::const_pointer,
				   StreamData::pointer)> frames;
      
      // Specify the output format you want (Channel layout,
      // sample format and rate. When we receive the first frame,
      // we'll create a context to translate whatever we're
      // receiving to the format you're looking for.
      Resampler(int64_t layout, AVSampleFormat format,
		int sample_rate);
      Resampler(const Resampler& copy) = delete;
      virtual ~Resampler() override;

    protected:
      Frame::pointer workingFrame;
      SwrContext *context = nullptr;
      int maxSamples = 0;
      
      
      // Sets up sws context the first time process handles
      // a frame.
      void init(Frame::const_pointer);
      
      void process(Frame::const_pointer,
                   StreamData::pointer) override;


    };
    
  }
}
