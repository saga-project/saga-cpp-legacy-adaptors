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
#include <saga/saga/filesystem.hpp>

#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

#include "globus_rls_replica_adaptor_connection.hpp"
#include "globus_rls_replica_adaptor_logicalfile.hpp"

namespace globus_rls_replica_adaptor
{

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_get_url(saga::url& url)
{
	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	saga::url lfn_url(instanceData->location_.get_url()); 
    
    this->check_if_open ("logical_file_cpi_impl::sync_get_url", instanceData->location_);
	
	THROW_IF_INVALID(lfn_url.get_path())
    url = lfn_url;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_get_cwd(saga::url& url)
{
	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	saga::url lfn_url(instanceData->location_.get_url());
    
    this->check_if_open ("logical_file_cpi_impl::sync_get_cwd", instanceData->location_);
	
	THROW_IF_INVALID(lfn_url.get_path())
	// we don't have the notion of directories in RLS - cwd is always "/"
	lfn_url.set_path("");
	url = lfn_url;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_get_name(saga::url& url)
{
	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	saga::url lfn_url(instanceData->location_.get_url());
    
    this->check_if_open ("logical_file_cpi_impl::sync_get_name", instanceData->location_);
	
	THROW_IF_INVALID(lfn_url.get_path())
	url = lfn_url.get_path();
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_is_dir(bool& ret)
{
	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	saga::url lfn_url(instanceData->location_);
    
    this->check_if_open ("logical_file_cpi_impl::sync_is_dir", instanceData->location_);
	
	THROW_IF_INVALID(lfn_url.get_path())
	// we don't have the notion of directories in RLS - always return false
	ret = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_is_entry(bool& ret)
{
	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	saga::url lfn_url(instanceData->location_);
    
    this->check_if_open ("logical_file_cpi_impl::sync_is_entry", instanceData->location_);
	
	THROW_IF_INVALID(lfn_url.get_path())
	ret = true;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_is_link(bool& ret)
{
	adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	saga::url lfn_url(instanceData->location_);
    
    this->check_if_open ("logical_file_cpi_impl::sync_is_link", instanceData->location_);
	
	THROW_IF_INVALID(lfn_url.get_path())
	// we don't have the notion of links in RLS - always return false
	ret = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_remove(saga::impl::void_t&, int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented!", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_file_cpi_impl::sync_close(saga::impl::void_t&, double)
{
    is_open_ = false;
}
    
}
