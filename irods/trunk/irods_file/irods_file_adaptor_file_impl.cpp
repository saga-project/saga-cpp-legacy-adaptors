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
  file_cpi_impl::file_cpi_impl (proxy                * p,
                                cpi_info       const & info,
                                saga::ini::ini const & glob_ini,
                                saga::ini::ini const & adap_ini,
                                boost::shared_ptr <saga::adaptor> adaptor)

      : file_cpi (p, info, adaptor, cpi::Noflags)
  {
    //Initialize seek operation
    irdseek.offset = 0;
    irdseek.seek_mode = SEEK_CUR;

	file_instance_data_t f_idata(this);
    set_path_str(f_idata->location_);

//	std::cout<< "call namespace, file, dir_cpi_impl()" << std::endl;

    saga::filesystem::flags flags = static_cast<saga::filesystem::flags>(f_idata->mode_);
    saga::url irods_url_org(f_idata->location_);
    std::string irods_path = irods_url_org.get_path();

	//Check the entry existence
	bool is_entry = false;
	try {
		is_entry = irdsdir.is_entry(irods_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	std::string str_buf = "is_entry =" + is_entry;
	SAGA_LOG_CRITICAL(str_buf.c_str());

	if(is_entry){
		SAGA_LOG_CRITICAL("is_entry");
		if((flags & saga::filesystem::Create) && (flags & saga::filesystem::Exclusive)){
			SAGA_ADAPTOR_THROW ("Create and Exclusive flags are given, but the entry already exists.",
					saga::AlreadyExists);
		}
		else{
			str_buf = "entry open OK" + irods_url_org.get_string();
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
			SAGA_ADAPTOR_THROW("invalid entry name", saga::BadParameter);
		}
		else{
			if(flags & saga::filesystem::Create){

				// Check parent directory
				bool is_p_dir = false;
				boost::filesystem::path check_url_org(irods_path);
				std::string check_url_bpath = check_url_org.branch_path().string();

				try {
					is_p_dir = irdsdir.is_dir(check_url_bpath);
				}
				catch (boost::system::system_error const& e) {
					SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
				}

				if(is_p_dir){
					str_buf = "call irds_dir.open: " + irods_url_org.get_string();
					SAGA_LOG_CRITICAL(str_buf.c_str());
					irdsdir.open(irods_path, flags);
					is_opened = true;
				}
				else {
//					SAGA_ADAPTOR_THROW("parent directory does not exist", saga::BadParameter);
					SAGA_ADAPTOR_THROW("parent directory does not exist:" + check_url_bpath,
							saga::BadParameter);
				}
			}
			else{
		        SAGA_ADAPTOR_THROW ("the entry does not exist.", saga::DoesNotExist);
			}
		}

	}
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  file_cpi_impl::~file_cpi_impl (void)
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
  void file_cpi_impl::sync_get_size (saga::off_t & size_out)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_DEBUG("sync_get_size()");
	check_state();

    file_instance_data_t idata (this);
    std::string irods_path = idata->location_.get_path();

	try {
		size_out = irdfile.get_size(irods_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}


  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_read (saga::ssize_t        & len_out,
                                 saga::mutable_buffer   data,
                                 saga::ssize_t          len_in)
  {
	  //  SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);

	SAGA_LOG_DEBUG("sync_read()");
	check_state();

	file_instance_data_t idata (this);
	std::string irods_path = idata->location_.get_path();

	off_t offset = irdseek.offset;
	int seek_mode = irdseek.seek_mode;

	char* buf = static_cast<char*>(data.get_data());

	try {
		SAGA_LOG_CRITICAL( "sync_read start" );
	  std::size_t len = irdfile.read(irods_path, buf , len_in, offset, seek_mode);
	  len_out = len;
	  irdseek.offset = offset + len;
	}catch (boost::system::system_error const& e) {
	  SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_write (saga::ssize_t      & len_out,
                                  saga::const_buffer   data,
                                  saga::ssize_t        len_in)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_write()");
	check_state();

	file_instance_data_t idata (this);
	std::string irods_path = idata->location_.get_path();

	off_t offset = irdseek.offset;
	int seek_mode = irdseek.seek_mode;
	char* buf = (char*)data.get_data();

	try {
		SAGA_LOG_CRITICAL("sync_write start");
	  std::size_t len = irdfile.write(irods_path, buf , len_in, offset, seek_mode);
	  len_out = len;
	  irdseek.offset = offset + len;
	}catch (boost::system::system_error const& e) {
	  SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  void file_cpi_impl::sync_seek (saga::off_t                 & out,
                                 saga::off_t                   offset,
                                 saga::filesystem::seek_mode   whence)
  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_seek()");
	check_state();

	switch(whence){
		case saga::filesystem::Start:
			irdseek.seek_mode = SEEK_SET;
			break;
		case saga::filesystem::Current:
			irdseek.seek_mode = SEEK_SET;
			offset += irdseek.offset;
			break;
		case saga::filesystem::End:
			irdseek.seek_mode = SEEK_END;
			break;
		default:
			irdseek.seek_mode = SEEK_CUR;
			break;
	}

	irdseek.offset = offset;
  }

} // namespace

