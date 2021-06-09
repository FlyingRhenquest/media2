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

#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <filesystem>
#include <stdlib.h>
#include <sstream>
#include <string>
#include <uuid.h>
#include <vector>

namespace fr::media2::demos {

  class Job {
  public:
    Job();
    std::string filename;
    std::string jobId;
    // Resolution should be 4K, 1080 or 720.
    std::string resolution;
    std::vector<std::string> streamIds;

    // Returns a path to the root dir for this job.
    // You can set the working dir with the env var
    // JOB_STORAGE_ROOT. If undefined, will use
    // temp dir reported by std::filesystem.
    std::filesystem::path jobRoot();

    template<typename Archive>
    void serialize(Archive &ar) {
      ar(CEREAL_NVP(filename), CEREAL_NVP(jobId), CEREAL_NVP(resolution), CEREAL_NVP(streamIds));
    }

    
  private:
    std::filesystem::path storageRoot;
    
  };
  
}
