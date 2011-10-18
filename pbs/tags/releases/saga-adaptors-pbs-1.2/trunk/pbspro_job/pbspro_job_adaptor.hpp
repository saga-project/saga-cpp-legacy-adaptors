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

#ifndef ADAPTORS_TORQUE_JOB_ADAPTOR_HPP
#define ADAPTORS_TORQUE_JOB_ADAPTOR_HPP

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

////////////////////////////////////////////////////////////////////////
namespace pbspro_job
{
  namespace sa = saga::attributes;
  namespace sja = saga::job::attributes;

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

    std::string get_name (void) const
    {
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }

    bool init(saga::impl::session *,
	      saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini);

    typedef std::string pbsid_t;
    typedef std::map<pbsid_t, saga::job::description> known_jobs_t;

  private:
    // from adaptor configuration file
    std::string binary_path;
    std::string job_contact;

    known_jobs_t known_jobs_;

  public:
    //
    bool register_job(pbsid_t pbsid, saga::job::description jd);
    //
    bool find_job(pbsid_t pbsid, saga::job::description& jd);
    //
    std::string get_job_contact() { return job_contact; }

    std::string get_binary_path() { return binary_path; }
  };

} // namespace pbspro_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_TORQUE_JOB_ADAPTOR_HPP

