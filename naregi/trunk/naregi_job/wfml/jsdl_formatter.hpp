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


#ifndef JSDL_FORMATTER_HPP
#define JSDL_FORMATTER_HPP

#include "jsdl.hpp"
#include "debug.hpp"

namespace naregi_job { namespace jsdl {

  //////////////////////////////////////////////////////////////////////
  // job_identity formatter
  class job_identity_formatter {
    //
    virtual void output_jobname(std::ostream& s, jsdl_job_identity* j) = 0;

  public:
    //
    VDESTRUCTOR(job_identity_formatter);
    //
    void format(std::ostream& s, jsdl_job_identity* j) {
      output_jobname(s, j);
    }
  };

  //////////////////////////////////////////////////////////////////////
  // posix_application formatter
  class posix_application_formatter {
    virtual void output_executable(std::ostream& s, posix_application* j) = 0;
    virtual void output_arguments(std::ostream& s, posix_application* j) = 0;
    virtual void output_output(std::ostream& s, posix_application* j) = 0;
    virtual void output_error(std::ostream& s, posix_application* j) = 0;
    virtual void output_working_directory(std::ostream& s, posix_application* j) = 0;
    virtual void output_environments(std::ostream& s, posix_application* j) = 0;
    virtual void output_wall_time_limit(std::ostream& s, posix_application* j) = 0;

  public:
    //
    VDESTRUCTOR(posix_application_formatter);
    //
    void format(std::ostream& s, posix_application* j) {
      output_executable(s, j);
      output_arguments(s, j);
      output_output(s, j);
      output_error(s, j);
      output_working_directory(s, j);
      output_environments(s, j);
      output_wall_time_limit(s, j);
    }
  };

  //////////////////////////////////////////////////////////////////////
  // resources formatter
  class resources_formatter {
    virtual void output_candidate_hosts(std::ostream& s, jsdl_resources* j) = 0;
    virtual void output_operating_system(std::ostream& s, jsdl_resources* j) = 0;
    virtual void output_individual_cpu_count(std::ostream& s, jsdl_resources* j) = 0;

  public:
    //
    VDESTRUCTOR(resources_formatter);
    //
    void format(std::ostream& s, jsdl_resources* j) {
      output_candidate_hosts(s, j);
      output_operating_system(s, j);
      output_individual_cpu_count(s, j);
    }
  };
}}

#endif  // JSDL_FORMATTER_HPP
