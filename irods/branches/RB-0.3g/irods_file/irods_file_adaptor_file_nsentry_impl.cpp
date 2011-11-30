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

#include "irods_file_adaptor_file.hpp"

namespace irods_file_adaptor
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_get_url (saga::url & url)
  {
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    SAGA_LOG_DEBUG("sync_get_url()");
    check_state();

    file_instance_data_t idata (this);
    url = idata->location_;
  }

  void file_cpi_impl::sync_get_cwd  (saga::url & cwd)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_CRITICAL("call namespace, file, sync_get_cwd()");

	SAGA_LOG_DEBUG("sync_get_cwd()");
	check_state();

	if (path.empty()) {
	  SAGA_ADAPTOR_THROW ("path is empty.", saga::NoSuccess);
	}

	file_instance_data_t idata (this);
	cwd = idata->location_;
	cwd.set_path(path.branch_path().string()); // remove last element.
  }

  void file_cpi_impl::sync_get_name (saga::url & name)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_get_name()");
	check_state();

	file_instance_data_t idata (this);
	boost::filesystem::path path(idata->location_.get_path());
	if (path.empty()) {
	  SAGA_ADAPTOR_THROW ("path is empty.", saga::NoSuccess);
	}
	name = saga::detail::leaf(path);
  }

  void file_cpi_impl::sync_is_dir (bool & is_dir)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_dir()");
	check_state();

	is_dir = false;
  }

  void file_cpi_impl::sync_is_entry  (bool & is_file)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_entry()");
	check_state();

	is_file = true;
  }

  void file_cpi_impl::sync_is_link   (bool & is_link)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_link()");
	check_state();

	is_link = false;
  }

  void file_cpi_impl::sync_read_link (saga::url & target)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	  SAGA_ADAPTOR_THROW ("link does not supported", saga::NotImplemented);

  }

  void file_cpi_impl::sync_copy (saga::impl::void_t & ret,
                                 saga::url dest, int flags)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_copy()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get parent path

	file_instance_data_t idata (this);
    std::string src_path = idata->location_.get_path();

	std::string dest_path;
	boost::filesystem::path i_path(dest.get_path());
	if (i_path.has_root_path()){
		dest_path = dest.get_path();
	}
	else {
		dest_path = irods_url_org.get_path() + "/" + dest.get_path();
	}

	try {
		irdfile.copy(src_path, dest_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
//	catch (api::exception const& e) {
//		SAGA_ADAPTOR_THROW (e.what(), saga::NoSuccess);
//	}


  }

  void file_cpi_impl::sync_link (saga::impl::void_t & ret,
                                 saga::url            dest,
                                 int                  flags)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	  SAGA_ADAPTOR_THROW ("link does not supported", saga::NotImplemented);
  }

  void file_cpi_impl::sync_move (saga::impl::void_t & ret,
                                 saga::url            dest,
                                 int                  flags)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_move()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
//	std::string irods_path = irods_url_org.get_path();

	file_instance_data_t idata (this);
    std::string src_path = idata->location_.get_path();

	std::string dest_path;
	boost::filesystem::path i_path(dest.get_path());
	if (i_path.has_root_path()){
		dest_path = dest.get_path();
	}
	else {
		dest_path = irods_url_org.get_path() + "/" + dest.get_path();
	}

	try {
		irdfile.move(src_path, dest_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}


  }

  void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                   int                  flags)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_DEBUG("sync_remove()");
	check_state();

//	saga::url irods_url_org;
//	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
//	std::string irods_path = irods_url_org.get_path();

	file_instance_data_t idata (this);
    std::string irods_path = idata->location_.get_path();


	try {
		irdfile.remove(irods_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

  }


  void file_cpi_impl::sync_close (saga::impl::void_t & ret,
                                  double               timeout)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_close()");
	check_state();

    if (is_opened) {
      is_opened = false;
//      std::cout << "file is closed \n" << std::endl;
    }
    else {
      SAGA_LOG_DEBUG("this instance was already closed.");
//      std::cout << "this file is already closed \n" << std::endl;
    }

  }

} // namespace

