//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <fstream>

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <saga/saga/url.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/adaptors/task.hpp>

#include <saga/impl/config.hpp>

#include "curl_file_adaptor_file.hpp"
#include "curl_file_adaptor_connection.hpp"

using namespace curl_file_adaptor;

file_cpi_impl::~file_cpi_impl() {}

///////////////////////////////////////////////////////////////////////////////
//
file_cpi_impl::file_cpi_impl (proxy                * p, 
                              cpi_info       const & info,
                              saga::ini::ini const & glob_ini,
                              saga::ini::ini const & adap_ini,
                              boost::shared_ptr<saga::adaptor> adaptor)

: file_cpi (p, info, adaptor, cpi::Noflags)
{
    std::cerr << "cURL adaptor starting up..." << std::endl;
    
    // get storage handles //
    adaptor_data_t adaptorData(this);
    file_instance_data_t instanceData(this);
    
	// Initialize file position pointer
	instanceData->pointer_ = 0;
    
    cURLconnection cURL(instanceData->location_.get_string());
    
}
/*    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(InstanceData->location_);
    
	// check if we can handle this URL
		std::string scheme(InstanceData->location_.get_scheme());

	if (scheme == "file" || scheme == "any") return; // we need that for local file copy
		else if (scheme != "gridftp" && scheme != "gsiftp")
	{
        SAGA_OSSTREAM strm;
        strm << "Could not open file [" << InstanceData->location_ << "]. " 
             << "Supported URL schemes are: any:// gridftp:// and gsiftp://";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::IncorrectURL);
	}
	
    // check if file exists AND is a file (not a dir)
    bool exists  = false;
    bool is_file = false;
    
    try 
    {
        is_file = ConnectionHandle->is_file(InstanceData->location_.get_url());
        exists = true; // if is_file() succeeded, the entry must also exist :)
    }
    catch( curl_file_adaptor::exception const & e )
    {
        switch( e.get_error() )
        {
          case(DoesNotExist):
              exists = false;
              break;
              
          default:
              error_package ep = curl_file_adaptor
              ::error_default_redirect(e, InstanceData->location_.get_url());
              SAGA_OSSTREAM strm;
              strm << "Could not open file [" << InstanceData->location_ << "]. " 
              << ep.error_text;
              SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
        }
    }
    
    if( exists && !is_file )
    {
        SAGA_OSSTREAM strm;
        strm << "Could not open file [" << InstanceData->location_ << "]. " 
        << "The URL points to a directory.";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }
    
    // get the open mode flags
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)InstanceData->mode_;
    
    // set initial file pointer
    InstanceData->pointer_ = 0;
    
    if( !exists )
    {
        if( OpenMode & saga::filesystem::Create )
        {
            try {
                // this creates an empty file
                ConnectionHandle->write_to_file( InstanceData->location_.get_url(),"",0 );
            }
            catch( curl_file_adaptor::exception const & e )
            {
                error_package ep = curl_file_adaptor
                ::error_default_redirect(e, InstanceData->location_.get_url());
                SAGA_OSSTREAM strm;
                strm << "Could not open file [" << InstanceData->location_ << "]. " 
                << "The URL points to a directory.";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
            }  
        }
        else
        {
            SAGA_OSSTREAM strm;
            strm << "Could not open file [" << InstanceData->location_ << "]. " 
            << "The file doesn't exist and the 'Create' flag is not set!";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
        }
    }
    else
    {
        if((OpenMode & saga::filesystem::Create) && 
           (OpenMode & saga::filesystem::Exclusive))
        {
            SAGA_OSSTREAM strm;
            strm << "Could not open file [" << InstanceData->location_ << "]. " 
            << "The file already exists and the 'Exclusive' flag is set!";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
        }
        else
        {
            if( OpenMode & saga::filesystem::Truncate )
            {
                try {
                    ConnectionHandle->remove_file( InstanceData->location_.get_url() );
                    ConnectionHandle->write_to_file( InstanceData->location_.get_url(),"",0 );
                }
                catch( curl_file_adaptor::exception const & e )
                {
                    error_package ep = curl_file_adaptor
                    ::error_default_redirect(e, InstanceData->location_.get_url());
                    SAGA_OSSTREAM strm;
                    strm << "Could not open file [" << InstanceData->location_ << "]. " 
                    << ep.error_text;
                    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
                }
            }
            else if( OpenMode & saga::filesystem::Append )
            {
                try {
                    InstanceData->pointer_ = 
                    ConnectionHandle->get_size( InstanceData->location_.get_url() );
                }
                catch( curl_file_adaptor::exception const & e )
                {
                    error_package ep = curl_file_adaptor
                    ::error_default_redirect(e, InstanceData->location_.get_url());
                    SAGA_OSSTREAM strm;
                    strm << "Could not open file [" << InstanceData->location_ << "]. " 
                    << ep.error_text;
                    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//
file_cpi_impl::~file_cpi_impl (void)
{
}


///////////////////////////////////////////////////////////////////////////////
//
void file_cpi_impl::sync_get_size (saga::off_t& size_out)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData(this);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(saga::url(InstanceData->location_.get_url()));  
    
    try 
    {
        size_out = ConnectionHandle->get_size( InstanceData->location_.get_url() );
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not determine size of [" << InstanceData->location_ << "]. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    } 
}

///////////////////////////////////////////////////////////////////////////////
//
void file_cpi_impl::sync_read (saga::ssize_t & len_out,
                               saga::mutable_buffer data,
                               saga::ssize_t len_in)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData(this);
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)InstanceData->mode_;
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(saga::url(InstanceData->location_.get_url()));  
    
    // validate parameters
    if (len_in < 0)
    {
        SAGA_ADAPTOR_THROW("Could not read from file ["+ InstanceData->location_.get_url() + "]. "+
                           ". 'len_in' is negative", 
                           saga::BadParameter);
    }
    if (data.get_size() != -1 && len_in < data.get_size())
    {
        SAGA_ADAPTOR_THROW("Could not read from file ["+ InstanceData->location_.get_url() + "]. "+
                           ". The buffer is too small", 
                                saga::BadParameter);
    }
    
    if( !((OpenMode & saga::filesystem::Read) || (OpenMode & saga::filesystem::ReadWrite)) ) 
    {  
        SAGA_ADAPTOR_THROW ("Could not read from file ["+ InstanceData->location_.get_url() + "]. "+
                            ". File was not opened in 'Read' or 'ReadWrite' mode.",
                            saga::IncorrectState);    
    }
    
    // initialize buffer
    if (-1 == data.get_size())
    {
        data.set_size(len_in);    
    }
    
    // ensure, that the stream operates on bytes
    BOOST_ASSERT (sizeof (std::fstream::char_type) == sizeof (saga::char_t));
    
    // read data
    mutex_type::scoped_lock lock(mtx_);
    
    try 
    {
        len_out = ConnectionHandle->read_from_file( InstanceData->location_.get_url(),
                                                   (char *)data.get_data(),
                                                   (std::streamsize)data.get_size(),
                                                   InstanceData->pointer_ );
        InstanceData->pointer_ += len_out;
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not read from file [" << InstanceData->location_ << "]. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
void file_cpi_impl::sync_write (saga::ssize_t  & len_out, 
                                saga::const_buffer data,
                                saga::ssize_t len_in)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData(this);
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)InstanceData->mode_;
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(saga::url(InstanceData->location_.get_url()));  
    
    // validate parameters
    if (len_in < 0)
    {
        SAGA_ADAPTOR_THROW("Could not write to file [" + 
                           InstanceData->location_.get_url() + "]. " +
                           ". 'len_in' is negative", 
                           saga::BadParameter);
    }
    if( !((OpenMode & saga::filesystem::Write) || (OpenMode & saga::filesystem::ReadWrite)) ) 
    {  
        SAGA_ADAPTOR_THROW ("Could not write to file [" + 
                            InstanceData->location_.get_url() + "]. " +
                            ". File was not opened in 'Write' or 'ReadWrite' mode.",
                            saga::IncorrectState);     
    }
    
    // ensure, that the stream operates on bytes
    BOOST_ASSERT (sizeof (std::fstream::char_type) == sizeof (saga::char_t));
    
    // write data 
    mutex_type::scoped_lock lock(mtx_);
    
    int bytes_written = 0;
    
    try
    {
        bytes_written =
        ConnectionHandle->write_to_file(InstanceData->location_.get_url(),
                                        (char const*)data.get_data(),
                                        len_in,
                                        InstanceData->pointer_ );
        
        InstanceData->pointer_ += bytes_written;
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not write to file [" << InstanceData->location_ << "]. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
    
    len_out = bytes_written;
}

///////////////////////////////////////////////////////////////////////////////
//
void file_cpi_impl::sync_seek (saga::off_t & out, 
                               saga::off_t   offset, 
                               saga::filesystem::seek_mode whence)
{
    adaptor_data_t AdaptorData(this);
    file_instance_data_t InstanceData(this);
    
    saga::off_t pos = 0;
    
    switch ( (saga::filesystem::seek_mode) whence ) 
    {
        case saga::filesystem::Start:
            pos = 0;
            break;
        case saga::filesystem::Current:
            pos = InstanceData->pointer_;
            break;
        case saga::filesystem::End:
            try {
                sync_get_size(pos); // FIXME: doesn't work
            }
            catch( curl_file_adaptor::exception const & e )
        {
            error_package ep = curl_file_adaptor
            ::error_default_redirect(e, InstanceData->location_.get_url());
            SAGA_OSSTREAM strm;
            strm << "Could not seek file [" << InstanceData->location_ << "]. " 
            << ep.error_text;
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);            
        }
            break;
    }
    
    pos += offset;
    InstanceData->pointer_ = pos;
    out = InstanceData->pointer_;
}*/
