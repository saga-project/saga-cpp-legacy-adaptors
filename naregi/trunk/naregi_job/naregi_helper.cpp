/*
 * Copyright (C) 2008-2009 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2009 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <string>

extern "C" {
#include <stdlib.h>
}
#include <cerrno>
#include <cstring>

#include <saga/impl/exception.hpp>
#include <saga/impl/packages/job/split_commandline.hpp>

#include "naregi_helper.hpp"

namespace naregi_job { namespace helper {

  /////////////////////////////////////////////////////////////////////////////
  // convert JobID to NAREGI ID
  // [naregi://hostname/]-[CID_12345] => CID_12345
  std::string jobid_converter::convert_naregiid(const std::string& jobid)
  {
    boost::smatch m;

    if (!boost::regex_match(jobid, m, re_jobid)) {
      SAGA_ADAPTOR_THROW_NO_CONTEXT("bad JobID \"" + jobid + "\"",
			  saga::BadParameter);
    }

    std::string rm = backend_url.get_url();
    if (rm.compare(m[1]) != 0) {
      // from condor
      SAGA_OSSTREAM strm;
      strm << "Backend URL in job ID ('" << jobid << "')"
           << " does not match this saga::job::service's"
           << " resource manager URL ('" << rm << "').";
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
                                    saga::BadParameter);
    }

    return m[2];
  }

  /////////////////////////////////////////////////////////////////////////////
  // convert NAREGI ID to JobID
  // CID_12345 => [naregi://hostname/]-[CID_12345]
  std::string jobid_converter::convert_jobid(const std::string& naregiid)
  {
    return "[" + backend_url.get_url() + "]-[" + naregiid + "]";
  }

  /////////////////////////////////////////////////////////////////////////////
  //  NAREGI Job status ==> saga::job::state
  //
  saga::job::state convert_saga_job_state(std::string naregi_status)
  {
      saga::job::state saga_state = saga::job::Unknown;

      if (naregi_status == "Running") {
	saga_state = saga::job::Running;              /*   1 */
      } else if (naregi_status == "Exception") {
	saga_state = saga::job::Failed;               /*   2 */
      } else if (naregi_status == "Missing") {
	saga_state = saga::job::Failed;               /*   2 */
      } else if (naregi_status == "Done") {
	saga_state = saga::job::Done;                 /*   3 */
      } else {
	saga_state = saga::job::Unknown;
      }

      return saga_state;
  }


  //////////////////////////////////////////////////////////////////////
  // create saga::job::description
  bool create_saga_job_description(saga::job::description&     jd,
                                   std::string                 cmd,
                                   std::string                 host)
  {
    using namespace saga::job::attributes;

    std::string executable;
    std::vector<std::string> options;
    if (split_command_line(cmd, executable, options) == false) {
      return false;
    }
    jd.set_attribute(description_executable, executable);

    // Interactive
    jd.set_attribute(description_interactive, "False");

    // Arguments
    if (!options.empty()) {
      jd.set_vector_attribute(description_arguments, options);
    }

    // CandidateHosts
    if (!host.empty()) {
      std::vector <std::string> hosts;
      hosts.push_back(host);
      jd.set_vector_attribute(description_candidate_hosts, hosts);
    }

    return true;
  }

  //////////////////////////////////////////////////////////////////////
  // split_command_line.
  //
  // cmd : '/bin/cp "/tmp/my file with spaces" /data/'
  // =>  executable : "/bin/cmp"
  //     options[0] : "/tmp/my file with spaces"
  //     options[1] : "/data/"
  //
  bool split_command_line(std::string cmd,
			  std::string& executable,
			  std::vector<std::string>& options)
  {
    std::vector<std::string> elems = saga::impl::split_commandline(cmd);
    if (elems.empty()) {
      return false;
    }

    executable = elems[0];
    if (elems.size() > 1) {
      elems.erase(elems.begin());
      std::copy(elems.begin(), elems.end(), std::back_inserter(options));
    }

    return true;
  }

  //////////////////////////////////////////////////////////////////////
  // tempfile
  tempfile::tempfile(const std::string pattern)
  {
    char* p = new char[pattern.length()+1];
    pattern.copy(p, std::string::npos);
    p[pattern.length()] = 0;

    if (mkstemp(p) == -1) {
      delete p;
      SAGA_OSSTREAM strm;
      strm << "mkstemp failed. " << std::strerror(errno);
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
			  saga::NoSuccess);
    }

    name = p;
    real.open(p);
    delete p;
  }

  //
  tempfile::~tempfile()
  {
    close();
    if (remove(name.c_str()) == -1) {
      SAGA_OSSTREAM strm;
      strm << name << " remove failed. " << std::strerror(errno);
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
			  saga::NoSuccess);
    }
  }

  //
  void tempfile::close() {
    if (real.is_open()) {
      real.close();
    }
  }
}}
