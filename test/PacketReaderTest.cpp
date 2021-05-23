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
 * Verify packet reader reads packets as expected, what happens
 * when you send it unexpected events, all those things.
 */

#include <gtest/gtest.h>
#include <chrono>
#include <fr/media2/PacketReader.h>
#include <fr/media2/PacketSubscriber.h>
#include <mutex>
#include <thread>

using namespace fr::media2;

// Set up a fixture for a reader
class PacketReaderTest : public ::testing::Test {
public:
  std::shared_ptr<PacketReader> reader;

  void SetUp() {
    reader = std::make_shared<PacketReader>(TEST_FILE);
  }

  void TearDown() {
    reader.reset();
  }
};

TEST_F(PacketReaderTest, open) {
  using namespace boost::sml;
  ASSERT_TRUE(reader->state.is("opened"_s));
}

TEST_F(PacketReaderTest, countPackets) {
  using namespace boost::sml;
  ASSERT_TRUE(reader->state.is("opened"_s));
  ASSERT_GT(reader->streams.size(), 0);
  int packets = 0;
  for (auto stream : reader->streams) {
    stream->packets.connect(
      [&packets](const auto& packet, auto stream) {
	packets++;
      });
  }
  reader->sendEvent(PacketReaderStateMachine::play{});
  reader->join();
  ASSERT_GT(packets, 0);
}

// So I'm testing two things in this next one;
// I'm testing that pause works by writing a
// subscriber that receives a packet, sends
// a pause and then unsubscribes. I'm also
// making sure subscribers work. I'll need
// a class for that...

class PacketReaderTestSubscriber : public PacketSubscriber {
protected:
  void process(const Packet::pointer &packet,
	       StreamData::pointer stream) override {
    if (nullptr != owner.get()) {
      owner->sendEvent(PacketReaderStateMachine::pause{});
      unsubscribe();
    } else {
    }
  }
public:
  // I'll need owner to send the pause signal
  std::shared_ptr<PacketReader> owner;

  PacketReaderTestSubscriber(std::shared_ptr<PacketReader> owner)
    : owner(owner) {
  }
};

TEST_F(PacketReaderTest, subscribeAndPause) {
  using namespace boost::sml;
  ASSERT_TRUE(reader->state.is("opened"_s));
  ASSERT_GT(reader->streams.size(), 0);
  
  PacketReaderTestSubscriber subscriber(reader);
  // I can subscribe to either stream for this test
  subscriber.subscribe(reader->streams[0]);
  reader->sendEvent(PacketReaderStateMachine::play{});
  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  ASSERT_TRUE(reader->state.is("paused"_s));
  // We can send a play or a pause to start playing again
  reader->sendEvent(PacketReaderStateMachine::pause{});
  ASSERT_FALSE(reader->state.is("paused"_s));
  reader->join();
}
