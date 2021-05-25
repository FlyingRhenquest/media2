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
 * An <a href="https://ffmpeg.org/doxygen/trunk/structAVPacket.html">AVPacket</a>
 * is compressed video data. LibAV really wants to manage the memory
 * in them. This class provides some ulities to interact with them.
 * In this case, mostly static member functions to create, copy and
 * delete them.
 */

#pragma once

extern "C" {
#include <libavcodec/packet.h>
}

#include <memory>

namespace fr {
  namespace media2 {

    // TODO: Clean this up later and make it a handle/body container for an AVPacket.
    
    class Packet {
    public:

      // Destroy a packet. You can use this for a destructor
      // if you create your own shared/unique ptrs with
      // for AVPacket.
      static void destroy(AVPacket* pkt);
      
      // Define unique ptr and const ptr for same.
      // We can get by passing const ptrs around -- LibAV works
      // on the raw pointer anyway, which is probably a technical
      // foul but it works and no one complains.
      
      using pointer = std::unique_ptr<AVPacket, decltype(&Packet::destroy)>;

      // Create an empty packet. For reading (Which is mostly
      // all we're doing for the near term) it will be filled
      // in by the library reading the packets.
      static pointer create();

      // Create a packet pointer to nullptr
      static pointer nullPacket();

      // Copy a packet. Note that this may or may not actually copy
      // memory, depending on what LibAV feels like. What I'm
      // instructing it to do is to create a copy Ptr and then
      // av_packet_ref the copy with the original.
      static pointer copy(const pointer& toCopy);

      // Returns true if this packet contains an iframe
      static bool containsIFrame(const pointer& packet);
    };
    
  }
}
