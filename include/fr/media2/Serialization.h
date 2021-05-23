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
 * Set up serialization of a couple of ffmpeg's API objects -- packet and
 * stream.
 */

#pragma once

extern "C" {
#include <libavcodec/packet.h>
#include <libavcodec/codec_par.h>
#include <libavformat/avformat.h>
}

#include <fr/media2/Packet.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/binary_object.hpp>

// Packet side data
template<class Archive>
void save(Archive &ar, const AVPacketSideData &d, const unsigned int version) {
  ar << d.size;
  ar << d.type;
  ar << boost::serialization::make_binary_object(d.data, d.size);
}

template<class Archive>
void load(Archive &ar, AVPacketSideData &d, const unsigned int version) {
  ar >> d.size;
  ar >> d.type;
  d.data = (uint8_t*) malloc(sizeof(uint8_t) * d.size);
  ar >> boost::serialization::make_binary_object(d.data, d.size);
}

template<class Archive>
void serialize(Archive &ar, AVPacketSideData &d, const unsigned int version) {
  boost::serialization::split_free(ar, d, version);
}

// Packet
  
template<class Archive>
void save(Archive &ar, const AVPacket &packet, const unsigned int version) {
  ar << packet.pts;
  ar << packet.dts;
  ar << packet.size;
  ar << boost::serialization::make_binary_object(packet.data, packet.size);
  ar << packet.stream_index;
  ar << packet.flags;
  ar << packet.side_data_elems;
  if (packet.side_data_elems && (nullptr != packet.side_data)) {
    ar << *packet.side_data;
  }
  ar << packet.duration;
  ar << packet.pos;
}

template<class Archive>
void load(Archive &ar, AVPacket &packet, const unsigned int version) {
  ar >> packet.pts;
  ar >> packet.dts;
  ar >> packet.size;
  packet.data = (uint8_t*) malloc(sizeof(uint8_t) * packet.size);
  ar >> boost::serialization::make_binary_object(packet.data, packet.size);
  ar >> packet.stream_index;
  ar >> packet.flags;
  ar >> packet.side_data_elems;
  if (packet.side_data_elems > 0) {
    packet.side_data = (AVPacketSideData *) malloc(sizeof(AVPacketSideData));
    ar >> packet.side_data;
  } else {
    packet.side_data = nullptr;
  }
  ar >> packet.duration;
  ar >> packet.pos;
}

template<class Archive>
void serialize(Archive &ar, AVPacket &packet, const unsigned int version) {
  boost::serialization::split_free(ar, packet, version);
}

template<class Archive>
void serialize(Archive &ar, const fr::media2::Packet::pointer &packet, const unsigned int version) {
  boost::serialization::split_free(ar, (AVPacket &)(*packet), version);
}

// AVCodecParameters

template<class Archive>
void save(Archive &ar, const AVCodecParameters& par, const unsigned int version) {
  ar << par.codec_type;
  ar << par.codec_id;
  ar << par.codec_tag;
  ar << par.extradata_size;
  if (par.extradata_size) {
    ar << boost::serialization::make_binary_object(par.extradata, par.extradata_size);
  }
  ar << par.format;
  ar << par.bit_rate;
  ar << par.bits_per_coded_sample;
  ar << par.bits_per_raw_sample;
  ar << par.profile;
  ar << par.level;
  ar << par.width;
  ar << par.height;
  ar << par.sample_aspect_ratio.num;
  ar << par.sample_aspect_ratio.den;
  ar << par.field_order;
  ar << par.color_range;
  ar << par.color_primaries;
  ar << par.color_trc;
  ar << par.color_space;
  ar << par.chroma_location;
  ar << par.video_delay;
  ar << par.channel_layout;
  ar << par.channels;
  ar << par.sample_rate;
  ar << par.block_align;
  ar << par.frame_size;
  ar << par.initial_padding;
  ar << par.trailing_padding;
  ar << par.seek_preroll;
}

template<class Archive>
void load(Archive &ar, AVCodecParameters &par, const unsigned int version) {
  ar >> par.codec_type;
  ar >> par.codec_id;
  ar >> par.codec_tag;
  ar >> par.extradata_size;
  if (par.extradata_size) {
    par.extradata = (uint8_t *) malloc(sizeof(uint8_t) * par.extradata_size);
    ar >> boost::serialization::make_binary_object(par.extradata, par.extradata_size);
  }
  ar >> par.format;
  ar >> par.bit_rate;
  ar >> par.bits_per_coded_sample;
  ar >> par.bits_per_raw_sample;
  ar >> par.profile;
  ar >> par.level;
  ar >> par.width;
  ar >> par.height;
  ar >> par.sample_aspect_ratio.num;
  ar >> par.sample_aspect_ratio.den;
  ar >> par.field_order;
  ar >> par.color_range;
  ar >> par.color_primaries;
  ar >> par.color_trc;
  ar >> par.color_space;
  ar >> par.chroma_location;
  ar >> par.video_delay;
  ar >> par.channel_layout;
  ar >> par.channels;
  ar >> par.sample_rate;
  ar >> par.block_align;
  ar >> par.frame_size;
  ar >> par.initial_padding;
  ar >> par.trailing_padding;
  ar >> par.seek_preroll;
}


template<class Archive>
void serialize(Archive &ar, AVCodecParameters &par, const unsigned int version) {
  boost::serialization::split_free(ar, par, version);
}
