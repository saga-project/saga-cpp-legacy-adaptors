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

    void logical_file_cpi_impl::check_permissions(saga::replica::flags flags,
                                                  char const* name, std::string const& adname)
    {
//        instance_data data (this);
//
//        this->check_if_open ("logical_file_cpi_impl::check_permissions", data->location_);
//
//        if (!(data->mode_ & flags)) {
//            SAGA_OSSTREAM strm;
//            strm << name << " could not access ("
//            << ((flags == saga::replica::Read) ? "read" : "write")
//            << ") : " << adname;
//            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
//                               saga::PermissionDenied);
//        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    logical_file_cpi_impl::logical_file_cpi_impl (proxy* p, cpi_info const& info,
                                                  saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
                                                  TR1::shared_ptr<saga::adaptor> adaptor)
    :   base_cpi (p, info, adaptor, cpi::Noflags)
    {

    	adaptor_data_t  adaptorData(this);
    	instance_data   instanceData (this);

        set_path_str(instanceData->location_);

    	SAGA_LOG_CRITICAL("call replica, file, logical_file_cpi_impl()");

        saga::replica::flags flags = static_cast<saga::replica::flags>(instanceData->mode_);
        saga::url irods_url_org(instanceData->location_);
        std::string irods_path = irods_url_org.get_path();

    	//Check the entry existence
    	bool is_entry = false;
    	try {
    		is_entry = irdsdir.is_entry(irods_path);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}

    	std::string str_buf = "is_entry =" + is_entry?"true":"false";
        SAGA_LOG_CRITICAL(str_buf.c_str());

    	if(is_entry){
    		SAGA_LOG_CRITICAL( "is_entry");
    		if((flags & saga::replica::Create) && (flags & saga::replica::Exclusive)){
    			SAGA_ADAPTOR_THROW ("Create and Exclusive flags are given, but the entry already exists.",
    					saga::AlreadyExists);
    		}
    		else{
    		    str_buf = "entry open OK : " + irods_url_org.get_string();
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
    			if(flags & saga::replica::Create){

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
    					str_buf = "call irds_dir.open" + irods_url_org.get_string();
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
    logical_file_cpi_impl::~logical_file_cpi_impl (void)
    {
    	is_opened = false;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // logical_file functions
    void logical_file_cpi_impl::sync_list_locations(std::vector<saga::url>& locations)
    {
//        SAGA_ADAPTOR_THROW ("Not implemented! sync_list_locations()", saga::NotImplemented);

    	SAGA_LOG_DEBUG("file sync_list_locations()");
    	check_state();

    	saga::url irods_url_org;
    	this->sync_get_url(irods_url_org);
    	std::string irods_path = irods_url_org.get_path();

    	std::vector <std::string> results;
    	std::vector <saga::url>   list_tmp;

    	try {
    		results  = irdfile.meta_list_locations(irods_path);
    		list_tmp.insert(list_tmp.begin(), results.begin(), results.end());
    		for(unsigned int i=0; i< list_tmp.size(); i++){
    			list_tmp[i].set_scheme(irods_url_org.get_scheme());
//    			list_tmp[i].set_host(irods_url_org.get_host());
    		}
    		locations = list_tmp;
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_add_location(saga::impl::void_t&, saga::url location)
    {
        SAGA_ADAPTOR_THROW ("Not implemented! iRODS cannot support add_location via SAGA.", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_remove_location(saga::impl::void_t&, saga::url location)
    {
        SAGA_ADAPTOR_THROW ("Not implemented! iRODS cannot support remove_location via SAGA.", saga::NotImplemented);

    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_update_location(saga::impl::void_t& ret,
                                                     saga::url oldlocation, saga::url newlocation)
    {
        SAGA_ADAPTOR_THROW ("Not implemented! iRODS cannot specify a physical location directly.", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_replicate(saga::impl::void_t&, saga::url location,
                                               int mode)
    {
        SAGA_ADAPTOR_THROW ("Not implemented! iRODS cannot specify a physical location directly.", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
}   // namespace logicalfile

