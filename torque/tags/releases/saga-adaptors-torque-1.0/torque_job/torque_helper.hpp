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

#ifndef TORQUE_HELPER_HPP
#define TORQUE_HELPER_HPP

#include <boost/regex.hpp>

#include <saga/saga/attribute.hpp>
#include <saga/saga/job.hpp>

#include "torque_job_adaptor.hpp"

#define RE_PBS_JOBID   "((\\d+(-\\d+)?)(\\.(\\S+))?)"
#define RE_SAGA_JOBID  "^\\[([^\\]]+)\\]-\\[([^\\]]+)\\]$"

namespace torque_job { namespace helper {

  // baseid : 231.kek-sna131.soum.co.jp
  //  pbsid : 231.kek-sna131.soum.co.jp : true
  //        : 231.kek-sna131            : true
  //        : 231                       : true
  //        : 231.kek-sna132            : false
  //        : 232                       : false
  inline
  bool pbsid_match(std::pair<std::string, saga::job::description> entry,
		   std::string baseid) {
    //std::cout << "entry.first:" << entry.first << std::endl;
    //std::cout <<      "baseid:" << baseid << std::endl;
    // Sorry, under construction.
    return false;
  }

  //////////////////////////////////////////////////////////////////////
  // helper class

  class jobid_converter {
    saga::url backend_url;
    boost::regex re_jobid;
    boost::regex re_pbsid;
  public:
    //
    jobid_converter() {}
    jobid_converter(saga::url rm) :
      backend_url(rm),
      re_jobid(RE_SAGA_JOBID), re_pbsid(RE_PBS_JOBID)
    {}

    ////////////////////////////////////////////////////////////////////
    // convert pbs-id to job-id
    std::string convert_jobid(const std::string& pbsid);

    ////////////////////////////////////////////////////////////////////
    // convert job-id to pbs-id
    std::string convert_pbsid(const std::string& sagaid);

    ////////////////////////////////////////////////////////////////////
	// convert job-id to pbs-id , short version (only returns job#)
	std::string convert_pbsid_short(const std::string& sagaid);

    ////////////////////////////////////////////////////////////////////
    // get server_host from SAGA JobID
	std::string get_server_host(const std::string& sagaid);
  };

  //////////////////////////////////////////////////////////////////////
  // helper functions
  bool create_saga_job_description(saga::job::description& jd,
				   std::string cmd,
				   std::string host);
  //
  bool split_command_line(std::string cmd,
			  std::string& executable,
			  std::vector<std::string>& arguments);
  //
  saga::job::state convert_saga_job_state(std::string torque_status);

}}

#endif  // TORQUE_HELPER_HPP
