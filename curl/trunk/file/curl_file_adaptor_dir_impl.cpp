//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/exception.hpp>
#include <saga/url.hpp>
#include <saga/impl/config.hpp>

#include "curl_file_adaptor_dir.hpp"
#include "curl_file_adaptor_file.hpp"

using namespace curl_file_adaptor;

///////////////////////////////////////////////////////////////////////////////
//
dir_cpi_impl::dir_cpi_impl (proxy                * p, 
                            cpi_info       const & info,
                            saga::ini::ini const & glob_ini,
                            saga::ini::ini const & adap_ini,
                            boost::shared_ptr<saga::adaptor> adaptor)

: directory_cpi (p, info, adaptor, cpi::Noflags)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
        
    check_url(InstanceData->location_); // can throw...
    
    GridFTPConnection * ConnectionHandle = 
        AdaptorData->getConnectionHandleForURL(saga::url(InstanceData->location_.get_url()));
    
    std::string sdas(InstanceData->location_.get_url());
    
    // check if we can handle this URL
    std::string scheme(InstanceData->location_.get_scheme());
    if (scheme != "any" && scheme != "gridftp" && scheme != "gsiftp")
    {
        SAGA_OSSTREAM strm;
        strm << "Could not open file [" << InstanceData->location_ << "]. " 
             << "Supported URL schemes are: any:// gridftp:// gsiftp://";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::IncorrectURL);
    }
    
    // check if file exists AND is a dir (not a file)
    bool exists  = true;
    bool is_dir  = false;
    
    try 
    {
        is_dir = ConnectionHandle->is_dir(InstanceData->location_.get_url());
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
                std::string e_text("Could not opend directory. " + ep.error_text);
                SAGA_ADAPTOR_THROW(e_text, ep.saga_error);
                break;
        }
    }

    // check for openmode //
    saga::filesystem::flags OpenMode = (saga::filesystem::flags)InstanceData->mode_;
    
    if(exists)
    {
        if(!is_dir)
        {
            SAGA_ADAPTOR_THROW ("Could not open directory. URL doesn't point to a directory: " +
                                InstanceData->location_.get_url(), saga::BadParameter);
        }
        else
        {
            if((OpenMode & saga::filesystem::Create) && (OpenMode & saga::filesystem::Exclusive))
            {
                SAGA_ADAPTOR_THROW ("Could not open directory with 'Exclusive' flag set. The directory already exists: " +
                                    InstanceData->location_.get_url(), saga::AlreadyExists);
            }
        }
    }
    else // !exists
    {
        if(!(OpenMode & saga::filesystem::Create))
        {
            SAGA_ADAPTOR_THROW ("Could not open directory. The directory doesn't exist and 'Create' flag is not set: " +
                                InstanceData->location_.get_url(), saga::DoesNotExist);
        }
        else
        {
            try
            {
                ConnectionHandle->make_directory( InstanceData->location_.get_url() );
            }
            catch( curl_file_adaptor::exception const & e )
            {
                error_package ep = curl_file_adaptor
                ::error_default_redirect(e, InstanceData->location_.get_url());
                std::string e_text("Could not opend directory. " + ep.error_text);
                SAGA_ADAPTOR_THROW(e_text, ep.saga_error);
            }  
        }
        
    }
}

///////////////////////////////////////////////////////////////////////////////
//
dir_cpi_impl::~dir_cpi_impl (void)
{
}

void dir_cpi_impl::sync_get_size (saga::off_t& size_out, saga::url name, int flag)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), name);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(u);  
    
    try 
    {
        size_out = ConnectionHandle->get_size(u.get_url());
    }
    catch( curl_file_adaptor::exception const & e )
    {
        error_package ep = curl_file_adaptor
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
    directory_instance_data_t InstanceData(this);
    
    saga::url url = merge_urls(InstanceData->location_.get_url(), name_to_open);
    
    new_file_instance = saga::filesystem::file (this->get_proxy()->get_session(), 
                                                url.get_url(), 
                                                openmode);  
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_open_dir(saga::filesystem::directory & new_dir_instance, 
                                 saga::url name_to_open, int openmode)
{
    directory_instance_data_t InstanceData(this);
        
    saga::url url = merge_urls(InstanceData->location_.get_url(), name_to_open);
    
    new_dir_instance = saga::filesystem::directory (this->get_proxy()->get_session(), 
                                                    url.get_url(), 
                                                    openmode);
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_is_file(bool & is_file, std::string name, int flags)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    saga::url u = merge_urls(InstanceData->location_.get_url(), name);
    
    try 
    {
        GridFTPConnection * ConnectionHandle = 
        AdaptorData->getConnectionHandleForURL(u);    
        
        is_file  = ConnectionHandle->is_file(u.get_url());
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
}

