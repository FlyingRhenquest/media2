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

#include "fr/media2/Packet.h"
extern "C" {
#include <libavutil/frame.h>
}

namespace fr {
  namespace media2 {

    Packet::pointer Packet::create() {
      return pointer{av_packet_alloc(), &Packet::destroy}; 
    }

    Packet::pointer Packet::copy(const Packet::pointer& toCopy) {
      auto copy = create();
      av_packet_ref(copy.get(), toCopy.get());
      return copy;
    }

    void Packet::destroy(AVPacket *pkt) {
      av_packet_free(&pkt);
    }

    bool Packet::containsIFrame(const Packet::pointer& packet) {
      return packet->flags & AV_PKT_FLAG_KEY;
    }
    
  }
}
