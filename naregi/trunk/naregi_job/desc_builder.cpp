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

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <saga/impl/exception.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>

#include "desc_builder.hpp"

namespace naregi_job {

  using namespace jsdl;

  //////////////////////////////////////////////////////////////////////
  //
  class file_transfer_parser_impl : public file_transfer_parser {
  public:
    DESTRUCTOR(file_transfer_parser_impl);
    //
    file_transfer_ptr parse(std::string spec) const {
      std::string left, right;
      saga::adaptors::file_transfer_operator mode;

      if (!parse_file_transfer_specification(spec, left, mode, right)) {
	SAGA_OSSTREAM strm;
	strm << "Parse failed FileTransfer entry:'" << spec << "'.";
	SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
				      saga::BadParameter);
	throw;
      }

      file_transfer_ptr p;
      switch (mode) {
      case saga::adaptors::copy_local_remote:
	p = file_transfer_ptr(new file_transfer(left, right, file_transfer::in));
	break;
      case saga::adaptors::copy_remote_local:
	p = file_transfer_ptr(new file_transfer(right, left, file_transfer::out));
	break;
      case saga::adaptors::append_local_remote:
      case saga::adaptors::append_remote_local:
	{
	  SAGA_OSSTREAM strm;
	  strm << "Append operation is unsupported:'" << spec << "'";
	  SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
					saga::NotImplemented);
	}
      break;
      case saga::adaptors::unknown_mode:
      default:
	{
	  SAGA_OSSTREAM strm;
	  strm << "Unknown FileTransfer operator:'" << spec << "'";
	  SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
					saga::BadParameter);
	}
	break;
      }

      return p;
    }
  };

  //////////////////////////////////////////////////////////////////////
  //
  description_builder_impl::description_builder_impl()
  {
    parser = file_transfer_parser_ptr(new file_transfer_parser_impl());
  }

  //////////////////////////////////////////////////////////////////////
  //
  jsdl_job_identity* description_builder_impl::create_identity()
  {
    jsdl_job_identity* identity = new jsdl_job_identity();
    // TODO ini file ?
    identity->set_jobname("Program");
    return identity;
  }

  //////////////////////////////////////////////////////////////////////
  //
  jsdl_application* description_builder_impl::create_application(void)
  {
    posix_application* posixapp = new posix_application();

    // Executable
    posixapp->set_executable(jd->get_attribute(sja::description_executable));

    // Argument
    if (jd->attribute_exists(sja::description_arguments)) {
      std::vector<std::string> args;
      args = jd->get_vector_attribute(sja::description_arguments);
      posixapp->set_arguments(args);
    }

    // Output
    if (jd->attribute_exists(sja::description_output)) {
      posixapp->set_output(jd->get_attribute(sja::description_output));
    }

    // Error
    if (jd->attribute_exists(sja::description_error)) {
      posixapp->set_error(jd->get_attribute(sja::description_error));
    }

    // WorkingDirectory
    if (jd->attribute_exists(sja::description_working_directory)) {
      posixapp->set_working_directory(jd->get_attribute(sja::description_working_directory));
    }

    // Environment
    if (jd->attribute_exists(sja::description_environment)) {

      std::vector<std::string> envs =
	jd->get_vector_attribute(sja::description_environment);
      posix_application::envtype environment;

      BOOST_FOREACH(std::string spec, envs) {
	// "name=value" => name, value
	// TODO : trim
	// TODO : tokenizer<escaped_list_separator<char> >
	std::vector<std::string> name_value;
        boost::algorithm::split(name_value, spec,
                                boost::algorithm::is_any_of("="));
	environment.push_back(make_pair(name_value[0], name_value[1]));
      }
      posixapp->set_environments(environment);
    }

    // WallTimeLimit
    if (jd->attribute_exists(sja::description_wall_time_limit)) {
      std::string wtl = jd->get_attribute(sja::description_wall_time_limit);
      // TODO value check
      posixapp->set_wall_time_limit(wtl);
    }

    jsdl_application* app = new jsdl_application();
    app->set_impl(posixapp);

    return app;
  }

  //////////////////////////////////////////////////////////////////////
  //
  jsdl_resources* description_builder_impl::create_resources(void)
  {
    jsdl_resources* resources = new jsdl_resources();

    // OperatingSystemName
    if (jd->attribute_exists(sja::description_operating_system_type)) {
      resources->set_osnames(jd->get_vector_attribute(sja::description_operating_system_type));
    }

    // CandidateHosts
    if (jd->attribute_exists(sja::description_candidate_hosts)) {
      resources->set_candidate_hosts(jd->get_vector_attribute(sja::description_candidate_hosts));
    }

    return resources;
  }

  //////////////////////////////////////////////////////////////////////
  //
  jsdl_data_staging* description_builder_impl::create_data_staging(void)
  {

    if (!jd->attribute_exists(sja::description_file_transfer)) {
      return 0;
    }

    jsdl_data_staging* ds = new jsdl_data_staging();

    std::vector<std::string> ft =
      jd->get_vector_attribute(sja::description_file_transfer);

    std::vector<transfer> stage_in;
    std::vector<transfer> stage_out;

    BOOST_FOREACH(std::string spec, ft) {
      file_transfer_ptr result = parser->parse(spec);
      transfer_ptr t = result->get_transfer();
      switch (result->get_type()) {
      case file_transfer::in:
	stage_in.push_back(*t);
	break;
      case file_transfer::out:
	stage_out.push_back(*t);
	break;
      }
    }

    ds->set_stage_in(stage_in);
    ds->set_stage_out(stage_out);

    return ds;
  }

}
