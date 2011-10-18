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

#ifndef SCRIPT_HPP
#define SCRIPT_HPP

#include <boost/shared_ptr.hpp>

#include "debug.hpp"
#include "directives_impl.hpp"
#include "pbspro_cli_staging.hpp"

namespace pbspro_job { namespace cli {

  class job_script;
  typedef boost::shared_ptr<job_script> job_script_ptr;
  class job_script_builder;
  typedef boost::shared_ptr<job_script_builder> job_script_builder_ptr;

  //////////////////////////////////////////////////////////////////////
  //
  class job_script {
    directives_ptr dir;
    std::string commandline;

  protected:
    //
    std::string get_commandline() { return commandline; }

  public:
    job_script() {}
    DESTRUCTOR(job_script);
    //
    void set_directives(const directives_ptr dir);
    //
    void set_commandline(const std::string& command,
			 std::vector<std::string>& arguments);
    //
    void put(std::ostream& s);
  };

  std::ostream& operator<<(std::ostream& s, job_script& js);

  //////////////////////////////////////////////////////////////////////
  //
  inline std::ostream& operator<<(std::ostream& s, job_script& js)
  {
    js.put(s);
    return s;
  }

  class _directives_checker_impl : public directives_checker {
  public:
    DESTRUCTOR(_directives_checker_impl);
    //
    bool check_host(std::string& hostname) const { return true;  }
    //
    bool check_working_directory(std::string& path) const { return true; }
    //
    bool check_environment(std::string& env) const { return true; }
    //
    bool check_file_transfer(file_transfer_ptr ft) const { return true; }
    //
    bool check_walltime(std::string& seconds) const { return true; }
    //
    bool check_job_contact(saga::url& mail_uri) const {
      if (mail_uri.get_scheme() != "mailto" ||
	  mail_uri.get_path().empty()
	  // TODO mail address format check.
	  ) {
	return false;
      }
      return true;
    }
  };

  //////////////////////////////////////////////////////////////////////
  //
  class job_script_builder {
    std::string localhost;
    directives_builder_ptr dbl;

  public:
    //
    job_script_builder(std::string localhost) : localhost(localhost)
    {
      directives_checker_ptr
	checker(new _directives_checker_impl());

      file_transfer_parser_ptr
	parser(new file_transfer_parser_impl());

      dbl = directives_builder_ptr(new directives_builder_impl(checker, parser));
    }
#if 0
    //
    job_script_builder(file_transfer_parser_ptr parser) {
      directives_checker_ptr checker(new _directives_checker_impl());
      dbl = directives_builder_ptr(new directives_builder_impl(checker, parser));
    }
    //
    job_script_builder() {
      directives_checker_ptr checker(new _directives_checker_impl());
      dbl = directives_builder_ptr(new directives_builder_impl(checker));
    }
#endif
    //
    job_script_ptr build(saga::job::description& jd);
  };

}}

#endif // SCRIPT_HPP
