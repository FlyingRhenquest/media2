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

#include <fr/media2/Stream.h>
#include <fr/media2/Segment.h>
#include <uuid.h>

namespace fr {
  namespace media2 {

    Stream::Stream(Segment* seg) {
      char uuidstr[40];
      memset(uuidstr, '\0', sizeof(uuidstr));
      uuid_unparse(seg->jobId, uuidstr);
      std::string jobstr{uuidstr};
      // Segment and Stream don't know filename, so use
      // the job ID from the segment instead (You can
      // parse it back to a uuid with uuid_parse)
      data = std::make_shared<StreamData>(seg, jobstr);
    }
    
    void Stream::forward(const Packet::pointer& packet) {
      packets(packet, data);
    }
  }
}
