//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 
#include <map>
#include <vector>

#include <boost/tokenizer.hpp>

#include <saga/saga/util.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/filesystem.hpp>

#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

#include "globus_rls_replica_adaptor_logicaldirectory.hpp"
#include "../loader/globus_global_loader.hpp"

namespace globus_rls_replica_adaptor
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
	saga::url lfn_url(instanceData->location_);
    
	// we support only any:// and lfn:// schemes FIXME: what about LRC/RLI ?!?
	std::string scheme(instanceData->location_.get_scheme());
    std::string host(instanceData->location_.get_host());

	if (scheme != "lfn" && 
		scheme != GLOBUS_RLS_URL_SCHEME && scheme != GLOBUS_RLS_URL_SCHEME_NOAUTH)
	{
		SAGA_OSSTREAM strm;
		strm << "Could not open logical directory [" << instanceData->location_ << "]. " 
        << "Supported URL schemes are: lfn://";
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined);
	}
	
    // and we always fall back to 'localhost' if the hostname is empty
    if( host.empty() )
    {
        instanceData->location_.set_host("localhost"); 
    }
	
    
    // This is a trivial directory implementation. In this stage, we only 
    // accept the root path "/" as a valid directory since RLS doesn't 
    // support the concept of a directory. Sad bad true :-( 
    
    std::string path = lfn_url.get_path();
    if( path != "/" && path != "//" )
    {
 		SAGA_OSSTREAM strm;
		strm << "Could not open logical directory [" << instanceData->location_ << "]. " 
        << "Only \"/\" and \"//\" are valid directories with RLS :(";
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);       
    }
    
    // If we've made it here, it should be safe to load
    // the GRAM modules now. The loader employs a sigleton mechanism,
    // so ut doesn't matter if we call this method multiple times.
    globus_module_loader::globus_init ();
    
	// try to create/retreive a connection handle for the given host 
	try {
		RLSConnection * RLSHandle = 
        adaptorData->getConnectionHandle(instanceData->location_);
        RLSHandle = NULL; // supress compiler warnings
	}
	catch(globus_rls_replica_adaptor::exception const & e)
	{
		SAGA_OSSTREAM strm;
		strm << "Could not open logical directory [" << instanceData->location_ << "]. " 
        << e.RLSErrorText();
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
	}
    
    is_open_ = true;
    
    /*
	saga::replica::flags mode = 
    (saga::replica::flags)instanceData->mode_;
    
	if (((mode & saga::replica::Create) || 
		 (mode & saga::replica::CreateParents)) && 
        (mode & saga::replica::Exclusive)) 
	{
		// FIXME: handle modes...
	}
	
	if ((mode & saga::replica::Create) || 
		(mode & saga::replica::CreateParents))
	{
		// FIXME: Create replica entry if the create flag is given...
	}
	
    
	if(!exists) 
	{
		SAGA_OSSTREAM strm;
		strm << "Could not open logical file [" << instanceData->location_ << "]. " 
		<< "The file doesn't exist and the 'Create' flag is not set!";
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
	} */
}

///////////////////////////////////////////////////////////////////////////////
//  destructor
logical_directory_cpi_impl::~logical_directory_cpi_impl (void)
{
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_exists(bool& ret, std::string key)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_readonly(bool& ret, std::string key)
{
    instance_data data (this);
    ret = !(data->mode_ & saga::replica::Read);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_writable(bool& ret, std::string key)
{
    instance_data data (this);
    ret = data->mode_ & saga::replica::Write;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::check_permissions(saga::replica::flags flags,
    char const* name, std::string const& lfn)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_vector(bool& ret, std::string key)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_attribute_is_extended(bool& ret, std::string key)
{
    instance_data data (this);
    ret = data->mode_ & saga::replica::Write;
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_attribute(std::string& ret, std::string key)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_vector_attribute(std::vector<std::string>& ret, 
    std::string key)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_set_attribute(saga::impl::void_t&, std::string key, 
    std::string val)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_set_vector_attribute(saga::impl::void_t&, std::string key, 
    std::vector<std::string> val)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_remove_attribute(saga::impl::void_t&, std::string key)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_list_attributes(std::vector<std::string>& keys)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_find_attributes(std::vector<std::string>& keys, 
    std::string pattern)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
// namespace_entry functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_url(saga::url& url)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_cwd(saga::url& url)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_name(saga::url& url)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_dir(bool& ret)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_entry(bool& ret)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_link(bool& ret)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_remove(saga::impl::void_t&, int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_close(saga::impl::void_t&, double)
{
    is_open_ = false;
}


///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_list(std::vector<saga::url>& ret, 
                                           std::string pattern, int flags)
{
    adaptor_data_t  adaptorData(this);
	instance_data   instanceData (this);
	saga::url lfn_url(instanceData->location_);
	
    // retrieve all entries if
    // the pattern is empty otherwise,
    // RLS should understand UNIX wildcards...
    if(pattern.size() == 0)  pattern = "*";
    
	try {
		RLSConnection * RLSHandle = 
        adaptorData->getConnectionHandle(instanceData->location_);
		ret = RLSHandle->LFNList(pattern);
	}
	catch(globus_rls_replica_adaptor::exception const & e)
	{
		SAGA_OSSTREAM strm;
		strm << "Could not list entries for logical directory [" << 
		instanceData->location_ << "]. " << e.RLSErrorText();
		SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_find(std::vector<saga::url>& ret, 
    std::string pattern, int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_exists(bool& ret, saga::url entry)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_dir(bool& ret, saga::url entry)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_entry(bool& ret, saga::url entry)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_link(bool& ret, saga::url)
{
    ret = false;    // we don't support links
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_num_entries(std::size_t& num_entries)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_get_entry(saga::url& entry, std::size_t num)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_remove(saga::impl::void_t&, saga::url entry, 
    int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open(saga::name_space::entry& ret, 
    saga::url entry, int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open_dir(saga::name_space::directory& ret, 
    saga::url entry, int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_change_dir(saga::impl::void_t&, saga::url dir)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_make_dir(saga::impl::void_t&, saga::url dir, 
    int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_is_file(bool& ret, saga::url name)
{
    sync_is_entry(ret, name);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open(saga::replica::logical_file& ret, 
    saga::url entry, int flags)
{
    sync_open((saga::name_space::entry&)ret, entry, flags);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_open_dir(saga::replica::logical_directory& ret, 
    saga::url entry, int flags)
{
    sync_open_dir((saga::name_space::directory&)ret, entry, flags);
}

///////////////////////////////////////////////////////////////////////////////
//
void logical_directory_cpi_impl::sync_find(std::vector<saga::url>& ret, 
    std::string pattern, std::vector<std::string> patterns, int flags)
{
    SAGA_ADAPTOR_THROW ("Not implemented! (Globus RLS doesn't have the concept of directories)", saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
}   // namespace 


