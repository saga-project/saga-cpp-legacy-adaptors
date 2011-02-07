/*
 * Copyright (C) 2008-2009 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2009 National Institute of Informatics in Japan.
 * Copyright (C) 2011 Ole Weidner, Louisiana State University 
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
#include "pbspro_job_adaptor.hpp"
#include "pbspro_job_service.hpp"
#include "pbspro_job.hpp"

SAGA_ADAPTOR_REGISTER (pbspro_job::adaptor);


////////////////////////////////////////////////////////////////////////
namespace pbspro_job
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

  ////////////////////////////////////////////////////////////////////////
  // initialize the adaptor using the preferences, return false to
  // cancel adaptor loading
  bool adaptor::init(saga::impl::session *session,
          saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini)
  {
    if (adap_ini.has_section("cli")) {
      saga::ini::section s = adap_ini.get_section("cli");

        if (s.has_entry("binary_path")) {
    	  binary_path = s.get_entry("binary_path");
        }
        else
        {
          SAGA_ADAPTOR_THROW_NO_CONTEXT("the job was not submitted through SAGA.",
                         saga::DoesNotExist);

        }

       // disabled: 07/Feb/11 by Ole Weidner
       // this is not supported by all backends and leads to
       // errors and confusion! 
       // 
       /*if (s.has_section("description")) {
	     saga::ini::section ss = s.get_section("description");

		std::string v = ss.get_entry(sja::description_job_contact);
		saga::url mailto(v);
		if (mailto.get_scheme() != "mailto" ||
			mailto.get_path().empty()  )
			// TODO mail address format check.
		{
                  std::cout << "false" << std::endl;
		  return false;
		}
		job_contact = v;
      }*/

    }

    return true;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  bool adaptor::register_job(pbsid_t pbsid, saga::job::description jd) {
    std::pair<known_jobs_t::iterator, bool> p =
      known_jobs_.insert(known_jobs_t::value_type(pbsid, jd));
    return p.second;
  }

  ////////////////////////////////////////////////////////////////////////
  //
  bool adaptor::find_job(pbsid_t pbsid, saga::job::description& jd) {
    known_jobs_t::const_iterator i =
      find_if(known_jobs_.begin(), known_jobs_.end(),


	      boost::bind(&helper::pbsid_match, _1, pbsid));
	      //	      boost::bind<bool>(&helper::pbsid_match, _1, pbsid));
    if(i == known_jobs_.end()) {
      return false;
    }
    jd = i->second;

    return true;
  }

} // namespace pbspro_job
////////////////////////////////////////////////////////////////////////

