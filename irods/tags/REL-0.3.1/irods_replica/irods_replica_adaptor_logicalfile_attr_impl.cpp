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
    void logical_file_cpi_impl::sync_attribute_exists(bool& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_exists()", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file sync_attribute_exists()");
    	check_state();

    	saga::url irods_url_org;
    	this->sync_get_url(irods_url_org);
    	std::string irods_path = irods_url_org.get_path();

    	std::string val;
    	try {
    		ret = irdfile.meta_attr_exists(irods_path, key);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}

    }

    void
      logical_file_cpi_impl::sync_attribute_is_extended(bool & ret, std::string key)
      {
    	// Always true??
    	ret = true;
      }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_readonly(bool& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_is_readonly()", saga::NotImplemented);

//    	printf("file sync_attribute_is_readonly() \n");
    	// Always writable
    	ret = false;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_writable(bool& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_is_writable()", saga::NotImplemented);

    	printf("file sync_attribute_is_writable() \n");
    	// Always writable
    	ret = true;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_vector(bool& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_is_vector()", saga::NotImplemented);

    	// Always false at this moment.
    	ret = false;
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_get_attribute(std::string& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_get_attribute()", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file sync_get_attribute()");
    	check_state();

    	saga::url irods_url_org;
    	this->sync_get_url(irods_url_org);
    	std::string irods_path = irods_url_org.get_path();

    	try {
    		ret = irdfile.meta_get_val(irods_path, key);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_set_attribute(saga::impl::void_t& ret, std::string key,
                                                   std::string val)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_set_attribute()", saga::NotImplemented);
		SAGA_LOG_DEBUG("file sync_set_attribute()");
		check_state();

		saga::url irods_url_org;
		this->sync_get_url(irods_url_org);
		std::string irods_path = irods_url_org.get_path();

		try {
			irdfile.meta(irods_path, "add", "-d", key, val);
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

        SAGA_ADAPTOR_THROW ("Not implemented! sync_get_vector_attribute()", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_set_vector_attribute(saga::impl::void_t&, std::string key,
                                                          std::vector<std::string> val)
    {

        SAGA_ADAPTOR_THROW ("Not implemented! sync_set_vector_attribute()", saga::NotImplemented);
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_remove_attribute(saga::impl::void_t& ret, std::string key)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_remove_attribute()", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file sync_remove_attribute()");
    	check_state();

    	printf("file sync_remove_attribute() \n");

    	saga::url irods_url_org;
    	this->sync_get_url(irods_url_org);
    	std::string irods_path = irods_url_org.get_path();

		std::string val;
    	try {
    		val = irdfile.meta_get_val(irods_path, key);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}

    	try {
    		irdfile.meta(irods_path, "rm", "-d", key, val);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}


    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_list_attributes(std::vector<std::string>& keys)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_list_attributes()", saga::NotImplemented);
    	SAGA_LOG_DEBUG("file sync_list_attributes()");
    	check_state();

    	saga::url irods_url_org;
    	this->sync_get_url(irods_url_org);
    	std::string irods_path = irods_url_org.get_path();

    	try {
    		keys = irdfile.meta_list_attr(irods_path);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}
    }

    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_find_attributes(std::vector<std::string>& keys,
                                                     std::string attr_pattern)
    {

//        SAGA_ADAPTOR_THROW ("Not implemented! sync_find_attributes()", saga::NotImplemented);
    	SAGA_LOG_DEBUG("sync_find()");
    	check_state();

    	saga::url irods_url_org;
    	this->sync_get_url(irods_url_org);
    	std::string irods_path = irods_url_org.get_path();

    	try {
    		keys  = irdfile.find_attr(irods_path, attr_pattern);
    	}
    	catch (boost::system::system_error const& e) {
    		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
    	}



    }
}
