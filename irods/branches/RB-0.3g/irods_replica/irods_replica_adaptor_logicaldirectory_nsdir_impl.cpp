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
// namespace_directory functions
///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_change_dir(saga::impl::void_t&, saga::url name)
{

//    SAGA_ADAPTOR_THROW ("Not implemented! sync_change_dir", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_change_dir()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...

	std::string irods_path;
	boost::filesystem::path i_path(name.get_path());
	if (i_path.has_root_path()){
		irods_path = name.get_path();
	}
	else {
		irods_path = irods_url_org.get_path() + "/" + name.get_path();
	}

	try {
//		std::cout<< "irods_path    :" << irods_path << std::endl;
		irdsdir.change_dir(irods_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

}
///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_list(std::vector<saga::url>& list,
                                           std::string pattern, int flags)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_list", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_list()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::vector <std::string> results;
	std::vector <saga::url>   list_tmp;

	try {
		results  = irdsdir.list(irods_path, pattern, flags);
		list_tmp.insert(list_tmp.begin(), results.begin(), results.end());
		for(unsigned int i=0; i< list_tmp.size(); i++){
			list_tmp[i].set_scheme(irods_url_org.get_scheme());
//			list_tmp[i].set_host(irods_url_org.get_host());
		}
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
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_exists", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_exists()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string check_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		check_path = url.get_path();
	}
	else {
		check_path = irods_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the check_path
	if (check_path.substr(check_path.size()-1,1) == "/"){
		check_path.erase(check_path.size()-1);
//		std::cout << "check_path.erased:" << check_path << std::endl;
	}

	try {
		exists  = irdsdir.exists(check_path);
                std::string str_buf = "check_path:" + check_path;
                SAGA_LOG_CRITICAL(str_buf.c_str());
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_dir(bool& is_dir, saga::url url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_dir", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_dir()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string check_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		check_path = url.get_path();
	}
	else {
		check_path = irods_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the check_path
	if (check_path.substr(check_path.size()-1,1) == "/"){
		check_path.erase(check_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = irdsdir.exists(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (!exists){
		SAGA_ADAPTOR_THROW ("The specified entry does not exist. ", saga::DoesNotExist);
	}

	try {
		is_dir  = irdsdir.is_dir(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_entry(bool& is_entry, saga::url url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_entry", saga::NotImplemented);

	SAGA_LOG_CRITICAL("call namespace, dir, sync_is_entry()");

	SAGA_LOG_DEBUG("sync_is_entry()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string check_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		check_path = url.get_path();
	}
	else {
		check_path = irods_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the check_path
	if (check_path.substr(check_path.size()-1,1) == "/"){
		check_path.erase(check_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = irdsdir.exists(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (!exists){
		SAGA_ADAPTOR_THROW ("The specified entry does not exist. ", saga::DoesNotExist);
	}

	try {
		is_entry  = irdsdir.is_entry(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_link(bool& is_link, saga::url url)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_is_link", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_is_link()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string check_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		check_path = url.get_path();
	}
	else {
		check_path = irods_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the check_path
	if (check_path.substr(check_path.size()-1,1) == "/"){
		check_path.erase(check_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = irdsdir.exists(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (!exists){
		SAGA_ADAPTOR_THROW ("The specified entry does not exist. ", saga::DoesNotExist);
	}

	try {
		is_link  = irdsdir.is_link(check_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_num_entries(std::size_t& num)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_num_entries", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_get_num_entries()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	try {
		num  = irdsdir.get_num_entries(irods_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_entry(saga::url& ret, std::size_t entry)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_get_entry", saga::NotImplemented);
	SAGA_LOG_DEBUG("sync_get_entry()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	try {
		ret  = irdsdir.get_entry(irods_path, entry);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_remove(saga::impl::void_t&, saga::url url,
    int flags)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_remove", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_remove(src, dst, flags)");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string rm_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		rm_path = url.get_path();
	}
	else {
		rm_path = irods_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the rm_path
	if (rm_path.substr(rm_path.size()-1,1) == "/"){
		rm_path.erase(rm_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = irdsdir.exists(rm_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (!exists){
		SAGA_ADAPTOR_THROW ("The specified source entry does not exist. ", saga::DoesNotExist);
	}

	try {
//		std::cout << "rm_path=" << rm_path << std::endl;
		irdsdir.remove(rm_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_make_dir(saga::impl::void_t&, saga::url url,
    int flags)
{
//    SAGA_ADAPTOR_THROW ("Not implemented! sync_make_dir", saga::NotImplemented);
	SAGA_LOG_DEBUG("dir, sync_make_dir()");
	check_state();

	saga::url irods_url_org;
	this->sync_get_cwd(irods_url_org); // to get rid of the /. at the end...
	std::string irods_path = irods_url_org.get_path();

	std::string mkdir_path;
	boost::filesystem::path i_path(url.get_path());
	if (i_path.has_root_path()){
		mkdir_path = url.get_path();
	}
	else {
		mkdir_path = irods_url_org.get_path() + "/" + url.get_path();
	}

	// to get rid of the / at the end the mkdir_path
	if (mkdir_path.substr(mkdir_path.size()-1,1) == "/"){
		mkdir_path.erase(mkdir_path.size()-1);
	}

	bool exists = false;
	try {
		exists  = irdsdir.exists(mkdir_path);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}

	if (exists){
		SAGA_ADAPTOR_THROW ("The specified target already exists. ", saga::AlreadyExists);
	}


	try {
//		std::cout<< "mkdir_path    :" << mkdir_path << std::endl;
		irdsdir.make_dir(mkdir_path, flags);
	}
	catch (boost::system::system_error const& e) {
		SAGA_ADAPTOR_THROW(e.what(), saga::NoSuccess);
	}
}


///////////////////////////////////////////////////////////////////////////////

}   // namespace


