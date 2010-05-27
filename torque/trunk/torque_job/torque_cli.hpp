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

#ifndef TORQUE_CLI_HPP
#define TORQUE_CLI_HPP

#include <boost/process.hpp>
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

// job package includes
#include <saga/saga/error.hpp>
#include <saga/saga/job.hpp>

#include "directives.hpp"
#include "script.hpp"
#include "torque_cli_staging.hpp"
#include "torque_job_service.hpp"

#include "torque_helper.hpp"

namespace torque_job { namespace cli {

  //////////////////////////////////////////////////////////////////////
  //
  class output_parser {
  private:
    boost::regex reg;
  public:
    output_parser();
    void reset(std::string pattern);
    bool parse_line(std::string line, std::string& data);
    bool parse_line(std::string line,
		    std::vector<std::string>& matched);
    bool parse_line(std::string line);
  };

  //////////////////////////////////////////////////////////////////////
  //
  class jobstat {
    std::string id;
    std::map<std::string, std::string> stat;
  public:
    jobstat(std::string id) : id(id) {}
    //
    std::string get_job_id() { return id; }
    //
    void set_entry(std::string key, std::string value) {
      stat[key] = value;
    }
    //
    void append_value(std::string key, std::string value) {
      std::string s = stat[key];
      stat[key] = s + value;
    }
    //
    bool has_key(std::string key) const {
      return (stat.find(key) != stat.end());
    }
    //
    std::string& get_value(std::string key) {//const {
      // if (!has_key(key)) throw ...
      return stat[key];
    }
    //
    std::string& get_job_state() {//const {
      const std::string job_state = "job_state";
      if (has_key(job_state)) {
    	  return get_value(job_state);
      }
      // TODO else
    }
    //
    std::string& get_exit_status() {//const {
      const std::string exit_status = "exit_status";
      if (has_key(exit_status)) {
    	  return get_value(exit_status);
      }
      // TODO else
    }
  };

  typedef boost::shared_ptr<jobstat> jobstat_ptr;

  class jobstat_builder {
    output_parser parser1;
    output_parser parser2;
    output_parser parser3;
  public:
    //
    jobstat_builder();
    //
    jobstat_ptr create(std::istream& f);
  };

  //////////////////////////////////////////////////////////////////////
  //
  class qsub {
    const std::string command;
    job_script_builder_ptr jsbuilder;

  public:
    qsub(std::string localhost);
    DESTRUCTOR(qsub);
    //
    bool execute(saga::job::description& jd,
		 std::string& id, std::ostringstream& os);
  };

  //////////////////////////////////////////////////////////////////////
  //
  class qstat {
    const std::string command;
    output_parser parser;
    jobstat_builder builder;

    helper::jobid_converter jobid_converter;

    bool check_header(std::istream& stdout);

  public:
    qstat() : command("qstat") {}
    DESTRUCTOR(qstat);

    bool execute(std::vector<std::string>& idlist, std::ostringstream& os);

    bool get_state(std::string id, std::string& pbs_state,
		   std::ostringstream& os);

    jobstat_ptr get_full_status(std::string id,
				std::ostringstream& os);
  };

}}

#endif // TORQUE_CLI_HPP
