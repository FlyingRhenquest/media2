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
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <chrono>
#include <uuid.h>

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
  subscriber.receivedSegment.connect([&segsRecvd, &uuids](std::stringstream& buffer, uuid_t uuid, AVMediaType mt, int width, int height) -> void {
    char uuidChars[UUID_STR_LEN + 1];
    memset(uuidChars, '\0', sizeof(char) * UUID_STR_LEN + sizeof(char));
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

/**
 * Transport reassembly -- Muxer really isn't geared toward multi-threaded
 * input, which is going to be a problem eventually. But I can test my
 * transport reassembly anyway by running my SegmentUnpacker with one
 * thread. That should be linear (enough) not to overwhelm
 * av_interleaved_write_frame. Eventually I'll need to write a smarter
 * (probably threaded) muxer.
 */

TEST(Transport, reassembly) {
  PacketReader reader{TEST_FILE};
  std::string addr("tcp://127.0.0.1:2714");
  std::vector<std::shared_ptr<ZmqSegmentPublisher>> publishers;
  std::vector<std::shared_ptr<Segmenter>> segmenters;
  std::string outputFile("reassembly.mp4");
  if (std::filesystem::exists(outputFile)) {
    std::filesystem::remove(outputFile);
  }
  // Set up muxer
  Muxer muxer(outputFile);
  // Get segment subscriber waiting
  ZmqSegmentSubscriber subscriber(addr);
  // If I make a segmentunpacker with one thread, processing should
  // be linear (or at least linear enough.)
  SegmentUnpacker unpacker(1, [&muxer](Stream::pointer stream) {
    std::cout << "Subscribing muxer to " << stream->data->filename << std::endl;
    try {
      muxer.subscribe(stream);
    } catch (std::exception &e) {
      // I'm actually expecting an exception here, it's fine.
      std::cout << e.what() << std::endl;
    }
  });
  unpacker.subscribe(&subscriber);
  subscriber.process();

  // Set up publishers
  for(auto stream : reader.streams) {
    auto segmenter = std::make_shared<Segmenter>();
    auto publisher = std::make_shared<ZmqSegmentPublisher>(addr);
    segmenter->subscribe(stream);
    publisher->subscribe(segmenter.get());
    segmenters.push_back(segmenter);
    publishers.push_back(publisher);
  }

  reader.process();
  reader.join();
  for (auto segmenter : segmenters) {
    segmenter->flush();
  }
  segmenters.clear();
  publishers.clear();
}

