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


#include <fr/media2/Decoder.h>
#include <fr/media2/Frame.h>

namespace fr {
  namespace media2 {

    void Decoder::process(const Packet::pointer& packet,
			  StreamData::pointer stream) {
      if (stream.get() && stream->context.get()) {
	int avret = avcodec_send_packet(stream->context.get(),
					packet.get());
	while(avret >= 0) {
	  avret = avcodec_receive_frame(stream->context.get(),
					workingFrame.get());
	  if (AVERROR(EAGAIN) == avret || AVERROR_EOF == avret) {
	    break;
	  } else if (avret >= 0) {
	    frames(workingFrame, stream);
	    av_frame_unref(workingFrame.get());
	  }
	}
      }
    }


    void Decoder::subscribeCallback(Stream::pointer to) {
      avcodec_parameters_copy(parameters, to->data->parameters);
      time_base = to->data->time_base;
    }
  }
}
