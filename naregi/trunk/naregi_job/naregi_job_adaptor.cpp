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

//  Copyright (c) 2005-2007 Hartmut Kaiser
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE or copy at
//   http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

// adaptor includes
#include "naregi_job_adaptor.hpp"
#include "naregi_job_service.hpp"
#include "naregi_job.hpp"

SAGA_ADAPTOR_REGISTER (naregi_job::adaptor);


////////////////////////////////////////////////////////////////////////
namespace naregi_job
{
  // register function for the SAGA engine
  saga::impl::adaptor_selector::adaptor_info_list_type
    adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type list;

    // create empty preference list
    // these list should be filled with properties of the adaptor,
    // which can be used to select adaptors with specific preferences.
    // Example:
    //   'security' -> 'gsi'
    //   'logging'  -> 'yes'
    //   'auditing' -> 'no'
    preference_type prefs;

    // create file adaptor infos (each adaptor instance gets its own uuid)
    // and add cpi_infos to list
    job_service_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);
    job_cpi_impl::register_cpi         (list, prefs, adaptor_uuid_);

    // and return list
    return (list);
  }

  //
  bool adaptor::init(saga::impl::session *session,
          saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini)
  {
#if 0
    typedef std::pair <std::string, saga::ini::section>     section_pair;
    typedef std::pair <std::string, std::string> entry_pair;

    std::cout << "// glob_ini.get_sections()" << std::endl;
    BOOST_FOREACH(section_pair s, glob_ini.get_sections()) {
      std::cout << "[" << s.first << "]=\""
		<< s.second.get_name() << "\"" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "// adap_ini.get_sections()" << std::endl;
    BOOST_FOREACH(section_pair s, adap_ini.get_sections()) {
      std::cout << "[" << s.first << "]=\""
		<< s.second.get_name() << "\"" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "// glob_ini.get_entries()" << std::endl;
    BOOST_FOREACH(entry_pair e, glob_ini.get_entries()) {
      std::cout << "[" << e.first << "]=\""
		<< e.second << "\"" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "// adap_ini.get_entries()" << std::endl;
    BOOST_FOREACH(entry_pair e, adap_ini.get_entries()) {
      std::cout << "[" << e.first << "]=\""
		<< e.second << "\"" << std::endl;
    }
#endif

    if (adap_ini.has_section("cli")) {
      saga::ini::section s = adap_ini.get_section("cli");

      if (s.has_entry("binary_path")) {
	// TODO
      }
    }

    if (adap_ini.has_section("wfml")) {
      saga::ini::section s = adap_ini.get_section("wfml");

      if (s.has_section(wfml::element::transfer)) {
	saga::ini::section ss =
	  s.get_section(wfml::element::transfer);
	std::string v = ss.get_entry(wfml::attribute::wall_time_limit);
	// TODO check
        //  throw saga::BadParameter
	wfml_default_.transfer[wfml::attribute::wall_time_limit] = v;
      }
    }

    return true;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  bool adaptor::register_job(job_id_t job_id, saga::job::description jd)
  {
    std::pair<known_jobs_t::iterator, bool> p =
      known_jobs_.insert(known_jobs_t::value_type(job_id, jd));
    return p.second;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  bool adaptor::unregister_job(job_id_t job_id)
  {
    known_jobs_.erase(job_id);
    return false;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  bool adaptor::find_job(job_id_t job_id, saga::job::description& jd)
  {
    known_jobs_t::const_iterator i = known_jobs_.find(job_id);
    if(i == known_jobs_.end()) {
      return false;
    }

    jd = i->second;
    return true;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  wfml::workflow* adaptor::create_workflow(saga::job::description& jd,
                                           std::string localhost,
                                           std::string name)
  {
    if (descbuilder == 0) {
      descbuilder = new description_builder_impl();
      // adaptor configuration defined => builder
    }

    if (wb == 0) {
      wfml::activity_factory_ptr act_fc(new wfml::activity_factory());

      act_fc->set_transfer_default(wfml_default_.transfer);

      wb = new wfml::workflow_builder(act_fc);
    }

    descbuilder->set_saga_job_description(&jd);

    boost::scoped_ptr<jsdl::description>
      desc(descbuilder->get_description());

    return wb->get_workflow(name, localhost, desc.get());
  }

} // namespace naregi_job
////////////////////////////////////////////////////////////////////////

