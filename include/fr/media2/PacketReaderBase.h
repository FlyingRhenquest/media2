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
 * You don't need to look here!
 *
 * Oh, you really wanna? Ok. This base class defines some state machine
 * stuff for packet reader using boost::sml. End user doesn't really
 * need to see it.
 */

#pragma once

#include <boost/sml.hpp>
#include <iostream>

namespace fr {
  namespace media2 {

    class PacketReaderBase {
      friend class PacketReaderStateMachine;
    protected:
      virtual bool open() = 0;
      virtual void close() =0 ;
      virtual void reset() = 0 ;
      virtual void unpause() =0;
      virtual void join() =0;
      virtual void process() =0;
      virtual void shutdown() = 0;
      virtual void error(const std::string &msg) = 0;
      virtual void eof() = 0;
      
    public:

      PacketReaderBase() = default;
      virtual ~PacketReaderBase() = default;
      PacketReaderBase(const PacketReaderBase &copy) = delete;

    };

    // Set up a boost::sml state machine
    class PacketReaderStateMachine {
    public:
      
      // Dependencies
      struct Sender {
	Sender(PacketReaderBase* owner) : owner(owner) {}
	PacketReaderBase* owner;
	template<class TMsg>
	constexpr void send(const TMsg& msg) {}
      };
      // Events
      struct open{};
      struct open_success{};
      struct open_error{ std::string msg; };
      struct play {}; // Initiates PacketReader::process or unpauses
      struct pause{}; // Pauses packet reading
      struct eof{}; // Signals end of media file
      struct reset{}; // Closes and re-opens the source	
      // TODO: Add seek event with timestamp

      // Actions
      static constexpr auto send_open_error = [](const auto &event, Sender &s) { s.owner->error(event.msg); };
      static constexpr auto send_open = [](Sender &s){ s.owner->open(); };
      static constexpr auto start_processing = [](Sender &s) { s.owner->process(); };
      static constexpr auto unpause = [](Sender &s) { s.owner->unpause(); };
      static constexpr auto send_reset = [](Sender &s) { s.owner->reset(); };
      static constexpr auto send_eof = [](Sender &s) { s.owner->eof(); };
	
      // machine
      struct PlayerState {
	auto operator()() const {
	  // It's in scope, it's fine!
	  using namespace boost::sml;
	  return make_transition_table(
	    *"ready"_s + event<open> / send_open                         = "opening"_s,
	    "opening"_s + event<open_success>                            = "opened"_s,
	    "opening"_s + event<open_error> / send_open_error            = X, // Failed
	    "opened"_s + event<play> / start_processing                  = "playing"_s,
	    "opened"_s + event<reset> / send_reset                       = "done"_s,
	    "playing"_s + event<pause>                                   = "paused"_s,
	    "playing"_s + event<reset> / send_reset                      = "done"_s,
	    "playing"_s + event<eof> / send_eof                          = "done"_s,
	    "paused"_s + event<play> / unpause                           = "playing"_s,
	    "paused"_s + event<pause> / unpause                          = "playing"_s,
	    "paused"_s + event<reset> / send_reset                       = "done"_s,
	    "done"_s + event<reset> / send_reset                         = "done"_s,
	    "done"_s + event<open> / send_open                           = "opening"_s
	  );
	}
      }; // Struct playerstate
    }; // class PacketReaderStateMachine
    
  }
}
