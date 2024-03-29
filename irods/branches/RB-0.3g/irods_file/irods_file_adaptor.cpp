/*
 * Copyright (C) 2008-2011 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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

//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
// 
//  Distributed under the Boost Software License, 
//  Version 1.0. (See accompanying LICENSE file 
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/adaptors/adaptor.hpp>

#include "irods_file_adaptor.hpp"
#include "irods_file_adaptor_dir.hpp"
#include "irods_file_adaptor_file.hpp"

namespace irods_file_adaptor
{
  SAGA_ADAPTOR_REGISTER (file_adaptor);

  ///////////////////////////////////////////////////////////////////////////////
  //
  saga::impl::adaptor_selector::adaptor_info_list_type
    file_adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type infos;
    preference_type prefs; 

    file_cpi_impl::register_cpi(infos, prefs, adaptor_uuid_);
    dir_cpi_impl::register_cpi(infos, prefs, adaptor_uuid_);

    return infos;
  }

} // namespace

