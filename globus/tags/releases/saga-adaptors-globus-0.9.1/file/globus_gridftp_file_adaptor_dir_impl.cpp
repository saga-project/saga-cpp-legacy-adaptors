//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/exception.hpp>
#include <saga/saga/url.hpp>
#include <saga/impl/config.hpp>

#include "globus_gridftp_file_adaptor_dir.hpp"
#include "globus_gridftp_file_adaptor_file.hpp"
#include "globus_gridftp_file_adaptor_connection.hpp"
#include "../shared/globus_gsi_cert_utils.hpp"

using namespace globus_gridftp_file_adaptor;

///////////////////////////////////////////////////////////////////////////////
//
dir_cpi_impl::dir_cpi_impl (proxy                * p, 
                            cpi_info       const & info,
                            saga::ini::ini const & glob_ini,
                            saga::ini::ini const & adap_ini,
                            boost::shared_ptr<saga::adaptor> adaptor)

: directory_cpi (p, info, adaptor, cpi::Noflags)
{
    adaptor_data_t adata(this);
    instance_data idata(this);
           
	  saga::url location(idata->location_);
    std::string host(location.get_host());
    std::string scheme(location.get_scheme());
	
    // check if we can handle url scheme
    if (scheme != "file" && scheme != "gridftp" && scheme != "gsiftp")
    {
       SAGA_OSSTREAM strm;
       strm << "Could not initialize file object for [" << idata->location_ << "]. " 
            << "Only griftp:// and gsiftp:// schemes are supported.";
       SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined); 
    }
    
    if (scheme == "file") {
        is_open_ = true; // otherwise check in copy() will fail
        return; 
    }
    else
    {
        if (host.empty())
        {
            SAGA_OSSTREAM strm;
            strm << "Could not initialize file object for [" << idata->location_ << "]. " 
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
        strm << "Could not initialize directory object for " << idata->location_ << ". "
             << "No valid and/or usable x.509 context could be found:\n";
        for(unsigned int i=0; i<context_error_list.size(); ++i) {
          strm << "    - " << context_error_list[i] << "\n";
        }
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm),
                           saga::AuthorizationFailed);
    }
    
    GridFTPConnection * ConnectionHandle = 
    adata->getConnectionHandleForURL(saga::url(idata->location_.get_url()));
	
    // check if file exists AND is a dir (not a file)
    bool exists  = true;
    bool is_dir  = false;
    
    try 
    {
        is_dir = ConnectionHandle->is_dir(idata->location_.get_url());
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        if( e.get_error() == DoesNotExist ) {
            exists = false;
        }    
        else {
            error_package ep = globus_gridftp_file_adaptor
            ::error_default_redirect(e, idata->location_.get_url());
            std::string e_text("Could not opend directory. " + ep.error_text);
            SAGA_ADAPTOR_THROW(e_text, ep.saga_error);
        }
    }

    // check for openmode //
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)idata->mode_;
    
    if(exists)
    {
        if(!is_dir)
        {
            SAGA_ADAPTOR_THROW ("Could not open directory. URL doesn't point to a directory: " +
                                idata->location_.get_url(), saga::BadParameter);
        }
        else
        {
            if((OpenMode & saga::filesystem::Create) && (OpenMode & saga::filesystem::Exclusive))
            {
                SAGA_ADAPTOR_THROW ("Could not open directory with 'Exclusive' flag set. The directory already exists: " +
                                    idata->location_.get_url(), saga::AlreadyExists);
            }
        }
    }
    else // !exists
    {
        if(!(OpenMode & saga::filesystem::Create))
        {
            SAGA_ADAPTOR_THROW ("Could not open directory. The directory doesn't exist and 'Create' flag is not set: " +
                                idata->location_.get_url(), saga::DoesNotExist);
        }
        else
        {
            try
            {
                ConnectionHandle->make_directory( idata->location_.get_url() );
            }
            catch( globus_gridftp_file_adaptor::exception const & e )
            {
                error_package ep = globus_gridftp_file_adaptor
                ::error_default_redirect(e, idata->location_.get_url());
                std::string e_text("Could not opend directory. " + ep.error_text);
                SAGA_ADAPTOR_THROW(e_text, ep.saga_error);
            }  
        }
        
    }
    
    is_open_ = true;
}

///////////////////////////////////////////////////////////////////////////////
//
dir_cpi_impl::~dir_cpi_impl (void)
{
}

void dir_cpi_impl::sync_get_size (saga::off_t& size_out, saga::url name, int flags)
{
    adaptor_data_t adata(this);
    instance_data idata(this);
    
    this->check_if_open ("dir_cpi_impl::sync_get_size", idata->location_);
    
    saga::url u = merge_urls(idata->location_.get_url(), name);
    
    GridFTPConnection * ConnectionHandle = 
    adata->getConnectionHandleForURL(u);  
    
    try 
    {
        size_out = ConnectionHandle->get_size(u.get_url());
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        error_package ep = globus_gridftp_file_adaptor
        ::error_default_redirect(e, u.get_url());
        
        SAGA_OSSTREAM strm;
        strm << "Could not determine size for [" << u << "]. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    } 
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_open(saga::filesystem::file  & new_file_instance, 
							 saga::url name_to_open, int openmode)
{
    instance_data idata(this);
    
    this->check_if_open ("dir_cpi_impl::sync_open", idata->location_);
    
    saga::url url = merge_urls(idata->location_.get_url(), name_to_open);
    
    new_file_instance = saga::filesystem::file (this->get_proxy()->get_session(), 
                                                url.get_url(), 
                                                openmode);  
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_open_dir(saga::filesystem::directory & new_dir_instance, 
                                 saga::url name_to_open, int openmode)
{
    instance_data idata(this);
    
    this->check_if_open ("dir_cpi_impl::sync_open_dir", idata->location_);
        
    saga::url url = merge_urls(idata->location_.get_url(), name_to_open);
    
    new_dir_instance = saga::filesystem::directory (this->get_proxy()->get_session(), 
                                                    url.get_url(), 
                                                    openmode);
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_is_file(bool & is_file, saga::url name)
{
    adaptor_data_t adata(this);
    instance_data idata(this);
    
    this->check_if_open ("dir_cpi_impl::sync_is_file", idata->location_);
    
    saga::url u = merge_urls(idata->location_.get_url(), name);
    
	try 
    {
		GridFTPConnection * ConnectionHandle = 
		adata->getConnectionHandleForURL(u);    
        
        is_file  = ConnectionHandle->is_file(u.get_url());
    } 
    catch( globus_gridftp_file_adaptor::exception const &e)
    {
		error_package ep = globus_gridftp_file_adaptor
		::error_default_redirect(e, idata->location_.get_url());
        
		SAGA_OSSTREAM strm;
        strm << "Could not check if [" << u.get_url() << "] is an entry. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }
}

