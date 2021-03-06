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

#include <fr/media2/FrameSubscriber.h>

namespace fr {
  namespace media2 {
    
    FrameSubscriber::~FrameSubscriber() {
      unsubscribe();
    }
    
    void FrameSubscriber::unsubscribe() {
      for (auto subscription : subscriptions) {
	subscription.disconnect();
      }
    }

    void FrameSubscriber::subscribe(FrameSource &source) {
      this->subscribe(&source);
    }

    void FrameSubscriber::subscribe(FrameSource *source) {
      boost::signals2::connection subscription = source->frames.connect(
	 [this](Frame::const_pointer frame, StreamData::pointer stream) {
	   this->process(frame, stream);
	 });
      subscriptions.push_back(subscription);
      subscribeCallback(source);
    }
  }
}
