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
// attribute functions
///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_exists(bool& ret, std::string key)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_exists()", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir sync_attribute_exists()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_url(irods_url_org);
	std::string irods_path = irods_url_org.get_path();

	try {
		ret = irdsdir.meta_attr_exists(irods_path, key);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_readonly(bool& ret, std::string key)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_is_readonly()", saga::NotImplemented);

	// Always readable
	ret = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_writable(bool& ret, std::string key)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_is_writable()", saga::NotImplemented);

	// Always writable
	ret = true;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_vector(bool& ret, std::string key)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_is_vector()", saga::NotImplemented);

	// Always false at this moment.
	ret = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_extended(bool& ret, std::string key)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_attribute_is_extended()", saga::NotImplemented);

	// Always true?
	ret = true;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_attribute(std::string& ret, std::string key)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_attribute()", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir sync_get_attribute()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_url(irods_url_org);
	std::string irods_path = irods_url_org.get_path();

//	//Check the entry existence
//	bool exist = false;
//
//	try {
//		exist = irdsdir.meta_attr_exists(irods_path, key);
//	}
//	catch (boost::system::system_error const& e) {
//		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
//	}
//
//	if(exist){
		try {
			ret = irdsdir.meta_get_val(irods_path, key);
		}
		catch (boost::system::system_error const& e) {
			SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
		}
//	}else{
//		SAGA_ADAPTOR_THROW ("The attribute does not exist.", saga::DoesNotExist);
//	}


}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_vector_attribute(std::vector<std::string>& ret,
    std::string key)
{

    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_vector_attribute()", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_set_attribute(saga::impl::void_t&, std::string key,
    std::string val)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_set_attribute()", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir sync_set_attribute()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_url(irods_url_org);
	std::string irods_path = irods_url_org.get_path();

	try {
		irdsdir.meta(irods_path, "add", "-C", key, val);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_set_vector_attribute(saga::impl::void_t&, std::string key,
    std::vector<std::string> val)
{

    SAGA_ADAPTOR_THROW ("Not implemented! sync_set_vector_attribute()", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_remove_attribute(saga::impl::void_t&, std::string key)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_remove_attribute()", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir sync_remove_attribute()");
	check_state();

	printf("file sync_remove_attribute() \n");

	saga::url irods_url_org;
	this->sync_get_url(irods_url_org);
	std::string irods_path = irods_url_org.get_path();

	std::string val;
	try {
		val = irdsdir.meta_get_val(irods_path, key);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	try {
		irdsdir.meta(irods_path, "rm", "-C", key, val);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_list_attributes(std::vector<std::string>& keys)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_list_attributes()", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir sync_list_attributes()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_url(irods_url_org);
	std::string irods_path = irods_url_org.get_path();

	try {
		keys = irdsdir.meta_list_attr(irods_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_find_attributes(std::vector<std::string>& keys,
    std::string attr_pattern)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_find_attributes()", saga::NotImplemented);

	SAGA_LOG_DEBUG("sync_find()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_url(irods_url_org);
	std::string irods_path = irods_url_org.get_path();

	try {
		keys  = irdsdir.find_attr(irods_path, attr_pattern);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

}


}   // namespace


