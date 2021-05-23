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
 * Tests decoder. I don't want to look at the audio/video data in this
 * particular test, I just want to make sure I get a steady bunch
 * of frames out of some decoders.
 */

#include <gtest/gtest.h>
#include <chrono>
#include <fr/media2/Decoder.h>
#include <fr/media2/PacketReader.h>
#include <fr/media2/PacketSubscriber.h>
#include <fr/media2/FrameSubscriber.h>
#include <memory>
#include <vector>

using namespace fr::media2;

class DecoderTest : public ::testing::Test {

public:
  std::shared_ptr<PacketReader> reader;
  
  void SetUp() {
    reader = std::make_shared<PacketReader>(TEST_FILE);
  }

  void TearDown() {
    reader.reset();
  }
};

class FrameProcessor : public FrameSubscriber {
protected:
  void process(Frame::const_pointer frame, StreamData::pointer stream) override {
    nframes++;
  }
  
public:
  virtual ~FrameProcessor() override = default;
  long nframes = 0;
};

TEST_F(DecoderTest, decodeFrames) {
  std::vector<std::shared_ptr<Decoder>> decoders;
  std::vector<std::shared_ptr<FrameProcessor>> processors;
  for (auto stream : reader->streams) {
    auto decoder = std::make_shared<Decoder>();
    decoder->subscribe(stream);
    auto processor = std::make_shared<FrameProcessor>();
    processor->subscribe(decoder.get());
    decoders.push_back(decoder);
    processors.push_back(processor);
  }
  reader->sendEvent(PacketReaderStateMachine::play{});
  reader->join();
  for (auto processor : processors) {
    ASSERT_GT(processor->nframes, 0);
  }
}
