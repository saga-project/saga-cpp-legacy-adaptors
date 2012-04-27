/*
 * Copyright (C) 2008-2012 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
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

#include "rns_replica_adaptor_logicaldirectory.hpp"

namespace rns_replica_adaptor
{

///////////////////////////////////////////////////////////////////////////////
//  constructor
logical_directory_cpi_impl::logical_directory_cpi_impl (
        proxy* p, cpi_info const& info,
        saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
        TR1::shared_ptr<saga::adaptor> adaptor)
:   base_cpi (p, info, adaptor, cpi::Noflags)
{
	instance_data   instanceData (this);

    set_path_str(instanceData->location_);

    SAGA_LOG_CRITICAL("call namespace, dir, dir_cpi_impl()");
	saga::replica::flags flags = static_cast<saga::replica::flags>(instanceData->mode_);

	saga::url rns_url_org(instanceData->location_);
	std::string rns_path = rns_url_org.get_path();

	// to get rid of the / at the end the rns_path
	if (rns_path.substr(rns_path.size()-1,1) == "/"){
		rns_path.erase(rns_path.size()-1);
	}

//	std::cout<< "rns_path =" << rns_path << std::endl;

	//Check the entry existence
	bool is_dir = false;
	try {
		is_dir = rns_ldir.is_dir(rns_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

//	std::cout<< "is_dir =" << is_dir << std::endl;
	std::string str_buf;

	if(is_dir){
//		std::cout<< "is_dir" << std::endl;
		if((flags & saga::filesystem::Create) && (flags & saga::filesystem::Exclusive)){
			SAGA_ADAPTOR_THROW ("Create and Exclusive flags are given, but the directory already exists.",
					saga::AlreadyExists);
		}
		else{
			str_buf = "dir open OK: " + rns_url_org.get_string();
			SAGA_LOG_CRITICAL(str_buf.c_str());
			is_opened = true;
		}
	}
	else{
		// Check the existence
		bool exists = false;
		try {
			exists = rns_ldir.exists(rns_path);
		}
		catch (boost::system::system_error const& e) {
			SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
		}

		if(exists){
			SAGA_ADAPTOR_THROW("Invalid directory name", saga::BadParameter);
		}
		else{
			if(flags & saga::filesystem::Create){
				str_buf = "create rns_dir: " + rns_url_org.get_string();
				SAGA_LOG_CRITICAL(str_buf.c_str());
				rns_ldir.make_dir(rns_path, flags);
				is_opened = true;
			}
			else{
				SAGA_ADAPTOR_THROW ("The directory does not exist.", saga::DoesNotExist);
			}
		}
	}

}

///////////////////////////////////////////////////////////////////////////////
//  destructor
logical_directory_cpi_impl::~logical_directory_cpi_impl (void)
{
	SAGA_LOG_DEBUG("~dir_cpi_impl()");

	if (is_opened) {
	  is_opened = false;
	} else {
	  SAGA_LOG_DEBUG("this instance was closed.");
	}
}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_file(bool& ret, saga::url name)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	bool is_entry;
    this->sync_is_entry(is_entry, name);

    ret = is_entry;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open(saga::replica::logical_file& ret,
															saga::url 	name,
															int 		openmode)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_open()");
	check_state();

	SAGA_LOG_CRITICAL("call logical dir, sync_open()");

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...

//	std::cout<< "rns_url_org: " << rns_url_org<< std::endl;

	std::string rns_path;
	boost::filesystem::path i_path(name.get_path());
	if (i_path.has_root_path()){
		rns_path = name.get_path();
	}
	else {
		rns_path = rns_url_org.get_path() + "/" + name.get_path();
	}

	saga::url open_url;
	this->sync_get_cwd(open_url); // to get rid of the /. at the end...
	open_url.set_path(rns_path);

	std::string str_buf = "open_url: " + open_url.get_string();
	SAGA_LOG_CRITICAL(str_buf.c_str());

	saga::replica::logical_file l_file(open_url, openmode);
	ret = l_file;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open_dir(saga::replica::logical_directory&  ret,
																		saga::url name,
																		int 	  openmode)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_open_dir()");
	check_state();

	SAGA_LOG_CRITICAL("call namespace, dir, sync_open_dir()");

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
//	std::cout<< "rns_url_org: " << rns_url_org<< std::endl;

	std::string rns_path;
	boost::filesystem::path i_path(name.get_path());
	if (i_path.has_root_path()){
		rns_path = name.get_path();
	}
	else {
		rns_path = rns_url_org.get_path() + "/" + name.get_path();
	}

	saga::url open_url;
	this->sync_get_cwd(open_url); // to get rid of the /. at the end...
	open_url.set_path(rns_path);

	std::string str_buf = "open_url: " + open_url.get_string();
	SAGA_LOG_CRITICAL(str_buf.c_str());

	saga::replica::logical_directory l_dir(open_url, openmode);
	ret = l_dir;

}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_find(std::vector<saga::url>& ret,
    std::string pattern, std::vector<std::string> patterns, int flags)
{

    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
}   // namespace


