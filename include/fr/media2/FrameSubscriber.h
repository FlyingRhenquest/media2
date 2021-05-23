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
 * Decoder has a signal that all frame sources should conform to.
 * If you don't have a streamdata for the source, just emit an
 * empty streamdata shared ptr. FrameSubscriber will attempt to
 * subscribe to that signal for any object passed to subscribe.
 */

#pragma once

#include <boost/signals2.hpp>
#include <fr/media2/FrameSource.h>
#include <fr/media2/Frame.h>
#include <fr/media2/StreamData.h>
#include <functional>
#include <memory>
#include <vector>

namespace fr {
  namespace media2 {

    class FrameSubscriber {
    protected:
      std::vector<boost::signals2::connection> subscriptions;
      // Override process to implement your callback
      virtual void process(Frame::const_pointer frame, StreamData::pointer stream) = 0;
      
    public:
      // This ptr will be copied from source on subscription. I could be convinced
      // to upgrade this to a shared ptr
      FrameSubscriber() = default;
      FrameSubscriber(const FrameSubscriber& copy) = delete;
      virtual ~FrameSubscriber();

      virtual void subscribe(FrameSource *source);
      virtual void subscribe(FrameSource &source);
      
      // You can override subscribeCallback in children classes
      // if you want to run some stuff after the subscription
      // has been set up
      virtual void subscribeCallback(FrameSource *source) {}

      void unsubscribe();
    };
    
  }
}
