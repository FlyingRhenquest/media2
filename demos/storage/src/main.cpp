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

#include <Storage.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {

  // Address this services subscribes to
  // OUTGOING_SEGMENT_ADDRESS from router
  std::string outgoingSegmentAddress={"OUTGOING_SEGMENT_ADDRESS"};
  // Address for query by stream ID
  // ENV: QUERY_BY_STREAM_ADDRESS
  std::string queryByStreamAddress={"QUERY_BY_STREAM_ADDRESS"};
  std::string errors;

  if (char *osa = getenv(outgoingSegmentAddress.c_str())) {
    outgoingSegmentAddress = osa;
  } else {
    errors = outgoingSegmentAddress;
  }
  if (char *qbs = getenv(queryByStreamAddress.c_str())) {
    queryByStreamAddress = qbs;
  } else {
    errors.append(errors.empty() ? queryByStreamAddress : std::string{", "} + queryByStreamAddress);
  }
  if (errors.empty()) {
    fr::media2::demos::Storage worker{outgoingSegmentAddress, queryByStreamAddress};
    worker.join();
  } else {
    std::cout << "These env variables must be set prior to running this program: " << errors << std::endl;
  }
}
