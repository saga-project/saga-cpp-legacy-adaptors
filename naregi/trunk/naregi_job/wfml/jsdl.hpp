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


#ifndef JSDL_HPP
#define JSDL_HPP

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "debug.hpp"
#include "jsdl_staging.hpp"

#define typedef_ptr(var) typedef boost::shared_ptr<var> var ## _ptr
//#define typedef_ptr(var) typedef var* var ## _ptr

namespace naregi_job { namespace jsdl {

  namespace elements {
    char const* const job_definition = "JobDefinition";
    char const* const jsdl = "http://schemas.ggf.org/jsdl/2005/06/jsdl";
    char const* const naregi = "http://www.naregi.org/ws/2005/08/jsdl-naregi-draft-02";

    char const* const job_description = "JobDescription";

    //
    char const* const job_identification = "JobIdentification";
    char const* const jobname = "JobName";

    //
    char const* const application = "Application";

    //
    char const* const resources = "Resources";
    char const* const candidate_hosts = "CandidateHosts";
    char const* const hostname = "HostName";
    char const* const operating_system = "OperatingSystem";
    char const* const operating_system_type = "OperatingSystemType";
    char const* const operating_system_name = "OperatingSystemName";
    char const* const individual_cpu_count = "IndividualCPUCount";
    char const* const lower_bounded_range = "LowerBoundedRange";
  }

  namespace jsdl_posix
  {
    //
    char const* const application = "POSIXApplication";
    char const* const jsdl_posix = "http://schemas.ggf.org/jsdl/2005/06/jsdl-posix";
    char const* const executable = "Executable";
    char const* const argument = "Argument";
    char const* const output = "Output";
    char const* const error = "Error";
    char const* const working_directory = "WorkingDirectory";
    char const* const environment = "Environment";

    char const* const wall_time_limit = "WallTimeLimit";
  }

  namespace attribute
  {
    char const* const name = "name";
  }

#define JSDL_STRING_ACCESSER(prop, member)                  \
      void set_ ## prop(std::string name)                   \
      { member = name; }                                    \
      std::string get_ ## prop(void)                        \
      { return member; }                                    \

#define JSDL_VECTOR_ACCESSER(prop, member)                  \
      void set_ ## prop(std::vector<std::string> v)         \
      { member = v; }                                       \
      std::vector<std::string> get_ ## prop(void)           \
      { return member; }                                    \

#define JSDL_CONTAINER_ACCESSER(C, prop, member)            \
      void set_ ## prop(C& v)                               \
      { member = v; }                                       \
      C& get_ ## prop(void)                                 \
      { return member; }                                    \

  //////////////////////////////////////////////////////////////////////
  // job_identity
  class jsdl_job_identity {
    std::string jobname;

  public:
    //
    jsdl_job_identity() {}
    //
    DESTRUCTOR(jsdl_job_identity);
    //
    JSDL_STRING_ACCESSER(jobname, jobname);
  };

  //////////////////////////////////////////////////////////////////////
  // application
  class application_impl {
  public:
    // for polymorpic (downcast)
    VDESTRUCTOR(application_impl);
  };
  typedef_ptr(application_impl);

  //////////////////////////////////////////////////////////////////////
  // jsdl_application
  class jsdl_application {
    application_impl_ptr impl;
  public:
    //
    jsdl_application() {}
    //
    DESTRUCTOR(jsdl_application);
    //
    application_impl_ptr get_impl(void) { return impl; }
    void set_impl(application_impl* app) {
      impl = application_impl_ptr(app);
    }
  };

  //////////////////////////////////////////////////////////////////////
  // posix_application
  class posix_application : public application_impl {

  public:
    typedef std::pair<std::string, std::string> envpair;
    typedef std::vector<envpair> envtype;

  private:
    std::string exe;
    std::vector<std::string> arguments;
    std::string output;
    std::string error;
    std::string workdir;
    envtype environments;
    std::string walltimelimit;

  public:
    //
    posix_application() {}
    //
    VDESTRUCTOR(posix_application);

