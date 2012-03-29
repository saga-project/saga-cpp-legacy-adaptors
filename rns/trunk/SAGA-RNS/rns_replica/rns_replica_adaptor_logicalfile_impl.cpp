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

#include "rns_replica_adaptor_logicalfile.hpp"

namespace rns_replica_adaptor
{
    ///////////////////////////////////////////////////////////////////////////////
    //
    logical_file_cpi_impl::logical_file_cpi_impl (proxy* p, cpi_info const& info,
                                                  saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
                                                  TR1::shared_ptr<saga::adaptor> adaptor)
    :   base_cpi (p, info, adaptor, cpi::Noflags)
    {

        instance_data instanceData(this);
        set_path_str(instanceData->location_);

        saga::replica::flags flags = static_cast<saga::replica::flags>(instanceData->mode_);
        saga::url rns_url_org(instanceData->location_);
        std::string rns_path = rns_url_org.get_path();

    	//Check the entry existence
    	bool is_entry = false;
    	try {
    		is_entry = rns_ldir.is_entry(rns_path);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}

//    	std::cout<< "is_entry =" << is_entry << std::endl;
    	std::string str_buf;

    	if(is_entry){
//    		std::cout<< "is_entry" << std::endl;
    		if((flags & saga::filesystem::Create) && (flags & saga::filesystem::Exclusive)){
    			SAGA_ADAPTOR_THROW ("Create and Exclusive flags are given, but the entry already exists.",
    					saga::AlreadyExists);
    		}
    		else{
    			str_buf = "entry open OK: " + rns_url_org.get_string();
    			SAGA_LOG_CRITICAL(str_buf.c_str());
    		    is_opened = true;
    		}
    	}
    	else{
    		// Check the existence
    		bool exists = false;
    		exists = rns_ldir.exists(rns_path);
    		try {
    			exists = rns_ldir.exists(rns_path);
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
    				boost::filesystem::path check_url_org(rns_path);
    				std::string check_url_bpath = check_url_org.branch_path().string();

    				try {
    					is_p_dir = rns_ldir.is_dir(check_url_bpath);
    				}
    				catch (boost::system::system_error const& e) {
    					SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    				}

    				if(is_p_dir){
    					str_buf = "call rns_ldir open. : " + rns_url_org.get_string();
    	    			SAGA_LOG_CRITICAL(str_buf.c_str());
    	    			SAGA_LOG_CRITICAL("But NOT created until adding a location");
//    					rns_ldir.open(rns_path, flags);
    					is_opened = true;
    				}
    				else {
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
    	if (is_opened) {
    	  is_opened = false;
    	} else {
    	  SAGA_LOG_DEBUG("this instance was closed.");
    	}
    }

    ///////////////////////////////////////////////////////////////////////////////
    // logical_file functions
    void logical_file_cpi_impl::sync_list_locations(std::vector<saga::url>& locations)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file sync_list_locations()");
    	check_state();

    	instance_data   instanceData (this);
        std::string rns_path = instanceData->location_.get_path();

    	std::vector <std::string> results;
    	std::vector <saga::url>   list_tmp;

    	try {
    		results = rns_lfile.list_locations(rns_path);
    		list_tmp.insert(list_tmp.begin(), results.begin(), results.end());
//    		for(unsigned int i=0; i< list_tmp.size(); i++){
//    			list_tmp[i].set_scheme(irods_url_org.get_scheme());
//    		}
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

//        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file, sync_add_location()");
    	check_state();

    	instance_data   instanceData (this);
        std::string rns_path = instanceData->location_.get_path();
        std::string loc_url = location.get_string();

//    	bool exists = false;
//    	try {
//    		exists  = rns_ldir.exists(rns_path);
//    	}
//    	catch (boost::system::system_error const& e) {
//    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
//    	}
//
//    	if (exists){
////    		SAGA_ADAPTOR_THROW ("The specified target already exists. ", saga::AlreadyExists);
//    		std::cout<< "The specified target already exists." << rns_path << std::endl;
//    		std::cout<< "Adding the location : " << location << std::endl;
//    		// rns_lfile.add_location??
//    	}


    	try {
//    		std::cout<< "rns_path : " << rns_path << std::endl;
//    		std::cout<< "loc_url  : " << loc_url  << std::endl;
    		rns_lfile.add_location(rns_path, loc_url);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}


    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_remove_location(saga::impl::void_t&, saga::url location)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_update_location(saga::impl::void_t& ret,
                                                     saga::url oldlocation, saga::url newlocation)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_replicate(saga::impl::void_t&, saga::url location,
                                               int mode)
    {
        SAGA_ADAPTOR_THROW ("Not implemented yet!", saga::NotImplemented);
    }


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
}   // namespace logicalfile

