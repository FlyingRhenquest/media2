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
 * A cache for streams. This is intended to be used with Segment and
 * its publishers so a remote host can assemble a stream for decoding.
 *
 * * Cache entries are indexed by jobId (uuid_t)
 * * Cache entries expire after a set amount of time
 * * If you query the cache using a segment pointer and it's a miss,
 *   the StreamCache will automatically assemble a Stream for you
 *   and return it. Note StreamCache-assembled streams' data->stream
 *   will be nullptr. This generally shouldn't be a problem, but
 *   always be sure to check stream->data->stream for nullptr
 *   before trying to use it.
 */

#pragma once

#include <atomic>
#include <chrono>
#include <fr/media2/Segment.h>
#include <fr/media2/Stream.h>
#include <fr/media2/StreamData.h>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <uuid.h>

namespace fr::media2 {

  // A data record for stream cache
  class StreamCacheRecord {
  public:
    using pointer = std::shared_ptr<StreamCacheRecord>;
    // A stream
    Stream::pointer stream;
    // The time at which the cache entry expires
    std::chrono::system_clock::time_point expires;
  };
  
  class StreamCache {
  public:
    
    StreamCache(long expSeconds = 600, long cleanupFreq = 10);
    ~StreamCache();

    Stream::pointer get(Segment*);
    Stream::pointer get(uuid_t jobId);

  private:
    // Number of seconds cache entries last
    long expSeconds;
    // Expire entries every cleanupFreq seconds
    long cleanupFreq;
    std::mutex cacheMutex;
    std::atomic<bool> shutdownFlag = false;
    std::unordered_map<std::string, StreamCacheRecord::pointer> cache;

    std::thread processingThread;
    
    // Runs in a thread. Wakes up every so often to expire
    // cache records
    void expireEntries();
    
  };
  
}
