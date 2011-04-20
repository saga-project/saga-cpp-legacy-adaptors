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

#if !defined(ADAPTORS_TORQUE_JOB_ISTREAM_HPP)
#define ADAPTORS_TORQUE_JOB_ISTREAM_HPP

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/job.hpp>

// adaptor includes
#include "pbspro_job_stream.hpp"


////////////////////////////////////////////////////////////////////////
namespace pbspro_job
{
  class istream : public saga::job::istream
  {
    private:
      typedef impl::pbspro_job::stream <saga::adaptors::istream_ptr>
              impl_type;

    public:
      template <typename Stream>
      istream (saga::impl::v1_0::job_cpi * cpi,
               Stream                    & child_istream)
        : saga::job::istream (new impl_type (cpi, child_istream.rdbuf ()))
      {
      }
  };

} // namespace pbspro_job
////////////////////////////////////////////////////////////////////////

#endif // !defined(ADAPTORS_TORQUE_JOB_ISTREAM_HPP)

