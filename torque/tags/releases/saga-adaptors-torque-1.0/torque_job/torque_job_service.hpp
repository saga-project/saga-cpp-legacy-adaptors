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

#ifndef ADAPTORS_TORQUE_JOB_SERVICE_HPP
#define ADAPTORS_TORQUE_JOB_SERVICE_HPP

// stl includes
#include <string>
#include <iosfwd>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/engine/proxy.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

// saga package includes
#include <saga/impl/packages/job/job_service_cpi.hpp>

// adaptor includes
#include "torque_job_adaptor.hpp"
#include "torque_helper.hpp"


////////////////////////////////////////////////////////////////////////
namespace torque_job
{
  class job_service_cpi_impl
    : public saga::adaptors::v1_0::job_service_cpi <job_service_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::job_service_cpi <job_service_cpi_impl>
              base_cpi;

      // adaptor data
      typedef saga::adaptors::adaptor_data <adaptor> adaptor_data_type;

      std::string localhost;
      helper::jobid_converter jobid_converter;

    public:
      // constructor of the job_service cpi
      job_service_cpi_impl  (proxy                           * p,
                             cpi_info const                  & info,
                             saga::ini::ini const            & glob_ini,
                             saga::ini::ini const            & adap_ini,
                             TR1::shared_ptr <saga::adaptor>   adaptor);

      // destructor of the job_service cpi
      ~job_service_cpi_impl (void);

      // CPI functions
      void sync_create_job (saga::job::job            & ret,
                            saga::job::description      jd);
      void sync_run_job    (saga::job::job            & ret,
                            std::string                 cmd,
                            std::string                 host,
                            saga::job::ostream        & in,
                            saga::job::istream        & out,
                            saga::job::istream        & err);
      void sync_run_job_noio (saga::job::job          & ret,
                              std::string               cmd,
                              std::string               host);
      void sync_list       (std::vector <std::string> & ret);
      void sync_get_job    (saga::job::job            & ret,
                            std::string                 jobid);
      void sync_get_self   (saga::job::self           & ret);

      // This adaptor implements the async functions
      // based on its own synchronous functions.
      saga::task async_create_job (saga::job::description      jd);
      saga::task async_run_job    (std::string                 cmd,
                                   std::string                 host,
                                   saga::job::ostream        & in,
                                   saga::job::istream        & out,
                                   saga::job::istream        & err);
      saga::task async_list       (void);
      saga::task async_get_job    (std::string                 jobid);
      saga::task async_get_self   (void);

  };  // class job_service_cpi_impl

} // namespace torque_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_TORQUE_JOB_SERVICE_HPP

