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

#include <fr/media2/Resampler.h>
#include <stdexcept>

namespace fr::media2 {

  Resampler::Resampler(int64_t layout,
		       AVSampleFormat format,
		       int rate) :
    workingFrame{Frame::create(layout, format, rate)} {
    workingFrame->nb_samples = 0;
  }

  Resampler::~Resampler() {
    if (nullptr == context) {
      swr_free(context);
    }
  }

  void init(AVFrame::const_pointer frame) {
    context = swr_alloc_set_opts(context, workingFrame->channel_layout,
				 (AVSampleFormat) workingFrame->format, workingFrame->sample_rate,
				 frame->channel_layout, (AVSampleFormat) frame->format,
				 frame->sample_rate, 0, nullptr);
    workingFrame->channels = av_get_channel_layout_nb_channels(workingFrame->channel_layout);
    // set context options for input ("ich") and output ("och") channels
    if (0 == frame->channel_layout) {
      av_opt_set_int(context, "ich", frame->channels, 0);
    } else {
      av_opt_set_int(context, "ich", av_get_channel_layout_nb_channels(frame->channel_layout), 0);
    }
    av_opt_set_int(context, "och", workingFrame->channels, 0);
    swr_init(context);
  }
  
  void process(AVFrame::const_pointer frame, StreamData::pointer stream) {
    if (nullptr == context) {
      init(frame);
    }
    // Compute the number of samples we have to output
    workingFrame->nb_samples = av_rescale_rnd(frame->nb_samples, workingFrame->sample_rate, frame->sample_rate, AV_ROUND_UP);
    if (workingFrame->nb_samples > maxSamples) {
      maxSamples = workingFrame->nb_samples;
      workingFrame->channels = av_get_channel_layout_nb_channels(workingFrame->channel_layout);
      int fillRc = av_samples_alloc_array_and_samples(&workingFrame->data, &workingFrame->linesize, workingFrame->channels, workingFrame->nb_samples, workingFrame->format, 0);
      if (0 > fillRc) {
	throw std::logic_error("Resample error filling buffers.");
      }
    }

    int convertRc = swr_convert(context, &workingFrame->data[0], workingFrame->nb_samples, (const uint8_t **) &frame->data[0], frame->nb_samples);
    if (0 >= convertRc) {
      throw std::logic_error("Resmaple error converting data.");
    }
    workingFrame->nb_samples = convertRc;
    workingFrame->pts += workingFrame->nb_samples;
    frames(workingFrame);
  }
  
}
