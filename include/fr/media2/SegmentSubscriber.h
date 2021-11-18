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
 * SegmentSubscriber -- Interface that subscribes to segments.
 */

#pragma once

#include <boost/signals2.hpp>
#include <fr/media2/Segment.h>
#include <fr/media2/Segmenter.h>

namespace fr::media2 {

  class SegmentSubscriber {
  public:
    SegmentSubscriber();
    virtual ~SegmentSubscriber();

    /**
     * If you plan to have more than one segmenter subscribed to your
     * subscriber, it might be a good idea to copy the segment
     * with segment::copy rather than try to use the data in the
     * pointer, especially if you're planning on modifying that
     * data.
     */
    
    virtual void subscribe(Segmenter *to);
    virtual void unsubscribe();

    virtual void process(const Segment::pointer&, StreamData::pointer) = 0;
  
  protected:
    
    std::vector<boost::signals2::connection> subscriptions;
  };
  
}
