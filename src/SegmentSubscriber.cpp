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
 */

#include <fr/media2/SegmentSubscriber.h>

namespace fr::media2 {

  SegmentSubscriber::SegmentSubscriber() {}
  SegmentSubscriber::~SegmentSubscriber() { unsubscribe(); }

  void SegmentSubscriber::subscribe(Segmenter *to) {
    boost::signals2::connection sub =
      to->segments.connect([this](const Segment::pointer &segment) {
	this->process(segment);
      });
    subscriptions.push_back(sub);
  }

  void SegmentSubscriber::unsubscribe() {
    for (auto sub : subscriptions) {
      sub.disconnect();
    }
    subscriptions.clear();
  }
  
}
