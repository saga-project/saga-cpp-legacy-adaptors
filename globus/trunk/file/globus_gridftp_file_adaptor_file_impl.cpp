//  Copyright (c) 2006-2009 Ole Weidner (oweidner@cct.lsu.edu)
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

#include "globus_gridftp_file_adaptor_file.hpp"
#include "globus_gridftp_file_adaptor_connection.hpp"

#include "../shared/globus_gsi_cert_utils.hpp"
#include "../loader/globus_global_loader.hpp"

using namespace globus_gridftp_file_adaptor;

///////////////////////////////////////////////////////////////////////////////
//
file_cpi_impl::file_cpi_impl (proxy                * p, 
                              cpi_info       const & info,
                              saga::ini::ini const & glob_ini,
                              saga::ini::ini const & adap_ini,
                              boost::shared_ptr<saga::adaptor> adaptor)

: file_cpi (p, info, adaptor, cpi::Noflags)
{        
    adaptor_data_t adata(this);
    instance_data data(this);
    
    // Read some stuff from the .ini file
    saga::ini::ini prefs = adap_ini.get_section ("preferences");
    
    if (prefs.has_entry("write_ftp_log")) 
    {
        std::string wfl = prefs.get_entry("write_ftp_log");
        if(wfl == "true" || wfl == "True" || wfl == "TRUE")
        {
            write_log_ = true;
        }
        else 
        {
            write_log_ = false;
        }

    }
    
    if (prefs.has_entry("logilfe_location"))
        logfile_loc_ = prefs.get_entry("logilfe_location");
    else 
        logfile_loc_ = "saga_gridftp.log";
    
    
	// Initialize file position pointer
	data->pointer_ = 0;
	  
	saga::url location(data->location_);
    std::string host(location.get_host());
    std::string scheme(location.get_scheme());

    // check if we can handle url scheme
    if (scheme != "file" && scheme != "gridftp" && scheme != "gsiftp")
    {
       SAGA_OSSTREAM strm;
       strm << "Could not initialize file object for [" << data->location_ << "]. " 
            << "Only griftp:// and gsiftp:// schemes are supported.";
       SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined); 
    }
    
    this->is_local_file_ = false;
    
    if (scheme == "file") 
    {
        this->is_local_file_ = true;
        
        // make sure file exist
        namespace fs = boost::filesystem;
        try 
        {
            std::string url = saga::url::unescape(location.get_path());
            fs::path fpath (url, fs::native);
            if ( ! fs::exists (fpath) )
            {
                SAGA_OSSTREAM strm;
                strm << "Local file doesn't exist: [" << location << "].";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined); 
            }
            else 
            {
                is_open_ = true; // otherwise check in copy() will fail
                return;
            }
        }
        catch (boost::system::system_error const& e) 
        {
            SAGA_ADAPTOR_THROW(
            location.get_string() + ": caught filesystem exception: " + e.what(),
            saga::NoSuccess);
        }
    }
    else
    {
        if (host.empty())
        {
            SAGA_OSSTREAM strm;
            strm << "Could not initialize file object for [" << data->location_ << "]. " 
            << "URL doesn't define a hostname.";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter); 
        }
    }
        
    // check if we have x.509 contexts available and if they are usable
    // with this adaptor. if no context is usable, the constructor fails with
    // an authorization failed exception.
    std::vector <saga::context> contexts = p->get_session ().list_contexts ();
    std::vector <saga::context> context_list;
    // holds a list of reasons why a context can't be used. if no context
    // can be used, the list will be appended to the exception message otherwise
    // it will be discarded. 
    std::vector <std::string> context_error_list;
    
    for (unsigned int i = 0; i < contexts.size (); i++)
    {
      globus_adaptors_shared::check_x509_globus_cert(contexts[i], 
                                                     context_list, 
                                                     context_error_list);
    } 
    
    if(context_list.size() <1) {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize file object for " << data->location_ << ". "
             << "No valid and/or usable x.509 context could be found:\n";
        for(unsigned int i=0; i<context_error_list.size(); ++i) {
          strm << "    - " << context_error_list[i] << "\n";
        }
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
                           saga::AuthorizationFailed);
    }
        
    //If we've made it here, it should be safe to load
    // the GRAM modules now. The loader employs a sigleton mechanism,
    // so ut doesn't matter if we call this method multiple times.
    globus_module_loader::globus_init ();
        
    GridFTPConnection * ConnectionHandle = 
    adata->getConnectionHandleForURL(data->location_, write_log_, logfile_loc_);
	
    // check if file exists AND is a file (not a dir)
    bool exists  = false;
    bool is_file = false;
    
    try 
    {
        is_file = ConnectionHandle->is_file(data->location_.get_url());
        exists = true; // if is_file() succeeded, the entry must also exist :)
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        if( e.get_error() == DoesNotExist) {
            exists = false;
        }
        else {
             error_package ep = globus_gridftp_file_adaptor
              ::error_default_redirect(e, data->location_.get_url());
              SAGA_OSSTREAM strm;
              strm << "Could not open file [" << data->location_ << "]. " 
              << ep.error_text;
              SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
        }
    }
    
    if( exists && !is_file )
    {
        SAGA_OSSTREAM strm;
        strm << "Could not open file [" << data->location_ << "]. " 
        << "The URL points to a directory.";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }
    
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)data->mode_;
   
    if( !exists )
    {
        if( OpenMode & saga::filesystem::Create )
        {
            try {
                // this creates an empty file
                ConnectionHandle->write_to_file( data->location_.get_url(),"",0 );
            }
            catch( globus_gridftp_file_adaptor::exception const & e )
            {
                error_package ep = globus_gridftp_file_adaptor
                ::error_default_redirect(e, data->location_.get_url());
                SAGA_OSSTREAM strm;
                strm << "Could not open file [" << data->location_ << "]. " 
                << "The URL points to a directory.";
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
            }  
        }
        else
        {
            SAGA_OSSTREAM strm;
            strm << "Could not open file [" << data->location_ << "]. " 
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
            strm << "Could not open file [" << data->location_ << "]. " 
            << "The file already exists and the 'Exclusive' flag is set!";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
        }
        else
        {
            if( OpenMode & saga::filesystem::Truncate )
            {
                try {
                    ConnectionHandle->remove_file( data->location_.get_url() );
                    ConnectionHandle->write_to_file( data->location_.get_url(),"",0 );
                }
                catch( globus_gridftp_file_adaptor::exception const & e )
                {
                    error_package ep = globus_gridftp_file_adaptor
                    ::error_default_redirect(e, data->location_.get_url());
                    SAGA_OSSTREAM strm;
                    strm << "Could not open file [" << data->location_ << "]. " 
                    << ep.error_text;
                    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
                }
            }
            else if( OpenMode & saga::filesystem::Append )
            {
                try {
                    data->pointer_ = 
                    ConnectionHandle->get_size( data->location_.get_url() );
                }
                catch( globus_gridftp_file_adaptor::exception const & e )
                {
                    error_package ep = globus_gridftp_file_adaptor
                    ::error_default_redirect(e, data->location_.get_url());
                    SAGA_OSSTREAM strm;
                    strm << "Could not open file [" << data->location_ << "]. " 
                    << ep.error_text;
                    SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
                }
            }
        }
    }
    
    is_open_ = true;
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
    adaptor_data_t adata(this);
    instance_data idata(this);
    
    this->throw_if_local(idata->location_);
    this->check_if_open ("file_cpi_impl::sync_get_size", idata->location_);
    
    GridFTPConnection * ConnectionHandle = 
    adata->getConnectionHandleForURL(saga::url(idata->location_.get_url()), write_log_, logfile_loc_);  
    
    try 
    {
        size_out = ConnectionHandle->get_size( idata->location_.get_url() );
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        error_package ep = globus_gridftp_file_adaptor
        ::error_default_redirect(e, idata->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not determine size of [" << idata->location_ << "]. " 
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
    adaptor_data_t adata(this);
    instance_data idata(this);
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)idata->mode_;
    
    this->throw_if_local(idata->location_);
    this->check_if_open ("file_cpi_impl::sync_read", idata->location_);
    
    GridFTPConnection * ConnectionHandle = 
    adata->getConnectionHandleForURL(saga::url(idata->location_.get_url()), write_log_, logfile_loc_);  
    
    // validate parameters
    if (len_in < 0)
    {
        SAGA_ADAPTOR_THROW("Could not read from file ["+ idata->location_.get_url() + "]. "+
                           ". 'len_in' is negative", 
                           saga::BadParameter);
    }
    if (data.get_size() != -1 && len_in < data.get_size())
    {
        SAGA_ADAPTOR_THROW("Could not read from file ["+ idata->location_.get_url() + "]. "+
                           ". The buffer is too small", 
                                saga::BadParameter);
    }
    
    if( !((OpenMode & saga::filesystem::Read) || (OpenMode & saga::filesystem::ReadWrite)) ) 
    {  
        SAGA_ADAPTOR_THROW ("Could not read from file ["+ idata->location_.get_url() + "]. "+
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
        len_out = ConnectionHandle->read_from_file( idata->location_.get_url(),
                                                   (char *)data.get_data(),
                                                   (std::streamsize)data.get_size(),
                                                   idata->pointer_ );
        idata->pointer_ += len_out;
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        error_package ep = globus_gridftp_file_adaptor
        ::error_default_redirect(e, idata->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not read from file [" << idata->location_ << "]. " 
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
    adaptor_data_t adata(this);
    instance_data idata(this);
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)idata->mode_;
    
    this->throw_if_local(idata->location_);
    this->check_if_open ("file_cpi_impl::sync_write", idata->location_);
    
    GridFTPConnection * ConnectionHandle = 
    adata->getConnectionHandleForURL(saga::url(idata->location_.get_url()), write_log_, logfile_loc_);  
    
    // validate parameters
    if (len_in < 0)
    {
        SAGA_ADAPTOR_THROW("Could not write to file [" + 
                           idata->location_.get_url() + "]. " +
                           ". 'len_in' is negative", 
                           saga::BadParameter);
    }
    if( !((OpenMode & saga::filesystem::Write) || (OpenMode & saga::filesystem::ReadWrite)) ) 
    {  
        SAGA_ADAPTOR_THROW ("Could not write to file [" + 
                            idata->location_.get_url() + "]. " +
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
        ConnectionHandle->write_to_file(idata->location_.get_url(),
                                        (char const*)data.get_data(),
                                        len_in,
                                        idata->pointer_ );
        
        idata->pointer_ += bytes_written;
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        error_package ep = globus_gridftp_file_adaptor
        ::error_default_redirect(e, idata->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not write to file [" << idata->location_ << "]. " 
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
    adaptor_data_t adata(this);
    instance_data idata(this);
    
    this->throw_if_local(idata->location_);
    this->check_if_open ("file_cpi_impl::seek", idata->location_);
    
    saga::off_t pos = 0;
    
    switch ( (saga::filesystem::seek_mode) whence ) 
    {
        case saga::filesystem::Start:
            pos = 0;
            break;
        case saga::filesystem::Current:
            pos = idata->pointer_;
            break;
        case saga::filesystem::End:
            try {
                sync_get_size(pos); // FIXME: doesn't work
            }
            catch( globus_gridftp_file_adaptor::exception const & e )
        {
            error_package ep = globus_gridftp_file_adaptor
            ::error_default_redirect(e, idata->location_.get_url());
            SAGA_OSSTREAM strm;
            strm << "Could not seek file [" << idata->location_ << "]. " 
            << ep.error_text;
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);            
        }
            break;
    }
    
    pos += offset;
    idata->pointer_ = pos;
    out = idata->pointer_;
}
