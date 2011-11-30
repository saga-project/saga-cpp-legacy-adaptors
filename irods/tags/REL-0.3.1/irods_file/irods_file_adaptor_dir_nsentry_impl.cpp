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

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>

#include "irods_file_adaptor_dir.hpp"

namespace irods_file_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_url (saga::url & url)
  {
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    SAGA_LOG_DEBUG("sync_get_url()");
    check_state();

    directory_instance_data_t idata (this);
    url = idata->location_;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_cwd (saga::url & cwd)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_DEBUG("sync_get_cwd()");
	check_state();

	if (path.empty()) {
	  SAGA_ADAPTOR_THROW ("path is empty.", saga::NoSuccess);
	}

	directory_instance_data_t idata (this);
	cwd = idata->location_;
	cwd.set_path(path.branch_path().string()); // remove last element.

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_get_name (saga::url & name)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
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
  void dir_cpi_impl::sync_is_dir (bool & is_dir)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_dir()");
	check_state();

	is_dir = true;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_entry (bool & is_entry)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_entry()");
	check_state();

	is_entry = false;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_is_link (bool & is_link)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_link()");
	check_state();

	is_link = false;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_read_link (saga::url & target)
  {
  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	  SAGA_ADAPTOR_THROW ("link does not supported", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_link (saga::impl::void_t & ret,
                                saga::url            dest,
                                int                  flags)
  {
  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	  SAGA_ADAPTOR_THROW ("link does not supported", saga::NotImplemented);
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_copy (saga::impl::void_t & ret,
                                saga::url            dest,
                                int                  flags)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_DEBUG("dir, sync_copy()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string dest_path;
	boost::filesystem::path i_path(dest.get_path());
	if (i_path.has_root_path()){
		dest_path = dest.get_path();
	}
	else {
		dest_path = irods_url_org.get_path() + "/" + dest.get_path();
	}

//	directory_instance_data_t idata (this);
//
//	saga::url src_url_org = idata->location_;
//	saga::url src_url (src_url_org.get_scheme() + "://" + src_url_org.get_host() +
//										myEnv.rodsHome + src_url_org.get_path());
//	saga::url tar_url = dest.get_url();
//
//	std::string src_path = src_url.get_path();
//	std::string tar_path = tar_url.get_path();

	try {
		irdsdir.copy(irods_path, dest_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}


  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_move (saga::impl::void_t & ret,
                                saga::url            dest,
                                int                  flags)
  {
	  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_move()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string dest_path;
	boost::filesystem::path i_path(dest.get_path());
	if (i_path.has_root_path()){
		dest_path = dest.get_path();
	}
	else {
		dest_path = irods_url_org.get_path() + "/" + dest.get_path();
	}

//	directory_instance_data_t idata (this);
//
//	saga::url src_url_org = idata->location_;
//	saga::url src_url (src_url_org.get_scheme() + "://" + src_url_org.get_host() +
//										myEnv.rodsHome + src_url_org.get_path());
//	saga::url tar_url = dest.get_url();
//
//	std::string src_path = src_url.get_path();
//	std::string tar_path = tar_url.get_path();

	try {
		irdsdir.move(irods_path, dest_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                  int                  flags)
  {
  //    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_DEBUG("dir sync_remove()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

//	directory_instance_data_t idata (this);
//
//	saga::url irods_url_org = idata->location_;
//	saga::url irods_url (irods_url_org.get_scheme() + "://" + irods_url_org.get_host() +
//										myEnv.rodsHome + irods_url_org.get_path());
//
//	std::string irods_path = irods_url.get_path();

	try {
		irdsdir.remove(irods_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void dir_cpi_impl::sync_close (saga::impl::void_t & ret,
                                 double               timeout)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_close()");
	check_state();

	if (is_opened) {
	  is_opened = false;
//	  std::cout << "file is closed \n" << std::endl;
	}
	else {
	  SAGA_LOG_DEBUG("this instance was already closed.");
//	  std::cout << "this file is already closed \n" << std::endl;
	}
  }

} // namespace

