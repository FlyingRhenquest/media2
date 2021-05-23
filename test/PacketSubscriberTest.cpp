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

#include <gtest/gtest.h>
#include <fr/media2/Stream.h>
#include <fr/media2/StreamData.h>
#include <fr/media2/Packet.h>
#include <fr/media2/PacketSubscriber.h>

using fr::media2::Stream;
using fr::media2::StreamData;
using fr::media2::PacketSubscriber;
using fr::media2::Packet;

class DummySubscriber : public PacketSubscriber {
protected:
  void process(const Packet::pointer &packet, StreamData::pointer data) override {
    failed = false;
  }
public:
  virtual ~DummySubscriber() override = default;
  // process sets this false
  bool failed = true;
};

TEST(PacketSubscriberTest, connectivity) {
  Stream::pointer source = std::make_unique<Stream>();
  DummySubscriber subscriber;
  // source.data will be a shared ptr to a nullptr, which is fine
  // for this test. We will need a dummy packet, though...
  Packet::pointer packet = Packet::create();
  subscriber.subscribe(source);
  source->forward(packet);
  ASSERT_FALSE(subscriber.failed);
}