    JSDL_STRING_ACCESSER(executable, exe);

    JSDL_VECTOR_ACCESSER(arguments, arguments);

    JSDL_STRING_ACCESSER(output, output);

    JSDL_STRING_ACCESSER(error, error);

    JSDL_STRING_ACCESSER(working_directory, workdir);

    JSDL_CONTAINER_ACCESSER(envtype, environments, environments);

    JSDL_STRING_ACCESSER(wall_time_limit, walltimelimit);
  };
  typedef_ptr(posix_application);

  //////////////////////////////////////////////////////////////////////
  // jsdl_resources
  class jsdl_resources {
    std::vector<std::string> osnames;
    std::vector<std::string> hosts;
    std::string lowerboundedrange;

  public:
    //
    jsdl_resources() : lowerboundedrange("1") {}
    //
    VDESTRUCTOR(jsdl_resources);

    JSDL_VECTOR_ACCESSER(osnames, osnames);

    JSDL_VECTOR_ACCESSER(candidate_hosts, hosts);

    JSDL_STRING_ACCESSER(lower_bounded_range, lowerboundedrange);
  };

  //////////////////////////////////////////////////////////////////////
  // jsdl_data_staging
  class jsdl_data_staging {
    std::vector<transfer> stage_in;
    std::vector<transfer> stage_out;

  public:
    //
    DESTRUCTOR(jsdl_data_staging);
    //
    std::vector<transfer> get_stage_in(void) { return stage_in; }
    void set_stage_in(std::vector<transfer> x) { stage_in = x; }
    //
    std::vector<transfer> get_stage_out(void) { return stage_out; }
    void set_stage_out(std::vector<transfer> x) { stage_out = x; }
  };

  //////////////////////////////////////////////////////////////////////
  //
  typedef_ptr(jsdl_job_identity);
  typedef_ptr(jsdl_application);
  typedef_ptr(jsdl_resources);
  typedef_ptr(jsdl_data_staging);

  inline posix_application_ptr get_posix_application_ptr(jsdl_application_ptr app)
  {
    return boost::static_pointer_cast<posix_application>(app->get_impl());
  }

  //////////////////////////////////////////////////////////////////////
  // job submission description
  class description {
    jsdl_job_identity_ptr identity;
    jsdl_application_ptr application;
    jsdl_resources_ptr resources;
    jsdl_data_staging_ptr data_staging;

  public:
    description() {}
#if 0
    description() :
      identity((jsdl_job_identity*)0),
      application((jsdl_application*)0),
      resources((jsdl_resources*)0),
      data_staging((jsdl_data_staging*)0) {}
#endif
    //
    DESTRUCTOR(description);

    jsdl_job_identity_ptr get_identity() { return identity; }
    void set_identity(jsdl_job_identity* i) {
      identity = jsdl_job_identity_ptr(i);
    }

    jsdl_application_ptr get_application() { return application; }
    void set_application(jsdl_application* a) {
      application = jsdl_application_ptr(a);
    }

    jsdl_resources_ptr get_resources() { return resources; }
    void set_resources(jsdl_resources* r) {
      resources = jsdl_resources_ptr(r);
    }

    jsdl_data_staging_ptr get_data_staging() { return data_staging; }
    void set_data_staging(jsdl_data_staging* d) {
      data_staging = jsdl_data_staging_ptr(d);
    }
  };

  //////////////////////////////////////////////////////////////////////
  // job submission description builder
  class description_builder {
    virtual jsdl_job_identity* create_identity() = 0;
    virtual jsdl_application* create_application() = 0;
    virtual jsdl_resources* create_resources() = 0;
    virtual jsdl_data_staging* create_data_staging() = 0;

  public:
    VDESTRUCTOR(description_builder);
    //
    description* get_description(void) {
      description* desc = new description();
      desc->set_identity(create_identity());
      desc->set_application(create_application());
      desc->set_resources(create_resources());
      desc->set_data_staging(create_data_staging());
      return desc;
    }
  };
}}

#endif  // JSDL_HPP
