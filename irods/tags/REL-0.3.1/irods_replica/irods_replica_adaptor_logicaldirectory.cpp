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
//  constructor
logical_directory_cpi_impl::logical_directory_cpi_impl (
        proxy* p, cpi_info const& info,
        saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
        TR1::shared_ptr<saga::adaptor> adaptor)
:   base_cpi (p, info, adaptor, cpi::Noflags)
{
    adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);

	set_path_str(instanceData->location_);

    std::cout<< "call replica, dir, logical_directory_cpi_impl()" << std::endl;
	saga::replica::flags flags = static_cast<saga::replica::flags>(instanceData->mode_);

	saga::url irods_url_org(instanceData->location_);
	std::string irods_path = irods_url_org.get_path();

	// to get rid of the / at the end the irods_path
	if (irods_path.substr(irods_path.size()-1,1) == "/"){
		irods_path.erase(irods_path.size()-1);
	}

	std::cout<< "irods_path =" << irods_path << std::endl;

	//Check the entry existence
	bool is_dir = false;
	try {
		is_dir = irdsdir.is_dir(irods_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	std::cout<< "is_dir =" << is_dir << std::endl;

	if(is_dir){
		if((flags & saga::replica::Create) && (flags & saga::replica::Exclusive)){
			SAGA_ADAPTOR_THROW ("Create and Exclusive flags are given, but the directory already exists.",
					saga::AlreadyExists);
		}
		else{
			std::cout<< "dir open OK: " << irods_url_org << std::endl;
			is_opened = true;
		}
	}
	else{
		// Check the existence
		bool exists = false;
		try {
			exists = irdsdir.exists(irods_path);
		}
		catch (boost::system::system_error const& e) {
			SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
		}

		std::cout<< "exists =" << exists << std::endl;

		if(exists){
//			std::cout<< "Invalid directory name" << std::endl;
			SAGA_ADAPTOR_THROW("Invalid directory name", saga::BadParameter);
		}
		else{
			if(flags & saga::replica::Create){
				std::cout<< "call irds_dir.open: " << irods_url_org << std::endl;
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
//  destructor
logical_directory_cpi_impl::~logical_directory_cpi_impl (void)
{
}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_file(bool& ret, saga::url name)
{

    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_file()", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open(saga::replica::logical_file& ret,
															saga::url 	name,
															int 		openmode)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_open", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_open()");
	check_state();

	std::cout<< "logical_directory, sync_open()" << std::endl;

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...

	std::cout<< "irods_url_org: " << irods_url_org<< std::endl;

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

	std::cout<< "open_url: " << open_url<< std::endl;

	saga::filesystem::file file(open_url, openmode);
	ret = file;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open_dir(saga::replica::logical_directory&  ret,
																		saga::url name,
																		int 	  openmode)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_open_dir", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_open_dir()");
	check_state();

	std::cout<< "logical_directory, sync_open_dir()" << std::endl;

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...

	std::cout<< "irods_url_org: " << irods_url_org<< std::endl;

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

	std::cout<< "open_url: " << open_url<< std::endl;

	saga::filesystem::directory directory(open_url, openmode);
	ret = directory;
}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_find(std::vector<saga::url>& list,
    std::string name_pattern, std::vector<std::string> attr_patterns, int flags)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_find()", saga::NotImplemented);
	SAGA_LOG_DEBUG("logical_directory, sync_find()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::vector <std::string> ent_list;
	std::vector <std::string> fnd_list;
	std::vector <saga::url>   list_tmp;

	try {
		ent_list  = irdsdir.find(irods_path, name_pattern, flags);
//		list_tmp.insert(list_tmp.begin(), results.begin(), results.end());
//		list = list_tmp;
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

//	for (unsigned int xx=0; xx<ent_list.size(); xx++){
//		std::cout << "ent_list_" << xx << "=" << ent_list[xx] << std::endl;
//	}

	for(unsigned int i=0; i<ent_list.size(); i++){
		if(irdsdir.is_dir(ent_list[i])){
			for(unsigned int j=0; j<attr_patterns.size(); j++){
				std::vector<std::string> keys_coll;
				keys_coll = irdsdir.find_attr(ent_list[i], attr_patterns[j]);
				if(keys_coll.size()>0){
					fnd_list.push_back(ent_list[i]);
				}
			}
		}else{
			for(unsigned int j=0; j<attr_patterns.size(); j++){
				std::vector<std::string> keys_data;
				keys_data = irdfile.find_attr(ent_list[i], attr_patterns[j]);
				if(keys_data.size()>0){
					fnd_list.push_back(ent_list[i]);
				}
			}
		}
	}

	list_tmp.insert(list_tmp.begin(), fnd_list.begin(), fnd_list.end());
	for(unsigned int i=0; i< list_tmp.size(); i++){
		list_tmp[i].set_scheme(irods_url_org.get_scheme());
//		list_tmp[i].set_host(irods_url_org.get_host());
	}
	list = list_tmp;

}

///////////////////////////////////////////////////////////////////////////////
}   // namespace


