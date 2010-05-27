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

#ifndef NAREGI_HELPER_HPP
#define NAREGI_HELPER_HPP

#include <fstream>

// saga includes
#include <saga/saga/job.hpp>
#include <saga/saga/url.hpp>

// boost
#include <boost/asio/ip/host_name.hpp>
#include <boost/regex.hpp>

namespace naregi_job { namespace helper {

  //////////////////////////////////////////////////////////////////////
  //
  class jobid_converter {
    saga::url backend_url;
    boost::regex re_jobid;
  public:
    //
    jobid_converter() {}
    jobid_converter(saga::url rm) :
      backend_url(rm), re_jobid("^\\[(.+)]-\\[(C?ID_\\d+)\\]$") {}

    ////////////////////////////////////////////////////////////////////
    // convert naregi-id to job-id
    std::string convert_jobid(const std::string& naregiid);

    ////////////////////////////////////////////////////////////////////
    // convert job-id to naregi-id
    std::string convert_naregiid(const std::string& sagaid);
  };

  //////////////////////////////////////////////////////////////////////
  //
  saga::job::state convert_saga_job_state(std::string naregi_status);

  //////////////////////////////////////////////////////////////////////
  //
  inline std::string get_hostname(void) {
    boost::system::error_code ec;
    return boost::asio::ip::host_name(ec);
  }

  //////////////////////////////////////////////////////////////////////
  // create saga::job::description
  bool create_saga_job_description(saga::job::description&     jd,
                                   std::string                 cmd,
                                   std::string                 host);

  //////////////////////////////////////////////////////////////////////
  //
  bool split_command_line(std::string               cmd,
                          std::string&              executable,
                          std::vector<std::string>& options);

  //////////////////////////////////////////////////////////////////////
  // tempfile
  class tempfile {
    std::string name;
    std::ofstream real;

  public:
    //
    tempfile(const std::string pattern);
    //
    ~tempfile();
    //
    std::string get_name() { return name; }
    //
    void close();
    //
    std::ofstream& get_stream() { return real; }
  };
}}
#endif // NAREGI_HELPER_HPP
