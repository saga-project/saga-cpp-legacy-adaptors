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

#include <saga/impl/packages/job/split_commandline.hpp>

#include "pbspro_helper.hpp"

namespace pbspro_job { namespace helper {

  //////////////////////////////////////////////////////////////////////
  // helper class

  /////////////////////////////////////////////////////////////////////////////
  // convert SAGA JobID to PBSPro JobID
  //             [pbspro://hostname/]-[79] => 79
  // [pbspro://hostname/]-[79.server_host] => 79.server_host
  //
  std::string jobid_converter::convert_pbsid(const std::string& jobid)
  {
    boost::smatch m;

    if (!boost::regex_match(jobid, m, re_jobid)) {
      SAGA_ADAPTOR_THROW_NO_CONTEXT("bad JobID \"" + jobid + "\"",
			  saga::BadParameter);
    }

    // verify backend URL
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

    // verify PBS JoID format
    std::string pbsid = m[2];
    if (!boost::regex_match(pbsid, m, re_pbsid)) {
      SAGA_ADAPTOR_THROW_NO_CONTEXT("bad JobID \"" + jobid + "\"",
			  saga::BadParameter);
    }

    return pbsid;
  }

  /////////////////////////////////////////////////////////////////////////////
  // convert SAGA JobID to PBSPro JobID, short version
  //             [pbspro://hostname/]-[79] => 79
  // [pbspro://hostname/]-[79.server_host] => 79
  //
  std::string jobid_converter::convert_pbsid_short(const std::string& jobid)
  {
	std::string pbsid_long = convert_pbsid(jobid);
	std::string pbsid_short;

	boost::regex r("(\\d+)\\.(.*)");
	boost::smatch results;
	boost::regex_search(pbsid_long, results, r);
	pbsid_short = results.str(1);
	return pbsid_short;
  }

  /////////////////////////////////////////////////////////////////////////////
  // get server_host from SAGA JobID
  //             [pbspro://hostname/]-[79] => NULL
  // [pbspro://hostname/]-[79.server_host] => server_host
  //
  std::string jobid_converter::get_server_host(const std::string& jobid)
  {
	std::string pbsid_long = convert_pbsid(jobid);
	std::string sv_name;

	boost::regex r("(\\d+)\\.(.*)");
	boost::smatch results;
	boost::regex_search(pbsid_long, results, r);
	sv_name = results.str(2);

	return sv_name;
  }

  /////////////////////////////////////////////////////////////////////////////
  // convert PBSPro JobID to JobID
  //             79 => [pbspro://hostname/]-[79]
  // 79.server_host => [pbspro://hostname/]-[79.server_host]
  //
  std::string jobid_converter::convert_jobid(const std::string& pbsid)
  {
    return "[" + backend_url.get_url() + "]-[" + pbsid + "]";
  }



  //////////////////////////////////////////////////////////////////////
  // helper functions

  //////////////////////////////////////////////////////////////////////
  // create saga::job::description for saga::job::service::run_job()
  // 'Executable'
  // 'Arguments'
  // 'Interactive'
  // 'CandidateHosts'
  bool create_saga_job_description(saga::job::description& jd,
				   std::string cmd,
				   std::string host)
  {
    using namespace saga::job::attributes;

    std::string executable;
    std::vector<std::string> arguments;

    if (split_command_line(cmd, executable, arguments) == false) {
      return false;
    }

    // Executable
    jd.set_attribute(description_executable, executable);

    // Arguments
    if (!arguments.empty()) {
      jd.set_vector_attribute(description_arguments, arguments);
    }

    // Interactive
    //jd.set_attribute(description_interactive, "False");
    jd.set_attribute(description_interactive, sa::common_false);

#if 0
    // CandidateHosts
    if (!host.empty()) {
      std::vector <std::string> hosts;
      hosts.push_back(host);
      jd.set_vector_attribute(description_candidate_hosts, hosts);
    }
#endif

    return true;
  }

  //////////////////////////////////////////////////////////////////////
  // "/bin/uname -a" => "/bin/uname" "-a"
  bool split_command_line(std::string cmd,
			  std::string& executable,
			  std::vector<std::string>& arguments)
  {
    std::vector<std::string> elems = saga::impl::split_commandline(cmd);
    if (elems.empty()) {
      return false;
    }

    executable = elems[0];
    if (elems.size() > 1) {
      elems.erase(elems.begin());
      std::copy(elems.begin(), elems.end(), std::back_inserter(arguments));
    }

    return true;
  }

}}
