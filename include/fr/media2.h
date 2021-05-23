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

#pragma once
#include <fr/media2/Decoder.h>
#include <fr/media2/Encoder.h>
#include <fr/media2/Frame.h>
#include <fr/media2/Frame2Mat.h>
#include <fr/media2/FrameSource.h>
#include <fr/media2/FrameSubscriber.h>
#include <fr/media2/Muxer.h>
#include <fr/media2/Packet.h>
#include <fr/media2/PacketReader.h>
#include <fr/media2/PacketSubscriber.h>
#include <fr/media2/Resampler.h>
#include <fr/media2/Scaler.h>
#include <fr/media2/Segment.h>
#include <fr/media2/Segmenter.h>
#include <fr/media2/SegmentSubscriber.h>
#include <fr/media2/Serialization.h>
#include <fr/media2/Stream.h>
#include <fr/media2/StreamData.h>
#include <fr/media2/ZmqSegmentPublisher.h>
#include <fr/media2/ZmqSegmentSubscriber.h>
