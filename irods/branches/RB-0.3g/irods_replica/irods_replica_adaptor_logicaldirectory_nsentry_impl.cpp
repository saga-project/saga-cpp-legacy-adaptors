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

#include "irods_replica_adaptor_logicaldirectory.hpp"

namespace irods_replica_adaptor
{

///////////////////////////////////////////////////////////////////////////////
// namespace_entry functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_url(saga::url& url)
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
void logical_directory_cpi_impl::sync_get_cwd(saga::url& cwd)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_cwd", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_get_cwd()");
	check_state();

	if (path.empty()) {
	  SAGA_ADAPTOR_THROW ("path is empty.", saga::NoSuccess);
	}

	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);

	cwd = instanceData->location_;
	cwd.set_path(path.branch_path().string()); // remove last element.
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_name(saga::url& name)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_name", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_get_name()");
	check_state();

	saga::url u;
    this->sync_get_cwd(u); // to get rid of the /. at the end...

    boost::filesystem::path path(u.get_path());
    std::string path_str(u.get_path());

    if (path.empty()) {
	  SAGA_ADAPTOR_THROW ("path is empty.", saga::NoSuccess);
	}

    if( !path.has_root_path() )
        path = boost::filesystem::path("/"+path_str);

    path_str = path.string();
    std::string::size_type idx = path_str.rfind("/");

    ( idx == 0 ) ? path = boost::filesystem::path(path_str.substr(1, path_str.size()-1)) :
    path = boost::filesystem::path(path_str.substr(idx+1, path_str.size()-1));

    if( path.string().size() == 0 ) path = boost::filesystem::path("/");

    name = path.string();
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_dir(bool& is_dir)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_dir", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_dir()");
	check_state();

	is_dir = true;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_entry(bool& is_entry)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_entry", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_entry()");
	check_state();

	is_entry = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_link(bool& is_link)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_link", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_link()");
	check_state();

	is_link = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_remove(saga::impl::void_t&, int flags)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_remove", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir sync_remove()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	try {
		irdsdir.remove(irods_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_close(saga::impl::void_t&, double)
{
//	SAGA_ADAPTOR_THROW ("Not implemented! sync_close", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_close()");
	check_state();

	if (is_opened) {
	  is_opened = false;
	  SAGA_LOG_CRITICAL("file is closed \n");
	}
	else {
	  SAGA_LOG_DEBUG("this instance was already closed.");
	}
}

///////////////////////////////////////////////////////////////////////////////
}   // namespace


