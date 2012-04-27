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

#include "rns_replica_adaptor_logicaldirectory.hpp"

namespace rns_replica_adaptor
{
///////////////////////////////////////////////////////////////////////////////
// namespace_directory functions
///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_change_dir(saga::impl::void_t&, saga::url name)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_change_dir()");
	check_state();

//    if (!is_directory_exists(to_dir_str)) {
//      SAGA_ADAPTOR_THROW ("no such directory.", saga::DoesNotExist);
//    }

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...

	std::string rns_path;
	boost::filesystem::path i_path(name.get_path());
	if (i_path.has_root_path()){
		rns_path = name.get_path();
	}
	else {
		rns_path = rns_url_org.get_path() + "/" + name.get_path();
	}

	instance_data   instanceData (this);
	instanceData->location_.set_path(rns_path);
    set_path_str(instanceData->location_);

}
///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_list(std::vector<saga::url>& list,
                                           std::string pattern, int flags)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_list()");
	check_state();

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
	std::string rns_path = rns_url_org.get_path();

	std::vector <std::string> results;
	std::vector <saga::url>   list_tmp;

	try {
		results  = rns_ldir.list(rns_path, pattern, flags);
		list_tmp.insert(list_tmp.begin(), results.begin(), results.end());
		list = list_tmp;
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_exists(bool& exists, saga::url url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_exists()");
	check_state();

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
	std::string rns_path = rns_url_org.get_path();

	std::string check_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		check_path = url.get_path();
	}
	else {
		check_path = rns_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the check_path
	if (check_path.substr(check_path.size()-1,1) == "/"){
		check_path.erase(check_path.size()-1);
//		std::cout << "check_path.erased:" << check_path << std::endl;
	}

	try {
		exists  = rns_ldir.exists(check_path);
//		std::cout << "check_path:" << check_path << std::endl;
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_dir(bool& is_dir, saga::url url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_dir()");
	check_state();

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
	std::string rns_path = rns_url_org.get_path();

	std::string check_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		check_path = url.get_path();
	}
	else {
		check_path = rns_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the check_path
	if (check_path.substr(check_path.size()-1,1) == "/"){
		check_path.erase(check_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = rns_ldir.exists(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (!exists){
		SAGA_ADAPTOR_THROW ("The specified entry does not exist. ", saga::DoesNotExist);
	}

	try {
		is_dir  = rns_ldir.is_dir(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_entry(bool& is_entry, saga::url url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);

	SAGA_LOG_CRITICAL( "call namespace, dir, sync_is_entry()");

	SAGA_LOG_DEBUG("sync_is_entry()");
	check_state();

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
	std::string rns_path = rns_url_org.get_path();

	std::string check_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		check_path = url.get_path();
	}
	else {
		check_path = rns_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the check_path
	if (check_path.substr(check_path.size()-1,1) == "/"){
		check_path.erase(check_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = rns_ldir.exists(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (!exists){
		SAGA_ADAPTOR_THROW ("The specified entry does not exist. ", saga::DoesNotExist);
	}

	try {
		is_entry  = rns_ldir.is_entry(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_link(bool& is_link, saga::url url)
{
    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
//	SAGA_LOG_DEBUG("sync_is_link()");
//	check_state();
//
//	saga::url rns_url_org;
//	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
//	std::string rns_path = rns_url_org.get_path();
//
//	std::string check_path;
//	boost::filesystem::path i_path(url.get_path());
//	if (i_path.has_root_path()){
//		check_path = url.get_path();
//	}
//	else {
//		check_path = rns_url_org.get_path() + "/" + url.get_path();
//	}
//
//	// to get rid of the / at the end the check_path
//	if (check_path.substr(check_path.size()-1,1) == "/"){
//		check_path.erase(check_path.size()-1);
//	}
//
//	bool exists = false;
//	try {
//		exists  = rns_ldir.exists(check_path);
//	}
//	catch (boost::system::system_error const& e) {
//		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
//	}
//
//	if (!exists){
//		SAGA_ADAPTOR_THROW ("The specified entry does not exist. ", saga::DoesNotExist);
//	}
//
//	try {
//		is_link  = rns_ldir.is_link(check_path);
//	}
//	catch (boost::system::system_error const& e) {
//		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
//	}
}


///////////////////////////////////////////////////////////////////////////////
//
//void logical_directory_cpi_impl::sync_get_num_entries(std::size_t& num)
//{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
////	SAGA_LOG_DEBUG("sync_get_num_entries()");
////	check_state();
////
////	saga::url rns_url_org;
////	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
////	std::string rns_path = rns_url_org.get_path();
////
////	try {
////		num  = rns_ldir.get_num_entries(rns_path);
////	}
////	catch (boost::system::system_error const& e) {
////		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
////	}
//}

///////////////////////////////////////////////////////////////////////////////
//
//void logical_directory_cpi_impl::sync_get_entry(saga::url& ret, std::size_t entry)
//{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
////	SAGA_LOG_DEBUG("sync_get_entry()");
////	check_state();
////
////	saga::url rns_url_org;
////	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
////	std::string rns_path = rns_url_org.get_path();
////
////	try {
////		ret  = rns_ldir.get_entry(rns_path, entry);
////	}
////	catch (boost::system::system_error const& e) {
////		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
////	}
//}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_remove(saga::impl::void_t&, saga::url url,
    int flags)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_remove(src, dst, flags)");
	check_state();

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
	std::string rns_path = rns_url_org.get_path();

	std::string rm_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		rm_path = url.get_path();
	}
	else {
		rm_path = rns_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the rm_path
	if (rm_path.substr(rm_path.size()-1,1) == "/"){
		rm_path.erase(rm_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = rns_ldir.exists(rm_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (!exists){
		SAGA_ADAPTOR_THROW ("The specified source entry does not exist. ", saga::DoesNotExist);
	}

	bool isdir = false;
	try {
		isdir  = rns_ldir.is_dir(rm_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (isdir){
		try {
//			std::cout << "remove dir:" << rm_path << std::endl;
			rns_ldir.remove(rm_path, flags);
		}
		catch (boost::system::system_error const& e) {
			SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
		}
	}
	else{
		try {
//			std::cout << "remove entry:" << rm_path << std::endl;
			rns_lfile.remove(rm_path, flags);
		}
		catch (boost::system::system_error const& e) {
			SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
		}
	}


}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_make_dir(saga::impl::void_t&, saga::url url,
    int flags)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! ", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_make_dir()");
	check_state();

	saga::url rns_url_org;
	this->sync_get_cwd(rns_url_org); // to get rid of the /. at the end...
	std::string rns_path = rns_url_org.get_path();

	std::string mkdir_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		mkdir_path = url.get_path();
	}
	else {
		mkdir_path = rns_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the mkdir_path
	if (mkdir_path.substr(mkdir_path.size()-1,1) == "/"){
		mkdir_path.erase(mkdir_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = rns_ldir.exists(mkdir_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (exists){
		SAGA_ADAPTOR_THROW ("The specified target already exists. ", saga::AlreadyExists);
	}


	try {
//		std::cout<< "mkdir_path    :" << mkdir_path << std::endl;
		rns_ldir.make_dir(mkdir_path, flags);
//		rns_ldir.test();
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}


///////////////////////////////////////////////////////////////////////////////

}   // namespace


