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
#include <fr/media2/Packet.h>

using fr::media2::Packet;

// Just create and destroy some packets, make sure the address sanitizer
// doesn't yell at us.

TEST(PacketTest, create) {
  Packet::pointer pkt = Packet::create();
}

TEST(PacketTest, copy) {
  Packet::pointer pkt = Packet::create();
  Packet::pointer copy = Packet::copy(pkt);
}
