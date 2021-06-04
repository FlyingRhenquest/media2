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
 * Main function for router demo.
 *
 */

#include <Job.h>
#include <JobRegistry.h>
#include <RouterJobHandler.h>
#include <fr/media2.h>

using namespace fr::media2::demos;

int main(int argc, char *argv[]) {

  // Address to send RouterJobHandler notifications
  // Env: JOB_HANDLER_ADDRESS
  std::string jobHandlerAddress{"JOB_HANDLER_ADDRESS"};
  // Address to query registry by job id
  // Env: QUERY_BY_ID_ADDRESS
  std::string queryByIdAddress{"QUERY_BY_ID_ADDRESS"};
  // Env: QUERY_BY_STREAM_ADDRESS
  // Address to query registry by stream id
  std::string queryByStreamAddress{"QUERY_BY_STREAM_ADDRESS"};
  // Address to receive incoming segments to
  // Env: INCOMING_SEGMENT_ADDRESS
  std::string incomingSegmentAddress{"INCOMING_SEGMENT_ADDRESS"};
  // Address to write incoming segments to
  // Env: OUTGOING_SEGMENT_ADDRESS
  std::string segmentAddress{"OUTGOING_SEGMENT_ADDRESS"};
  std::string errors;

  if (char *jha = getenv(jobHandlerAddress.c_str())) {
    jobHandlerAddress = jha;
  } else {
    errors.append(jobHandlerAddress);
  }
  if (char *qbi = getenv(queryByIdAddress.c_str())) {
    queryByIdAddress = qbi;
  } else {
    errors.append(errors.empty() ? queryByIdAddress : std::string{", "} + queryByIdAddress); 
  }
  if (char *qbs = getenv(queryByStreamAddress.c_str())) {
    queryByStreamAddress = qbs;
  } else {
    errors.append(errors.empty() ? queryByStreamAddress : std::string{", "} + queryByStreamAddress);
  }
  if (char *isa = getenv(incomingSegmentAddress.c_str())) {
    incomingSegmentAddress = isa;
  } else {
    errors.append(errors.empty() ? incomingSegmentAddress : std::string{", "} + incomingSegmentAddress);
  }
  if (char *sa = getenv(segmentAddress.c_str())) {
    segmentAddress = sa;
  } else {
    errors.append(errors.empty() ? segmentAddress : std::string{", "} + segmentAddress);
  }
  if (!errors.empty()) {
    std::string err("The following environment variables must be set up with zmq URIs prior to running this demo: ");
    err.append(errors);
    throw std::runtime_error(err);
  }
  // Start registry service
  auto registry = std::make_shared<JobRegistry>(queryByIdAddress, queryByStreamAddress);
  // Start job handler service
  auto jobHandler = std::make_shared<RouterJobHandler>(jobHandlerAddress, registry);
  // Subscribe registry to job handler job notifications
  jobHandler->receivedJob.connect([registry](std::shared_ptr<Job> job) { registry->add(job); });
  // Set up receiving data from user
  std::cout << "Setting up zmq subscriber on " << incomingSegmentAddress << std::endl;
  auto subscriber = std::make_shared<fr::media2::ZmqSegmentSubscriber>(incomingSegmentAddress);
  // Set up republishing segments
  std::cout << "Setting up zmq publisher on " << segmentAddress << std::endl;
  auto publisher = std::make_shared<fr::media2::ZmqSegmentPublisher>(segmentAddress);
  subscriber->receivedSegment.connect([publisher,registry](std::stringstream &buffer, uuid_t uuid) {
    // Verify that we have a uuid in memory for this segment
    char uuidstr[40];
    uuid_unparse(uuid, uuidstr);
    std::string id(uuidstr);
    if (nullptr != registry->byStreamId(id).get()) {
      // Push to publisher
      publisher->process(buffer, uuid);
    }
  });
  subscriber->process();
  // Block forever
  subscriber->join();
}
