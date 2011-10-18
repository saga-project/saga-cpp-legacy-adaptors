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

#if !defined(ADAPTORS_TORQUE_JOB_STREAM_HPP)
#define ADAPTORS_TORQUE_JOB_STREAM_HPP

// stl includes
#include <iosfwd>

// saga engine includes
#include <saga/impl/engine/cpi.hpp>


///////////////////////////////////////////////////////////////////////////////
namespace impl
{
  namespace pbspro_job
  {
    template <typename Base>
    class stream
      :   public Base
    {
      private:
        typedef Base base_type;

        // a saga stream has to keep alive the proxy and the cpi instance
        TR1::shared_ptr <saga::impl::v1_0::cpi> cpi_;
        TR1::shared_ptr <saga::impl::proxy>     proxy_;


      public:
        stream (saga::impl::v1_0::job_cpi * cpi,
                std::streambuf            * buf)
          : base_type (buf),
            cpi_      (cpi->shared_from_this ()),
            proxy_    (cpi->get_proxy ()->shared_from_this ())
          {
          }
    };

  } // namespace pbspro_job
  //////////////////////////////////////////////////////////////////////

} // namespace impl
////////////////////////////////////////////////////////////////////////

#endif // !defined(ADAPTORS_TORQUE_JOB_STREAM_HPP)

