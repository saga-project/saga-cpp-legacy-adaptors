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

#include "curl_file_adaptor_dir.hpp"
#include "curl_file_adaptor_connection.hpp"

using namespace curl_file_adaptor;

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_change_dir (saga::impl::void_t &, saga::url new_dir)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url url = merge_urls(InstanceData->location_.get_url(), new_dir);
    
    GridFTPConnection * ConnectionHandle = 
		AdaptorData->getConnectionHandleForURL(url);
    
    bool is_dir = false;
    
    try 
    {
        is_dir = ConnectionHandle->is_dir( url.get_url() );
    }
    catch( exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        std::string e_text("Could not change directory. " + ep.error_text);
        SAGA_ADAPTOR_THROW(e_text, ep.saga_error);
    }
    
    if( !is_dir )
    {
		SAGA_OSSTREAM strm;
        strm << "Could not change to directory [" << InstanceData->location_ << "]. " 
        << "URL doesn't point to a directory." ;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }
    
    InstanceData->location_ = url;
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_list (std::vector <saga::url> & list, 
							  std::string pattern, int flags)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(InstanceData->location_);
    
    try 
    {
        list = ConnectionHandle->get_directory_entries( InstanceData->location_.get_url() );
    }
    catch ( curl_file_adaptor::exception const & e )
    {
		error_package ep = curl_file_adaptor
			::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not list entries for [" << InstanceData->location_ << "]. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_find (std::vector <saga::url> & list, 
							  std::string entry, int flags )
{
	// FIXME: implement
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_exists (bool & exists, saga::url url)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), url);
        
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL( u.get_url() );
    
    try 
    {
        exists = ConnectionHandle->exist( u.get_url() );
    }
    catch ( curl_file_adaptor::exception const & e )
    {
		error_package ep = curl_file_adaptor
		::error_default_redirect(e, InstanceData->location_.get_url());
		SAGA_OSSTREAM strm;
        strm << "Could not check if [" << u.get_url() << "] exists. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_is_dir (bool & is_dir, saga::url url)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), url);  
    
	try 
    {
		GridFTPConnection * ConnectionHandle = 
		AdaptorData->getConnectionHandleForURL(u);    
    
        is_dir = ConnectionHandle->is_dir(u.get_url());
    } 
    catch( curl_file_adaptor::exception const &e)
    {
		error_package ep = curl_file_adaptor
		::error_default_redirect(e, InstanceData->location_.get_url());

		SAGA_OSSTREAM strm;
        strm << "Could not check if [" << u.get_url() << "] is a directory. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_is_entry (bool & is_entry, saga::url url)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), url);  
    	
	bool is_dir = true;
	bool is_link = true;
		
	try 
    {
		GridFTPConnection * ConnectionHandle = 
		AdaptorData->getConnectionHandleForURL(u);    
    
        is_dir  = ConnectionHandle->is_dir(u.get_url());
		is_link = ConnectionHandle->is_symlink(u.get_url());
    } 
    catch( curl_file_adaptor::exception const &e)
    {
		error_package ep = curl_file_adaptor
		::error_default_redirect(e, InstanceData->location_.get_url());

		SAGA_OSSTREAM strm;
        strm << "Could not check if [" << u.get_url() << "] is an entry. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
	
	if( !is_dir && !is_link ) is_entry = true;
	else is_entry = false;
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_is_link (bool & is_link, saga::url url)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), url);  

	try {
		GridFTPConnection * ConnectionHandle = 
			AdaptorData->getConnectionHandleForURL(u);  
  
        is_link = ConnectionHandle->is_symlink(u.get_url());
    } 
    catch( curl_file_adaptor::exception const &e)
    {
		error_package ep = curl_file_adaptor
		::error_default_redirect(e, InstanceData->location_.get_url());
		
		SAGA_OSSTREAM strm;
        strm << "Could not check if [" << u.get_url() << "] is a (sym-)link. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_read_link (saga::url & ret, saga::url source)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url url = merge_urls(InstanceData->location_.get_url(), source);  
 
	GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(url);    
    
    try 
    {
        ret = ConnectionHandle->read_symlink(url.get_url());
    } 
    catch( curl_file_adaptor::exception const &e)
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, url.get_url());
        
		SAGA_OSSTREAM strm;
        strm << "Could not read (sym-)link [" << url << "]. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_get_num_entries (std::size_t & num)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(InstanceData->location_);  
    
    try 
    {
        num = ConnectionHandle->
        get_directory_entries_count(InstanceData->location_.get_url());
    }
    catch ( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
		
		SAGA_OSSTREAM strm;
        strm << "Could not get number of entries for [" << InstanceData->location_ << "]. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_get_entry (saga::url & ret, std::size_t entry )
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(InstanceData->location_);    
    
    std::vector<saga::url> directory_list;
    
    try 
    {
        directory_list = ConnectionHandle->
        get_directory_entries( InstanceData->location_.get_url() );
    }
    catch ( curl_file_adaptor::exception const & e )
    {        
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
		
		SAGA_OSSTREAM strm;
        strm << "Could not retrieve entry [" << entry << "]. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
    
    try 
    {
        ret = directory_list.at(entry);
    }
    catch( std::exception const &e)
    {
        SAGA_ADAPTOR_THROW ("Could not retrieve entry. Array index out of bounds. "
                            , saga::DoesNotExist);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_link (saga::impl::void_t & ret, saga::url source, 
							  saga::url url, int flags)
{
	directory_instance_data_t InstanceData(this);
	
	SAGA_OSSTREAM strm;
	strm << "Could not create (sym-)link for [" << source << "] - " 
    << "Not supported by GridFTP.";
	SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::NotImplemented);
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_copy (saga::impl::void_t & ret, saga::url src, 
							  saga::url dst, int flags)
{
    // FIXME: doesn't work with directories yet...
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData (this);
    
    saga::url u_src = merge_urls(InstanceData->location_.get_url(), src);
    saga::url u_dst = merge_urls(InstanceData->location_.get_url(), dst);
    
    try {
        
        GridFTPConnection * ConnectionHandle = 
        AdaptorData->getConnectionHandleForURL(InstanceData->location_);
        
    	if(ConnectionHandle->exist(u_dst.get_url())) {
			if(ConnectionHandle->is_dir(u_dst.get_url())) {
				std::string url_path = u_dst.get_path();
				if(url_path.rfind("/") != url_path.length()-1) url_path.append("/");
				saga::url this_name; this->sync_get_name(this_name);
				url_path.append(this_name.get_path());
				u_dst.set_path(url_path);
            }
		}
        ConnectionHandle->copy_url(u_src.get_url(), u_dst.get_url());
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, u_src.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not copy [" << u_src << " -> " << u_dst
        << "]. " << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);  
    }
    
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_move (saga::impl::void_t & ret, saga::url src, 
							  saga::url dest, int flags)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData (this);
    
    saga::url u_src = merge_urls(InstanceData->location_.get_url(), src);
    saga::url u_dst = merge_urls(InstanceData->location_.get_url(), dest);
    
    try {
        sync_copy(ret, u_src, u_dst, flags);
    } 
    catch(saga::exception const & e)
    {
        SAGA_OSSTREAM strm;
        strm << "Could not move [" << u_src << " -> " << dest
        << "]. Copying source file failed!";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.get_error());
    }
    try {
        sync_remove(ret, u_src, flags);
    } 
    catch(saga::exception const & e)
    {
        SAGA_OSSTREAM strm;
        strm << "Could not move [" << u_src << " -> " << dest
        << "]. Removing source file failed!";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.get_error());
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_remove (saga::impl::void_t & ret, saga::url url, int flags)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), url);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(u);
    
    try 
    {
        if( ConnectionHandle->is_dir( u.get_url() ) )
        {
            ConnectionHandle->remove_directory( u.get_url() );
        }
        else
        {
            ConnectionHandle->remove_file( u.get_url() );
        }
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, u.get_url());
    
        SAGA_OSSTREAM strm;
        strm << "Could not remove [" << u << "]. " << ep.error_text;        
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_copy_wildcard (saga::impl::void_t & ret, std::string source, 
									   saga::url dest, int flags)
{
	//FIXME - no wildcards implemented...
	//not sure how to do that on top of Globus
	sync_copy(ret, saga::url(source), saga::url(dest), flags);
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_link_wildcard (saga::impl::void_t & ret, std::string source, 
									   saga::url dest, int flags)
{
	//FIXME - no wildcards implemented...
	//not sure how to do that on top of Globus
	sync_link(ret, saga::url(source), saga::url(dest), flags);
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_move_wildcard (saga::impl::void_t & ret, std::string source, 
									   saga::url dest, int flags)
{
	//FIXME - no wildcards implemented...
	//not sure how to do that on top of Globus
	sync_move(ret, saga::url(source), saga::url(dest), flags);
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_remove_wildcard (saga::impl::void_t & ret, std::string url, 
										 int flags)
{
	//FIXME - no wildcards implemented...
	//not sure how to do that on top of Globus
	sync_remove(ret, saga::url(url), flags);
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_make_dir (saga::impl::void_t & ret, saga::url url, int flags)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), url);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(u);
    
    try
    {
        ConnectionHandle->make_directory( u.get_url() );
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, u.get_url() );
		
		SAGA_OSSTREAM strm;
        strm << "Could not make directory [" << u.get_url() << "]. " 
			 << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }   
}
