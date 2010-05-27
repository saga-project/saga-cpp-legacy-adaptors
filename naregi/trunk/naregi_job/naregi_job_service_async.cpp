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

// stl includes
#include <vector>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/config.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

// adaptor includes
#include "naregi_job_service.hpp"


////////////////////////////////////////////////////////////////////////
namespace naregi_job
{

  //////////////////////////////////////////////////////////////////////
  // This adaptor implements the async functions
  // based on its own synchronous functions.

  saga::task
    job_service_cpi_impl::async_create_job (saga::job::description   jd)
  {
    return saga::adaptors::task ("job_service_cpi_impl::async_create_job",
                                 shared_from_this (),
                                 &job_service_cpi_impl::sync_create_job,
                                 jd);
  }

  saga::task
    job_service_cpi_impl::async_run_job (std::string          cmd,
                                         std::string          host,
                                         saga::job::ostream & in,
                                         saga::job::istream & out,
                                         saga::job::istream & err)
  {
    return saga::adaptors::task ("job_service_cpi_impl::async_run_job",
                                 shared_from_this (),
                                 &job_service_cpi_impl::sync_run_job,
                                 cmd,
                                 host,
                                 TR1::ref (in),
                                 TR1::ref (out),
                                 TR1::ref (err));
  }

  saga::task
    job_service_cpi_impl::async_list (void)
  {
    return saga::adaptors::task ("job_service_cpi_impl::async_list",
                                 shared_from_this (),
                                 &job_service_cpi_impl::sync_list);
  }

  saga::task
    job_service_cpi_impl::async_get_job (std::string      jobid)
  {
    return saga::adaptors::task ("job_service_cpi_impl::async_get_job",
                                 shared_from_this (),
                                 &job_service_cpi_impl::sync_get_job,
                                 jobid);
  }

  saga::task
    job_service_cpi_impl::async_get_self (void)
  {
    return saga::adaptors::task ("job_service_cpi_impl::async_get_self",
                                 shared_from_this (),
                                 &job_service_cpi_impl::sync_get_self);
  }

} // namespace naregi_job
////////////////////////////////////////////////////////////////////////

