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

#include <fr/media2/StreamCache.h>

namespace fr::media2 {

  StreamCache::StreamCache(long expSeconds, long cleanupFreq) : expSeconds(expSeconds), cleanupFreq(cleanupFreq) {
    processingThread = std::thread([this]{this->expireEntries();});    
  }

  StreamCache::~StreamCache() {
    shutdownFlag = true;
    if (processingThread.joinable()) {
      processingThread.join();
    }
  }

  Stream::pointer StreamCache::get(Segment* seg) {
    // TODO: Throw together a utils and set up uuid_to_str in it.
    char uuidStr[40];
    memset(uuidStr, '\0', sizeof(uuidStr));
    uuid_unparse(seg->jobId, uuidStr);
    std::string idStr{uuidStr};

    std::lock_guard<std::mutex> lock(cacheMutex);
    if (cache.contains(idStr)) {
      // Refresh expiry
      cache.at(idStr)->expires = std::chrono::system_clock::now() +
	std::chrono::system_clock::duration(expSeconds);
      return cache.at(idStr)->stream;
    } else {
      auto rec = std::make_shared<StreamCacheRecord>();
      rec->stream = std::make_shared<Stream>(seg);
      rec->expires = std::chrono::system_clock::now() + std::chrono::system_clock::duration(expSeconds);
      cache[idStr] = rec;
      return rec->stream;
    }
  }

  // jobId version of get returns a nullptr if we get a cache miss
  Stream::pointer StreamCache::get(uuid_t jobId) {
    char uuidStr[40];
    memset(uuidStr, '\0', sizeof(uuidStr));
    uuid_unparse(jobId, uuidStr);
    std::string idStr{uuidStr};
    Stream::pointer ret;

    std::lock_guard<std::mutex> lock(cacheMutex);
    if (cache.contains(idStr)) {
      cache.at(idStr)->expires = std::chrono::system_clock::now() +
	std::chrono::system_clock::duration(expSeconds);
      ret = cache.at(idStr)->stream;
    }
    return ret;
  }

  void StreamCache::expireEntries() {
    while(!shutdownFlag) {
      {
	std::lock_guard<std::mutex> lock(cacheMutex);
	for (const auto& [key, value] : cache) {
	  if (shutdownFlag) {
	    return;
	  }
	  if (std::chrono::system_clock::now() > value->expires) {
	    cache.erase(key);
	}
	}
      }
      std::this_thread::sleep_for(std::chrono::seconds(cleanupFreq));
    }
  }
  
}
