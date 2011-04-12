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

#ifndef DIRECTIVES_HPP
#define DIRECTIVES_HPP

#include <string>
#include <vector>

#include <saga/saga/job.hpp>
#include <saga/saga/url.hpp>

#include <boost/shared_ptr.hpp>

#include "debug.hpp"
#include "staging.hpp"

namespace sja = saga::job::attributes;

namespace torque_job { namespace cli {

  class directives;
  class directives_checker;
  class directives_builder;

  typedef  boost::shared_ptr<directives> directives_ptr;
  typedef  boost::shared_ptr<directives_checker> directives_checker_ptr;
  typedef  boost::shared_ptr<directives_builder> directives_builder_ptr;

  //////////////////////////////////////////////////////////////////////
  //
  class directives  {
  public:
    VDESTRUCTOR(directives);

    //
    virtual void set_job_name(std::string job_name) = 0;
    virtual void set_host(std::string& hostname) = 0;
    //
    virtual void set_output(std::string& path) = 0;
    //
    virtual void set_error(std::string& path) = 0;
    //
    virtual void set_environment(std::vector<std::string>& env) = 0;
    //
    virtual void set_job_project(std::string job_project) = 0;

    virtual void set_working_directory(std::string path) = 0;
    //
    virtual void set_stagein(std::string file_list) = 0;
    //
    virtual void set_stageout(std::string file_list) = 0;
    //
    virtual void set_walltime(std::string& seconds) = 0;
    //
    virtual void set_job_contact(std::string& mailaddr) = 0;
    //
    virtual void put(std::ostream& s) = 0;
  };

  std::ostream& operator<<(std::ostream& s, directives& d);

  //////////////////////////////////////////////////////////////////////
  //
  class directives_checker {
  public:
    VDESTRUCTOR(directives_checker);
    //
    virtual bool check_host(std::string& hostname) const {
      return true;
    }
    //
    virtual bool check_working_directory(std::string& path) const {
      return true;
    }
    //
    virtual bool check_environment(std::string& env) const {
      return true;
    }
    //
    virtual bool check_file_transfer(file_transfer_ptr ft) const {
      return true;
    }
    //
    virtual bool check_walltime(std::string& seconds) const {
      return true;
    }
    //
    virtual bool check_job_contact(saga::url& mail_uri) const {
      return true;
    }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class directives_builder {
  public:
    //
    VDESTRUCTOR(directives_builder);
    //
    virtual directives_ptr build(saga::job::description& jd,
				 std::string localhost) = 0;
  };

  //////////////////////////////////////////////////////////////////////
  //
  inline std::ostream& operator<<(std::ostream& s, directives& d)
  {
    d.put(s);
    return s;
  }
}}

#endif  // DIRECTIVES_HPP
