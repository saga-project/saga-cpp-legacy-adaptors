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

#ifndef ADAPTORS_NAREGI_JOB_ADAPTOR_HPP
#define ADAPTORS_NAREGI_JOB_ADAPTOR_HPP

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

#include "desc_builder.hpp"
#include "wfml/workflow.hpp"

////////////////////////////////////////////////////////////////////////
namespace naregi_job
{
  namespace sa = saga::attributes;
  namespace sja = saga::job::attributes;

  struct wfml_default {
    std::map<std::string, std::string> transfer;
  };

  struct adaptor : public saga::adaptor
  {
    typedef saga::impl::v1_0::op_info         op_info;
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    // This function registers the adaptor with the factory
    // @param factory the factory where the adaptor registers
    //        its maker function and description table
    saga::impl::adaptor_selector::adaptor_info_list_type
      adaptor_register (saga::impl::session * s);

    //
    std::string get_name (void) const
    {
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }

    //
    bool init(saga::impl::session *,
	      saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini);

    typedef std::string job_id_t;
    typedef std::map<job_id_t, saga::job::description> known_jobs_t;

  private:
    known_jobs_t known_jobs_;

    // adaptor configuration file
    std::string binary_path;
    std::string wall_time_limit;

    // WFML
    wfml_default wfml_default_;
    description_builder_impl* descbuilder;
    wfml::workflow_builder* wb;

  public:
    //
    adaptor() : descbuilder(0), wb(0) {}
    //
    ~adaptor() {
      if (descbuilder != 0) {
	delete descbuilder;
      }
      if (wb != 0) {
	delete wb;
      }
    }
    //
    bool register_job(job_id_t job_id, saga::job::description jd);
    //
    bool unregister_job(job_id_t job_id);
    //
    bool find_job(job_id_t job_id, saga::job::description& jd);
    //
    wfml::workflow* create_workflow(saga::job::description& jd,
                                    std::string localhost,
                                    std::string name="saga-app");
  };

} // namespace naregi_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_NAREGI_JOB_ADAPTOR_HPP

