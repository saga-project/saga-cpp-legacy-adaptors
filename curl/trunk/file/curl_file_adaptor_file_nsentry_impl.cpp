//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <fstream>

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <saga/url.hpp>
#include <saga/exception.hpp>
#include <saga/adaptors/task.hpp>

#include <saga/impl/config.hpp>

#include "curl_file_adaptor_file.hpp"
#include "curl_file_adaptor_connection.hpp"

using namespace curl_file_adaptor;


void file_cpi_impl::sync_get_url  (saga::url & url)
{
    saga::url u;
    {
        instance_data data (this);
        u = data->location_;
    }
    
    // complete url
    if (u.get_scheme().empty())
        u.set_scheme("gridftp");
    
    url = u.get_url();
}

void file_cpi_impl::sync_get_cwd  (saga::url & cwd)
{
    saga::url u;
    {
        instance_data data (this);
        u = data->location_;
    }
    
    std::string u_str(u.get_url());
	std::string::size_type idx = u_str.rfind("/");
		
	if( idx != std::string::npos )
        u_str = u_str.substr(0, idx);
	    
    cwd = saga::url(u_str);
}

void file_cpi_impl::sync_get_name (saga::url & name)
{
    namespace fs = boost::filesystem;
    
    saga::url u;
    {
        instance_data data (this);
        u = data->location_;
    }
    
    fs::path path(u.get_path());
    std::string path_str(u.get_path());
    
    if( !path.has_root_path() )
        path = fs::path("/"+path_str);
    
    path_str = path.string();
    std::string::size_type idx = path_str.rfind("/");
    
    ( idx == 0 ) ? path = fs::path(path_str.substr(1, path_str.size()-1)) : 
    path = fs::path(path_str.substr(idx+1, path_str.size()-1));
    
    if( path.string().size() == 0 ) path = fs::path("/");
    
    name = path.string();
}
    
void file_cpi_impl::sync_is_dir (bool & is_dir)
{
	is_dir = false;
}

void file_cpi_impl::sync_is_entry  (bool & is_file)
{
	is_file = true;
}

void file_cpi_impl::sync_is_link   (bool & is_link)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData (this);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(InstanceData->location_);  
    
    bool test = false;
    
    try 
    {
        test = ConnectionHandle->is_symlink(InstanceData->location_.get_url());
    } 
    catch( curl_file_adaptor::exception const &e)
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not check if [" << InstanceData->location_ << "] is a (sym-)link." 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
    
    is_link = test;
}

void file_cpi_impl::sync_read_link (saga::url & target)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData (this);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(InstanceData->location_);    
    
	std::string source_entry("");
	
    try 
    {
        source_entry = ConnectionHandle->read_symlink(InstanceData->location_.get_url());
    } 
    catch( curl_file_adaptor::exception const &e)
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not read (sym-)link [" << InstanceData->location_ << "]. " 
             << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);  
    }
	
	target = merge_urls(InstanceData->location_.get_url(), source_entry);
}
    
void file_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                               saga::url dest, int flags)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData (this);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(InstanceData->location_);
    
	saga::url target = merge_urls(InstanceData->location_.get_url(), dest);
		
    try {
		if(ConnectionHandle->exist(target.get_url())) {
			if(ConnectionHandle->is_dir(target.get_url())) {
				std::string url_path = target.get_path();
				if(url_path.rfind("/") != url_path.length()-1) url_path.append("/");
				saga::url this_name; this->sync_get_name(this_name);
				url_path.append(this_name.get_path());
				target.set_path(url_path);
            }
		}
        ConnectionHandle->copy_url(InstanceData->location_.get_url(), target.get_url());
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not copy [" << InstanceData->location_ << " -> " << target
        << "]. " << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);  
    }
}

void file_cpi_impl::sync_link (saga::impl::void_t & ret,    
                               saga::url dest, 
                               int flags)
{
    file_instance_data_t InstanceData (this);

	SAGA_OSSTREAM strm;
	strm << "Could not create (sym-)link for [" << InstanceData->location_ << "] - " 
		 << "Not supported by GridFTP.";
	SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NotImplemented);
}

void file_cpi_impl::sync_move (saga::impl::void_t & ret,   
                               saga::url dest, 
                               int flags)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData (this);
    
    try {
        sync_copy(ret, dest, flags);
    } 
    catch(saga::exception const & e)
    {
        SAGA_OSSTREAM strm;
        strm << "Could not move [" << InstanceData->location_ << " -> " << dest
             << "]. Copying source file failed!";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.get_error());
    }
    try {
        sync_remove(ret, flags);
    } 
    catch(saga::exception const & e)
    {
        SAGA_OSSTREAM strm;
        strm << "Could not move [" << InstanceData->location_ << " -> " << dest
        << "]. Removing source file failed!";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.get_error());
    }
    
    /// This code tries to generate a proper URL for the moved file
    /// and sets this instance's location to the new location
    /// FIXME: is this behaviour ok - spec. doesn't say anything about it?
    saga::url target = merge_urls(InstanceData->location_.get_url(), dest);
    try {
        GridFTPConnection * ConnectionHandle = 
        AdaptorData->getConnectionHandleForURL(InstanceData->location_);
        
		if(ConnectionHandle->exist(target.get_url())) {
			if(ConnectionHandle->is_dir(target.get_url())) {
				std::string url_path = target.get_path();
				if(url_path.rfind("/") != url_path.length()-1) url_path.append("/");
				saga::url this_name; this->sync_get_name(this_name);
				url_path.append(this_name.get_path());
				target.set_path(url_path);
            }
		}
    }
    catch( curl_file_adaptor::exception const & e )
    {
        //FIXME ???
    }
    
    InstanceData->location_ = target; // FIXME: is this behaviour ok?
}

void file_cpi_impl::sync_remove (saga::impl::void_t & ret,
                                 int flags)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData (this);
        
	try {
		GridFTPConnection * ConnectionHandle = 
			AdaptorData->getConnectionHandleForURL(InstanceData->location_);  
    
		ConnectionHandle->remove_file( InstanceData->location_.get_url() );
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not remove [" << InstanceData->location_ << "]. " << ep.error_text;        
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}
	
    
void file_cpi_impl::sync_close (saga::impl::void_t & ret, double timeout)
{
    //FIXME: no idea...
}
