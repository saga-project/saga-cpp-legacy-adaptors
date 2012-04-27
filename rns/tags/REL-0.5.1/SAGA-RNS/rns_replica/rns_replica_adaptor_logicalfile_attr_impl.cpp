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
    void logical_file_cpi_impl::sync_attribute_exists(bool& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file, sync_attribute_exists()");
    	check_state();

    	instance_data   instanceData (this);
        std::string rns_path = instanceData->location_.get_path();

    	try {
//    		std::cout<< "rns_path : " << rns_path << std::endl;
//    		std::cout<< "key(attribute)  : " << key  << std::endl;
    		ret = rns_lfile.attribute_exists(rns_path, key);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}

    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_readonly(bool& ret, std::string key)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_writable(bool& ret, std::string key)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_vector(bool& ret, std::string key)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_get_attribute(std::string& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
		SAGA_LOG_DEBUG("file, sync_get_attribute()");
		check_state();

		instance_data   instanceData (this);
		std::string rns_path = instanceData->location_.get_path();

		try {
//			std::cout<< "rns_path : " << rns_path << std::endl;
//			std::cout<< "key  : " << key  << std::endl;
			ret = rns_lfile.get_attribute(rns_path, key);
		}
		catch (boost::system::system_error const& e) {
			SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
		}
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_set_attribute(saga::impl::void_t&, std::string key,
                                                   std::string val)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file, sync_set_attribute()");
    	check_state();

    	instance_data   instanceData (this);
        std::string rns_path = instanceData->location_.get_path();

    	try {
//    		std::cout<< "rns_path : " << rns_path << std::endl;
//    		std::cout<< "key  : " << key  << std::endl;
//    		std::cout<< "value: " << val  << std::endl;
    		rns_lfile.set_attribute(rns_path, key, val);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}


    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_get_vector_attribute(std::vector<std::string>& ret,
                                                          std::string key)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_set_vector_attribute(saga::impl::void_t&, std::string key,
                                                          std::vector<std::string> val)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_remove_attribute(saga::impl::void_t&, std::string key)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_list_attributes(std::vector<std::string>& keys)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file sync_list_attributes()");
    	check_state();

    	instance_data   instanceData (this);
        std::string rns_path = instanceData->location_.get_path();

    	try {
    		keys = rns_lfile.list_attributes(rns_path);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_find_attributes(std::vector<std::string>& keys,
                                                     std::string pattern)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
    }
}
