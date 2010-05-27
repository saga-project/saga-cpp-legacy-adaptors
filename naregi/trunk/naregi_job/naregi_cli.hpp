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

#ifndef NAREGI_CLI_HPP
#define NAREGI_CLI_HPP

#include <boost/process.hpp>
#include <boost/regex.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// job package includes
#include <saga/saga/error.hpp>
#include <saga/saga/job.hpp>

#include "wfml/workflow.hpp"

namespace naregi_job {

  namespace cli {

    //////////////////////////////////////////////////////////////////////
    //
    class output_parser {
    private:
      boost::regex reg;
    public:
      output_parser();
      void reset(std::string pattern);
      bool parse_line(std::string line, std::string& data);
      bool parse_line(std::string line);
    };

    class error_msg {
      boost::regex reg_auth;
      boost::regex reg_id;
      boost::regex reg_no_file;
      boost::regex reg_unknown;
    public:
      //
      error_msg();

      //
      saga::error check(std::string msg);
    };

    extern error_msg em;

    //////////////////////////////////////////////////////////////////////
    //
    namespace bp = ::boost::process;

    //
    bp::child execute(std::string command,
                      std::vector<std::string>& options,
                      std::string arg = "");

    //////////////////////////////////////////////////////////////////////
    // naregi cli
    //
    bool naregi_simplejob_submit(saga::job::description & jd,
                                 std::string& id,
                                 std::ostringstream& os);
    //
    bool naregi_job_list(std::vector<std::string>& ids,
                         std::ostringstream& os);

    //
    bool naregi_job_submit(naregi_job::wfml::workflow* wf,
                           std::string& id,
                           std::ostringstream& os);

    //
    bool naregi_job_status(std::string id,
                           std::string& status,
                           std::ostringstream& os);

    //
    bool naregi_job_cancel(std::string id, std::ostringstream& os);

    bool naregi_std_print(std::string id,
							std::ostringstream& std_out,
							std::ostringstream& os);
  }
}

#endif // NAREGI_CLI_HPP
