=Media2 Project=

I've redesigned the original media library:

 * I pay attention to AVStreams in the FormatContext now
 * My objects wrap the AVStreams and can be subscribed to
   in order to receive data from the individual streams
 * Split reading packets out from decoding them
 * Added a decoder, encoder and muxer (Along with resampler
 * and rescaler.)
 * Frame2Mat made its way over so you can split off frames
   and send them to OpenCV.
 * Introduced Segmenters, which subscribe to a stream and
   create serializable segments which consist of an IFrame
   (for video data) and all frames until the next IFrame.
 * Added a ZeroMQ transport for segments, see
   ZmqSegmentPublisher and ZmqSegmentSubscriber.

At the moment I can't guarantee there aren't memory leaks,
but it should be pretty solid. I also haven't gotten around
to bulletproofing it yet, so doing unexpected things could
still cause it to crash.

==Build Dependencies==

 * A fairly recent CMake
 * The ffmpeg dev libraries (You may need to build it yourself.)
 * boost::signals2
 * boost::sml (https://github.com/boost-ext/sml)
 * OpenCV (You'll probably have to build it yourself.)
 * A recent ZeroMQ (libzmq) (You may need to build it yourself.)
 * cppzmq (You shouldn't need to build it yourself.)