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

#include <Job.h>

namespace fr::media2::demos {

  Job::Job() {
    if (char *sr = getenv("JOB_STORAGE_ROOT")) {
      storageRoot = std::string{sr};
    } else {
      storageRoot = std::filesystem::temp_directory_path();
    }
  }

  std::filesystem::path Job::jobRoot() {
    return storageRoot / jobId;
  }
  
}
