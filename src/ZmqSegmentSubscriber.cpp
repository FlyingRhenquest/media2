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

#include <fr/media2/ZmqSegmentSubscriber.h>

namespace fr::media2 {

  ZmqSegmentSubscriber::ZmqSegmentSubscriber(std::string listenAddress) : socket{context, zmq::socket_type::sub} {
    socket.set(zmq::sockopt::subscribe, "");
    socket.bind(listenAddress); 
  }

  ZmqSegmentSubscriber::~ZmqSegmentSubscriber() {
    close();
    join();
  }

  void ZmqSegmentSubscriber::process() {
    processingThread = std::thread([this]{ this->processPrivately(); });
  }

  void ZmqSegmentSubscriber::close() {
    // May need to make this std::atomic if it doesn't behave.
    shutdownPlox.store(true);
  }

  void ZmqSegmentSubscriber::join() {
    if (processingThread.joinable()) {
      processingThread.join();
    }
  }

  void ZmqSegmentSubscriber::processPrivately() {
    zmq::active_poller_t poller;
    
    poller.add(socket, zmq::event_flags::pollin, [this](zmq::event_flags flags) {
      std::stringstream buffer;
      zmq::multipart_t multimsg;
      uuid_t jobId;
      AVMediaType mt;
      int width;
      int height;
      multimsg.recv(socket);
      auto msgiter = multimsg.begin();
      zmq::message_t uuidMsg = *msgiter++;
      zmq::message_t mediaTypemessage = *msgiter++;
      zmq::message_t widthMsg = *msgiter++;
      zmq::message_t heightMsg = *msgiter++;
      zmq::message_t bufferMsg = *msgiter;
      
      char idMsgStr[40];
      memcpy(jobId, uuidMsg.data(), uuidMsg.size());
      uuid_unparse(jobId, idMsgStr);
      memcpy(&mt, mediaTypemesage.data(), mediaTypemessage.size());
      memcpy(&width, widthMsg.data(), widthMsg.size());
      memcpy(&height, heightMsg.data(), heightMsg.size());
      buffer << bufferMsg.to_string();
      this->receivedSegment(buffer, jobId, mt, width, height);
    });
    // This can be fairly long as we only want to pull the message off the
    // transport and dispatch it to listeners. The only reason to make it
    // shorter is for close latency, but the shorter it is, the more time
    // you'll spend checking for empty messages.
    std::chrono::milliseconds timeout(250);
    while(!shutdownPlox.load()) {
      // This is all I have to do here. If it times out, we ignore it and
      // wait again. The lambda I added up above will take care of any
      // messaging that needs done.
      auto nsocks = poller.wait(timeout);
    }
  }
  
}
