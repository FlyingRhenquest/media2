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
 * Subscribes to the packet signal in Stream.h. Implement the
 * protected process method to process the packets however
 * you feel like it.
 */

#pragma once

#include <fr/media2/Stream.h>
#include <fr/media2/StreamData.h>
#include <vector>

namespace fr {
  namespace media2 {
    class PacketSubscriber {
    protected:
      virtual void process(const Packet::pointer& packet, StreamData::pointer stream) = 0;
      // Subscription stores your subscription so you can disconnect
      // when you want to. Right now I'm assuming that most of these objects
      // will only subscribe to one thing. We'll see how long that lasts...
      std::vector<boost::signals2::connection> subscriptions;
    public:
      
      using pointer = std::unique_ptr<PacketSubscriber>;
      PacketSubscriber() = default;
      // These subscribe to a thing, having a copy constructor for them
      // doesn't really make sense.
      PacketSubscriber(const PacketSubscriber &copy) = delete;
      virtual ~PacketSubscriber();

      // Subscribe to stream. process will be called whenever the
      // packet source has a packet for you to process
      virtual void subscribe(Stream::pointer to);
      // Unsubscribe from stream.
      virtual void unsubscribe();
      // Called after subscribe so children can do things
      virtual void subscribeCallback(Stream::pointer to);
    };
  }
}
