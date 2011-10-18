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

#include <sstream>
#include <boost/foreach.hpp>

#include "script.hpp"

namespace pbspro_job { namespace cli {

  //////////////////////////////////////////////////////////////////////
  //
  void job_script::put(std::ostream& s)
  {
    s << "#! /bin/sh" << "\n";
    if (dir.get() != 0)
      s << *dir;
    s << get_commandline();
  }

  //////////////////////////////////////////////////////////////////////
  //
  void job_script::set_directives(const directives_ptr d)
  {
    // copy
    dir = d;
  }

  //////////////////////////////////////////////////////////////////////
  //
  void job_script::set_commandline(const std::string& command,
				   std::vector<std::string>& arguments)
  {
#if 0
    if (command.empty()) {
      throw
    }
#endif

    std::ostringstream os;
    os << command;

    BOOST_FOREACH(std::string arg, arguments) {
      os << " " << arg;
    }

    commandline = os.str();
  }

  //////////////////////////////////////////////////////////////////////
  //
  job_script_ptr job_script_builder::build(saga::job::description& jd)
  {
    std::string command = jd.get_attribute(sja::description_executable);

    std::vector<std::string> args;
    if (jd.attribute_exists(sja::description_arguments)) {
      args = jd.get_vector_attribute(sja::description_arguments);
    }

    job_script_ptr j = job_script_ptr(new job_script());

    j->set_directives(dbl->build(jd, localhost));
    j->set_commandline(command, args);

    return j;
  }

}}
