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
 * Test that the encoder can encode frames without error.
 */

#include <gtest/gtest.h>
#include <fr/media2/Decoder.h>
#include <fr/media2/Encoder.h>
#include <fr/media2/PacketReader.h>

using namespace fr::media2;

TEST(EncoderTest, encodeFrames) {

  PacketReader reader(TEST_FILE);
  // Nothing indicates what a decoder decodes until
  // it receives its first packets.
  Decoder audioDecoder;
  Decoder videoDecoder;
  ASSERT_GT(reader.streams.size(), 0);
  ASSERT_GT(reader.audioStreams.size(), 0);
  ASSERT_GT(reader.videoStreams.size(), 0);
  audioDecoder.subscribe(reader.audioStreams[0]);
  videoDecoder.subscribe(reader.videoStreams[0]);
  // You have to specify codec for encoders
  // It's up to you to make sure they're in
  // the correct format for the codec.
  Encoder videoEncoder("libx264");
  Encoder audioEncoder("aac");
  videoEncoder.subscribe(videoDecoder);
  audioEncoder.subscribe(audioDecoder);
  int videoPacketCount = 0;
  int audioPacketCount = 0;
  videoEncoder.stream->packets.connect(
    [&videoPacketCount](const auto& packet, const auto& stream) {
      videoPacketCount++;
    });
  audioEncoder.stream->packets.connect(
    [&audioPacketCount](const auto& packet, const auto& stream) {
      audioPacketCount++;
    });
  
  reader.sendEvent(PacketReaderStateMachine::play{});
  reader.join();
  
  ASSERT_GT(videoPacketCount, 0);
  ASSERT_GT(audioPacketCount, 0);
}
