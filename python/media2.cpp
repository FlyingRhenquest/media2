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
 * Pybind11 API for media2 library
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <fr/media2.h>

PYBIND11_MODULE(fr_media2, m) {

  pybind11::class_<fr::media2::Stream>(m, "Stream")
    .def_readonly("data", &fr::media2::Stream::data)
    ;

  pybind11::class_<fr::media2::StreamData>(m, "StreamData")
    .def_readonly("filename", &fr::media2::StreamData::filename)
    .def_readonly("mediaType", &fr::media2::StreamData::mediaType)
    ;


  // Reads compressed packets from a media source
  pybind11::class_<fr::media2::PacketReader>(m, "PacketReader")
    .def(pybind11::init<std::string>(), pybind11::arg("filename"))
    .def("play", [](fr::media2::PacketReader &reader) ->void { reader.sendEvent(fr::media2::PacketReaderStateMachine::play{}); })
    .def("pause", [](fr::media2::PacketReader &reader) -> void { reader.sendEvent(fr::media2::PacketReaderStateMachine::pause{}); })
    .def("reset", [](fr::media2::PacketReader &reader) -> void { reader.sendEvent(fr::media2::PacketReaderStateMachine::reset{}); })
    .def_readonly("filename", &fr::media2::PacketReader::filename)
    .def("join", &fr::media2::PacketReader::join)
    .def_readonly("streams", &fr::media2::PacketReader::streams)
    .def_readonly("audioStreams", &fr::media2::PacketReader::audioStreams)
    .def_readonly("videoStreams", &fr::media2::PacketReader::videoStreams)
    ;

  // PacketSubscriber classes subscribe to streams (Like the ones in PacketReader)
  pybind11::class_<fr::media2::PacketSubscriber>(m, "PacketSubscriber")
    .def("subscribe", &fr::media2::PacketSubscriber::subscribe)
    .def("unsubscribe", &fr::media2::PacketSubscriber::unsubscribe)
    ;

  // Decoder is a PacketSubscriber
  pybind11::class_<fr::media2::Decoder, fr::media2::PacketSubscriber>(m, "Decoder")
    ;

  // FrameSubscribers subscribe to FrameSources (Like Decoder)
  pybind11::class_<fr::media2::FrameSubscriber>(m, "FrameSubscriber")
    .def("subscribe", static_cast<void (fr::media2::FrameSubscriber::*)(fr::media2::FrameSource*)>(&fr::media2::FrameSubscriber::subscribe))
    .def("unsubscribe", &fr::media2::FrameSubscriber::unsubscribe)
    ;

  pybind11::class_<fr::media2::Encoder, fr::media2::FrameSubscriber>(m, "Encoder")
    .def(pybind11::init<std::string>(), pybind11::arg("codecName") = "")
    .def_readonly("stream", &fr::media2::Encoder::stream)
    ;

  // Muxer is a packet subscriber (subscribes to streams)
  // If format isn't specified, will try to guess from the filename
  pybind11::class_<fr::media2::Muxer, fr::media2::PacketSubscriber>(m, "Muxer")
    .def(pybind11::init<std::string, std::string>(), pybind11::arg("filename"), pybind11::arg("format") = "")
    .def("subscribe", &fr::media2::Muxer::subscribe)
    .def("unsubscribe", &fr::media2::Muxer::unsubscribe)
    // open opens file for writing. Once you do this, you can not subscribe any more streams
    // unless you really want a segfault
    // TODO: Make the muxer refuse to subscribe to any more streams once open is called
    .def("open", &fr::media2::Muxer::open)
    // Close writes the trailer and closes the file. You should generally close your file
    // prior to your program terminating
    .def("close", &fr::media2::Muxer::close)
    ;
}
