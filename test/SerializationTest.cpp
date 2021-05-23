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
 * Test serialization
 */

#include <gtest/gtest.h>
#include <fr/media2.h>
#include <fr/media2/Serialization.h>
#include <fr/media2/SegmentSubscriber.h>
#include <fr/media2/Segmenter.h>
#include <sstream>
#include <memory>

using namespace fr::media2;
using namespace boost::serialization;

// Retrieves one packet for later use.
class PacketHelper : public PacketSubscriber {
protected:
  void process(const Packet::pointer &packet,
	       StreamData::pointer stream) override {
    testPacket = Packet::copy(packet);
    hasPacket = true;
    unsubscribe();
  }

public:

  PacketHelper() {}
  virtual ~PacketHelper() override {}
  Packet::pointer testPacket = Packet::create();
  bool hasPacket = false;

};

TEST(SerializationTest, serializePacket) {
  std::stringstream buffer;

  auto reader = std::make_shared<PacketReader>(TEST_FILE);
  ASSERT_GT(reader->videoStreams.size(), 0);
  PacketHelper helper;
  helper.subscribe(reader->videoStreams[0]);
  reader->sendEvent(PacketReaderStateMachine::play{});
  reader->join();

  ASSERT_TRUE(helper.hasPacket);

  {
    boost::archive::binary_oarchive ar(buffer);
    ar << *helper.testPacket;
  }
  AVPacket result;
  {
    boost::archive::binary_iarchive ar(buffer);
    ar >> result;
  }
  ASSERT_EQ(helper.testPacket->pts, result.pts);
  ASSERT_EQ(helper.testPacket->dts, result.dts);
  ASSERT_EQ(0, memcmp(helper.testPacket->data, result.data, helper.testPacket->size));
}

// SegmentHelper retrieves a segment for later use.
class SegmentHelper : public PacketSubscriber {
protected:
  void process(const Packet::pointer &packet,
	       StreamData::pointer stream) override {
    if (nullptr == segment.get()) {
     // We don't need a UUID right now

      static uuid_t empty;
      memset((void*) empty, '\0', sizeof(uuid_t));
      segment = Segment::create(empty, *stream->parameters);
      segment->append(packet);
    } else {
      if (Packet::containsIFrame(packet)) {
	// We're done!
	unsubscribe();
      } else {
	segment->append(packet);
      }
    }
  }

public:

  SegmentHelper() {}
  virtual ~SegmentHelper() override {};
  
  Segment::pointer segment;
  
};

TEST(SerializationTest, serializeSegment) {
  std::stringstream buffer;

  auto reader = std::make_shared<PacketReader>(TEST_FILE);
  SegmentHelper helper;
  ASSERT_GT(reader->videoStreams.size(), 0);
  helper.subscribe(reader->videoStreams[0]);
  reader->sendEvent(PacketReaderStateMachine::play{});
  reader->join();
  ASSERT_NE(helper.segment.get(), nullptr);
  ASSERT_GT(helper.segment->packets.size(), 0);
  {
    boost::archive::binary_oarchive ar(buffer);
    ar << *(helper.segment.get());
  }
  // Get a default construction
  Segment::pointer result = std::make_unique<Segment>();
  {
    boost::archive::binary_iarchive ar(buffer);
    ar >> *result;
  }
  ASSERT_EQ(helper.segment->pts, result->pts);
  ASSERT_EQ(helper.segment->dts, result->dts);
  ASSERT_EQ(helper.segment->npackets, result->npackets);
  ASSERT_EQ(helper.segment->packets.size(), result->packets.size());
}

// Well segments seem to work. What if I have a LOT of segments?
// 

class SubscriberTestHelper : public SegmentSubscriber {
public:
  SubscriberTestHelper() = default;
  virtual ~SubscriberTestHelper() = default;
  
  size_t segmentCount = 0l;
  int64_t dts = AV_NOPTS_VALUE;

protected:
  void process(const Segment::pointer& segment) override {
    segmentCount++;
    if (!segment->empty()) {
      if (!Packet::containsIFrame(segment->packets[0])) {
	throw std::runtime_error("First packet in segment is not an IFrame");
      }
    }
    // This would indicate I'm not copying packets correctly
    if (dts >= segment->dts) {
      throw std::runtime_error("Segment DTS is less than or equal to previous one.");
    }
    dts = segment->dts;
  }
  
};

// This just tests breaking up the segments, but it kind of fits here.

TEST(SerializationTest, multiSegment) {
  auto reader = std::make_shared<PacketReader>(TEST_FILE);
  auto videoSegmenter = std::make_shared<Segmenter>();
  auto audioSegmenter = std::make_shared<Segmenter>();
  ASSERT_GT(reader->videoStreams.size(), 0l);
  ASSERT_GT(reader->audioStreams.size(), 0l);
  videoSegmenter->subscribe(reader->videoStreams[0]);
  audioSegmenter->subscribe(reader->audioStreams[0]);

  auto audioHelper = std::make_shared<SubscriberTestHelper>();
  auto videoHelper = std::make_shared<SubscriberTestHelper>();
  audioHelper->subscribe(audioSegmenter.get());
  videoHelper->subscribe(videoSegmenter.get());

  reader->sendEvent(PacketReaderStateMachine::play{});
  reader->join();
  
  ASSERT_GT(audioHelper->segmentCount, 0l);
  ASSERT_GT(videoHelper->segmentCount, 0l);
}
