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

#include <filesystem>
#include <gtest/gtest.h>
#include <chrono>
#include <memory>
#include <vector>
#include <iostream>

#include <fr/media2/Decoder.h>
#include <fr/media2/PacketReader.h>
#include <fr/media2/Encoder.h>
#include <fr/media2/Muxer.h>

using namespace fr::media2;

// Encode video end to end and verify that the same number
// of packets were written as read

// I could just read the packets straight into a couple of encoders
// and from there to the muxer, but I actually want to
// decode and re-encode the video, too.

TEST(MuxerTest, endToEnd) {
  std::string outputFile("endToEnd.mp4");
  if (std::filesystem::exists(outputFile)) {
    std::filesystem::remove(outputFile);
  }
  std::vector<std::shared_ptr<Decoder>> decoders;
  std::vector<std::shared_ptr<Encoder>> encoders;
  PacketReader reader(TEST_FILE);
  // Add decoders to streams
  for (auto stream : reader.streams) {
    std::cout << "Subscribing decoder to reader stream " << stream->data->stream->index << std::endl;
    auto decoder = std::make_shared<Decoder>();
    decoder->subscribe(stream);
    decoders.push_back(decoder);
  }
  // Add encoders to decoders
  for (auto decoder : decoders) {
    // We'll try to have the encoder guess what
    // codec it's encoding to based on the packets
    // it receives.
    std::cout << "Subscribing encoder to decoder" << std::endl;
    auto encoder = std::make_shared<Encoder>();
    encoder->subscribe(decoder.get());
    encoders.push_back(encoder);
  }
  Muxer muxer(outputFile);
  // Add encoders to muxer
  for (auto encoder : encoders) {
    std::cout << "Subscribing muxer to encoder" << std::endl;
    muxer.subscribe(encoder->stream);
  }
  // Now all we have to do is stream the packets
  // through the graph we've built
  reader.sendEvent(PacketReaderStateMachine::play{});
  reader.join();
  muxer.close();

  ASSERT_TRUE(std::filesystem::exists(outputFile));
}
