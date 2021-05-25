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
 * Holds a StreamData object and provides a callback to subscribe
 * to packets from a stream. PacketReader will hold a vector of these
 * if it able to open the media source it is created with.
 */

#pragma once

#include <boost/signals2.hpp>
#include <memory>
#include <fr/media2/Packet.h>
#include <fr/media2/StreamData.h>


namespace fr {
  namespace media2 {

    class Segment;
    
    class Stream {
    public:
      using pointer = std::shared_ptr<Stream>;

      // The usual construction
      Stream() = default;
      ~Stream() = default;
      // Construct from a segment
      explicit Stream(Segment*);
      
      // A pointer to hold the stream data (Different than
      // above pointer def)
      StreamData::pointer data;
      
      /**
       * Expose a boost signal that can be subscribed to. You can subscribe
       * to it with a lambda or a custom object, depending on what you
       * need done. Once subscribed, your callback function will receive 
       * packets until you unsubscribe or the stream in question goes dry
       * (Usually do to an EOF or something.) Your callback function
       * will run in the thread of the caller and you can block reading
       * if you spend a long time processing in your callback. If you're
       * doing fairly simple things, this is fine. If you intend to spend
       * a while processing, it would probably be better if you copied the
       * original packet into a buffer and processed it in its own thread.
       * That is way more complex and harder on your application memory,
       * so it really kind of depends on what you're trying to accomplish.
       *
       * Once you return from the callback, you can consider the packet
       * you received to be invalid. Don't just try to push it into a
       * deque or something or you'll just have a deque full of bad data.
       * Copy the packet instead, if you need it. It should be a fairly
       * low-impact operation.
       */
      
      boost::signals2::signal<void(const Packet::pointer& packet, StreamData::pointer stream)> packets;

      /**
       * Forward a packet to subscribers. Users of the API don't need to
       * worry about this particularly.
       */
      void forward(const Packet::pointer& packet);
    };
    
  }
}
