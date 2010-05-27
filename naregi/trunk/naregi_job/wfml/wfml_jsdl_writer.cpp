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


#include <boost/foreach.hpp>

#include "wfml.hpp"
#include "wfml_jsdl_writer.hpp"

namespace naregi_job { namespace wfml { namespace jsdl {

  //////////////////////////////////////////////////////////////////////
  // jobdefinition_writer
  void jobdefinition_writer::output_identification(std::ostream& s)
  {
    char const* name = elements::job_identification;

    s << xml->start_tag(name) << '\n';
    jsdl_job_identity_ptr pident = desc->get_identity();
    ident->format(s, pident.get());
    s << xml->end_tag(name) << '\n';
  }

  void jobdefinition_writer::output_application(std::ostream& s)
  {
    char const* name = elements::application;

    s << xml->start_tag(name) << '\n';
    application->format(s, desc->get_application());
    s << xml->end_tag(name) << '\n';
  }

  void jobdefinition_writer::output_resources(std::ostream& s)
  {
    char const* name = elements::resources;

    // TODO desc->get_resources().empty()

    s << xml->start_tag(name) << '\n';
    jsdl_resources_ptr pres = desc->get_resources();
    resources->format(s, pres.get());
    s << xml->end_tag(name) << '\n';
  }

  std::ostream& jobdefinition_writer::put(std::ostream& s)
  {
    xml_tag_ptr t(get_tag());
    char const* name = elements::job_description;

    s << xml->start_tag(*t) << '\n';
    s << xml->start_tag(name) << '\n';
    format(s);
    s << xml->end_tag(name) << '\n';
    s << xml->end_tag(*t) << '\n';

    return s;
  }

  //////////////////////////////////////////////////////////////////////
  // identification_writer
  void job_identity_writer::output_jobname(std::ostream& s, jsdl_job_identity* j)
  {
    s << xml->simple_tag(elements::jobname, j->get_jobname())  << '\n';
  }

  //////////////////////////////////////////////////////////////////////
  //  posix_application_writer
  void posix_application_writer::output_executable(std::ostream& s, posix_application* j)
  {
    s << xml->simple_tag(jsdl_posix::executable, j->get_executable())  << '\n';
  }

  void posix_application_writer::output_arguments(std::ostream& s, posix_application* j)
  {
    BOOST_FOREACH(std::string arg, j->get_arguments()) {
      s << xml->simple_tag(jsdl_posix::argument, arg) << '\n';
    }
  }

  void posix_application_writer::output_output(std::ostream& s, posix_application* j)
  {
    // TODO
	std::string buf = j->get_output();
	if(!buf.empty()){
		s << xml->simple_tag(jsdl_posix::output, buf)  << '\n';
	}
  }

  void posix_application_writer::output_error(std::ostream& s, posix_application* j)
  {
    // TODO
	std::string buf = j->get_error();
	if(!buf.empty()){
		s << xml->simple_tag(jsdl_posix::error, buf)  << '\n';
	}
  }

  void posix_application_writer::output_working_directory(std::ostream& s, posix_application* j)
  {
    if (j->get_working_directory().empty())
      return;

    s << xml->simple_tag(jsdl_posix::working_directory,
			 j->get_working_directory())
      << '\n';
  }

  void posix_application_writer::output_environments(std::ostream& s, posix_application* j)
  {
    posix_application::envtype m = j->get_environments();
    for (posix_application::envtype::iterator i = m.begin();
	 i != m.end(); ++i) {

      xml_tag_ptr t(new xml_tag(jsdl_posix::environment));

      t->add_attr(attribute::name, i->first);
      s << xml->simple_tag(*t, i->second) << '\n';
    }
  }

  void posix_application_writer::output_wall_time_limit(std::ostream& s, posix_application* j)
  {
    if (j->get_wall_time_limit().empty())
      return;

    s << xml->simple_tag(jsdl_posix::wall_time_limit,
			 j->get_wall_time_limit())
      << '\n';
  }

  //////////////////////////////////////////////////////////////////////
  //  resources_writer
  void resources_writer::output_candidate_hosts(std::ostream& s, jsdl_resources* j)
  {
    if (j->get_candidate_hosts().empty())
      return;

    char const* name = elements::candidate_hosts;

    s << xml->start_tag(name) << '\n';
    BOOST_FOREACH(std::string host, j->get_candidate_hosts()) {
      s << xml->simple_tag(elements::hostname, host) << '\n';
    }
    s << xml->end_tag(name) << '\n';
  }

  // operating system
  void resources_writer::output_operating_system(std::ostream& s, jsdl_resources* j)
  {
    if (j->get_osnames().empty())
      return;

    char const* name1 = elements::operating_system;
    char const* name2 = elements::operating_system_type;

    s << xml->start_tag(name1) << '\n';
    s << xml->start_tag(name2) << '\n';
    BOOST_FOREACH(std::string osname, j->get_osnames()) {
      s << xml->simple_tag(elements::operating_system_name, osname) << '\n';
    }
    s << xml->end_tag(name2) << '\n';
    s << xml->end_tag(name1) << '\n';
  }

  // IndividualCPUCount
  void resources_writer::output_individual_cpu_count(std::ostream& s, jsdl_resources* j)
  {
    char const* name = elements::individual_cpu_count;

    s << xml->start_tag(name) << '\n';
    s << xml->simple_tag(elements::lower_bounded_range,
			 j->get_lower_bounded_range())
      << '\n';
    s << xml->end_tag(name) << '\n';
  }

}}}
