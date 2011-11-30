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

//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/task.hpp>

#include "irods_replica_adaptor.hpp"
#include "irods_replica_adaptor_logicalfile.hpp"
#include "irods_replica_adaptor_logicaldirectory.hpp"

//#include "../loader/globus_global_loader.hpp"

SAGA_ADAPTOR_REGISTER (irods_replica_adaptor::adaptor);

using namespace irods_replica_adaptor;

adaptor::adaptor()
{
//    // load the required globus modules
//    globus_module_loader::globus_init ();
}

adaptor::~adaptor()
{
//    delete RLSConnectionPool_;
}

//// register function for the SAGA engine ///////////////////////////////////
//
saga::impl::adaptor_selector::adaptor_info_list_type
adaptor::adaptor_register(saga::impl::session *s)
{
    saga::impl::adaptor_selector::adaptor_info_list_type list;
    preference_type prefs;

    logical_file_cpi_impl::register_cpi(list, prefs, adaptor_uuid_);
    logical_directory_cpi_impl::register_cpi(list, prefs, adaptor_uuid_);

    return list;
}


