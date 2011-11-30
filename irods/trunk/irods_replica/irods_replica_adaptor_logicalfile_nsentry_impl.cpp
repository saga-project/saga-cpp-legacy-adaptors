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

#include <map>
#include <vector>

#include <saga/saga/util.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/task.hpp>
//#include <saga/saga/file.hpp>	// SAGA < 1.5
#include <saga/saga/filesystem.hpp>

#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

//#include "irods_replica_adaptor_connection.hpp"
#include "irods_replica_adaptor_logicalfile.hpp"

namespace irods_replica_adaptor
{

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_get_url(saga::url& url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_url", saga::NotImplemented);
    SAGA_LOG_DEBUG("sync_get_url()");
    check_state();

	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);

	saga::url lfn_url(instanceData->location_.get_url());
    url = lfn_url;

}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_get_cwd(saga::url& url)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_cwd", saga::NotImplemented);
    SAGA_LOG_DEBUG("sync_get_url()");
    check_state();

	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);

	saga::url cwd = instanceData->location_;
	cwd.set_path(path.branch_path().string()); // remove last element.

	saga::url lfn_url(instanceData->location_.get_url());
    url = cwd;

}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_get_name(saga::url& url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_name", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_get_name()");
	check_state();

	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	boost::filesystem::path path(instanceData->location_.get_path());
	if (path.empty()) {
	  SAGA_ADAPTOR_THROW ("path is empty.", saga::NoSuccess);
	}
	url = saga::detail::leaf(path);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_is_dir(bool& ret)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_dir", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_dir()");
	check_state();

	ret = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_is_entry(bool& ret)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_entry", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_entry()");
	check_state();

	ret = true;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_is_link(bool& ret)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_link", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_link()");
	check_state();

	ret = false;

}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_remove(saga::impl::void_t&, int flags)
{
	printf("file sync_remove \n");
    SAGA_ADAPTOR_THROW ("Not implemented! sync_remove()", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_close(saga::impl::void_t&, double)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_close", saga::NotImplemented);
	is_opened = false;
}

}
