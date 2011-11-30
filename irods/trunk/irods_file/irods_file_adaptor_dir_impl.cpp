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

#include <saga/saga/exception.hpp>

#include "irods_file_adaptor_dir.hpp"
#include "irods_file_adaptor_file.hpp"

namespace irods_file_adaptor
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::dir_cpi_impl (proxy                * p,
                              cpi_info       const & info,
                              saga::ini::ini const & glob_ini,
                              saga::ini::ini const & adap_ini,
                              boost::shared_ptr<saga::adaptor> adaptor)

      : directory_cpi (p, info, adaptor, cpi::Noflags)
  {
    directory_instance_data_t d_idata(this);

    set_path_str(d_idata->location_);

    SAGA_LOG_CRITICAL("call namespace, dir, dir_cpi_impl()");
	saga::filesystem::flags flags = static_cast<saga::filesystem::flags>(d_idata->mode_);

	saga::url irods_url_org(d_idata->location_);
	std::string irods_path = irods_url_org.get_path();

	// to get rid of the / at the end the irods_path
	if (irods_path.substr(irods_path.size()-1,1) == "/"){
		irods_path.erase(irods_path.size()-1);
	}

	std::string str_buf = "irods_path =" + irods_path;
	SAGA_LOG_CRITICAL(str_buf.c_str());

	//Check the entry existence
	bool is_dir = false;
	try {
		is_dir = irdsdir.is_dir(irods_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	str_buf = "is_dir =" + is_dir;
	SAGA_LOG_CRITICAL(str_buf.c_str());

	if(is_dir){
		SAGA_LOG_CRITICAL("is_dir");
		if((flags & saga::filesystem::Create) && (flags & saga::filesystem::Exclusive)){
			SAGA_ADAPTOR_THROW ("Create and Exclusive flags are given, but the directory already exists.",
					saga::AlreadyExists);
		}
		else{
			str_buf = "dir open OK: " + irods_url_org.get_string();
			SAGA_LOG_CRITICAL(str_buf.c_str());
			is_opened = true;
		}
	}
	else{
		// Check the existence
		bool exists = false;
		exists = irdsdir.exists(irods_path);
		try {
			exists = irdsdir.exists(irods_path);
		}
		catch (boost::system::system_error const& e) {
			SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
		}

		if(exists){
			SAGA_ADAPTOR_THROW("Invalid directory name", saga::BadParameter);
		}
		else{
			if(flags & saga::filesystem::Create){
				str_buf = "call irds_dir.open: " + irods_url_org.get_string();
				SAGA_LOG_CRITICAL(str_buf.c_str());
			    irdsdir.open_dir(irods_path, flags);
				is_opened = true;
			}
			else{
				SAGA_ADAPTOR_THROW ("The directory does not exist.", saga::DoesNotExist);
			}
		}

	}
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  dir_cpi_impl::~dir_cpi_impl (void)
  {
	SAGA_LOG_DEBUG("~dir_cpi_impl()");

	if (is_opened) {
	  is_opened = false;
	} else {
	  SAGA_LOG_DEBUG("this instance was closed.");
	}

  }

//  void dir_cpi_impl::sync_get_size (saga::off_t & size_out,
//                                    saga::url      name)
//  {
//////    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
////	SAGA_LOG_DEBUG("dir, sync_get_size()");
////	check_state();
////
////	saga::url irods_url_org;
////	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
////
////	std::string irods_path;
////	boost::filesystem::path i_path(name.get_path());
////	if (i_path.has_root_path()){
////		irods_path = name.get_path();
////	}
////	else {
////		irods_path = irods_url_org.get_path() + "/" + name.get_path();
////	}
////
////	try {
////		SAGA_LOG_CRITICAL("irods_path    :" << irods_path);
////		size_out = irdsdir.get_size(irods_path);
////	}
////	catch (boost::system::system_error const& e) {
////		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
////	}
//
//  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open (saga::filesystem::file & ret,
                                saga::url                name,
                                int                      openmode)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_DEBUG("dir, sync_open()");
	check_state();

	SAGA_LOG_CRITICAL("call namespace, dir, sync_open()");

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...

	std::string str_buf = "irods_url_org: " + irods_url_org.get_string();
	SAGA_LOG_CRITICAL(str_buf.c_str());


	std::string irods_path;
	boost::filesystem::path i_path(name.get_path());
	if (i_path.has_root_path()){
		irods_path = name.get_path();
	}
	else {
		irods_path = irods_url_org.get_path() + "/" + name.get_path();
	}

	saga::url open_url;
	this->sync_get_cwd(open_url); // to get rid of the /. at the end...
	open_url.set_path(irods_path);

	str_buf = "open_url: " + open_url.get_string();
	SAGA_LOG_CRITICAL(str_buf.c_str());

	saga::filesystem::file file(open_url, openmode);
	ret = file;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_open_dir (saga::filesystem::directory & ret,
                                    saga::url                     name,
                                    int                           openmode)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_open_dir()");
	check_state();

	SAGA_LOG_CRITICAL("call namespace, dir, sync_open_dir()");

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...

	std::string str_buf = "irods_url_org: " + irods_url_org.get_string();
	SAGA_LOG_CRITICAL(str_buf.c_str());

	std::string irods_path;
	boost::filesystem::path i_path(name.get_path());
	if (i_path.has_root_path()){
		irods_path = name.get_path();
	}
	else {
		irods_path = irods_url_org.get_path() + "/" + name.get_path();
	}

	saga::url open_url;
	this->sync_get_cwd(open_url); // to get rid of the /. at the end...
	open_url.set_path(irods_path);

	str_buf = "open_url: " + open_url.get_string();
	SAGA_LOG_CRITICAL(str_buf.c_str());

	saga::filesystem::directory directory(open_url, openmode);
	ret = directory;

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_file (bool    & is_file,
                                   saga::url name)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace

