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
 * Describes one "job", which is one video and whatever streams it contains,
 * as well as resolution of the job.
 *
 */

#pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/serialization.hpp>
#include <sstream>
#include <string>
#include <uuid.h>
#include <vector>

namespace fr::media2::demos {

  class Job {
  public:
    std::string filename;
    std::string jobId;
    // Resolution should be 4K, 1080 or 720.
    std::string resolution;
    std::vector<std::string> streamIds;

  private:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive &ar, const unsigned int version) {
      ar & filename;
      ar & jobId;
      ar & resolution;
      ar & streamIds;
    }
  };
  
}
