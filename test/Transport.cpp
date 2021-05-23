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
 * Test transporting packets via zmq
 */

#include <fr/media2.h>
#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <chrono>

using namespace fr::media2;

TEST(Transport, lowLevelSegments) {
  PacketReader reader{TEST_FILE};
  std::string addr("tcp://127.0.0.1:2713");
  std::vector<std::shared_ptr<ZmqSegmentPublisher>> publishers;
  std::vector<std::shared_ptr<Segmenter>> segmenters;
  long segsRecvd = 0l;
  std::vector<std::string> uuids;
  
  // Set up and run receiver
  ZmqSegmentSubscriber subscriber(addr);
  subscriber.receivedSegment.connect([&segsRecvd, &uuids](std::stringstream& buffer, uuid_t uuid) -> void {
    char uuidChars[37];
    memset(uuidChars, '\0', sizeof(uuidChars));
    if (!uuid_is_null(uuid)) {
      uuid_unparse(uuid, uuidChars);
      std::string uuidString(uuidChars);
      if (std::find(begin(uuids), end(uuids), uuidString) == std::end(uuids)) {
	uuids.push_back(uuidString);
      }
    }
    segsRecvd++;
  });

  // Kick off receiving
  subscriber.process();
  // std::this_thread::sleep_for(std::chrono::seconds(2));
  
  for(auto stream : reader.streams) {
    auto segmenter = std::make_shared<Segmenter>();
    auto publisher = std::make_shared<ZmqSegmentPublisher>(addr);
    segmenter->subscribe(stream);
    publisher->subscribe(segmenter.get());
    segmenters.push_back(segmenter);
    publishers.push_back(publisher);
  }

  // Send all packets
  reader.sendEvent(PacketReaderStateMachine::play{});
  reader.join();

  // Flush segmenters
  for (auto segmenter : segmenters) {
    segmenter->flush();
  }

  // Release ownership of segmenters and publishers
  segmenters.clear();
  publishers.clear();

  // Shut down subscriber (If we let it just destruct
  // we could lose some messages)
  subscriber.close();
  subscriber.join();
  
  ASSERT_GT(segsRecvd, 0l);
  ASSERT_GT(uuids.size(), 0l);

  // Output the UUIDs for fun and profit
  std::cout << "Job IDs received:" << std::endl;
  for (auto uuid : uuids) {
    std::cout << uuid << std::endl;
  }


}

