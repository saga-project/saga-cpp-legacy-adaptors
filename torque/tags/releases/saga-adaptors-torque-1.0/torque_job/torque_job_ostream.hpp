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

#ifdef  ADAPTORS_TORQUE_JOB_OSTREAM_HPP
#define ADAPTORS_TORQUE_JOB_OSTREAM_HPP


// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/job.hpp>

// adaptor includes
#include "torque_job_stream.hpp"


////////////////////////////////////////////////////////////////////////
namespace torque_job
{
  class ostream : public saga::job::ostream
  {
    private:
      typedef impl::torque_job_stream <saga::adaptors::ostream_ptr>
              impl_type;

    public:
      template <typename Stream>
      ostream (saga::impl::v1_0::job_cpi * cpi,
               Stream                    & child_ostream)
        : saga::job::ostream (new impl_type (cpi, child_ostream.rdbuf ()))
      {
      }
  };

} // namespace torque_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_TORQUE_JOB_OSTREAM_HPP

