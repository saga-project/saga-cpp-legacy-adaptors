//  Copyright (c) 2006-2008 Ole Weidner (oweidner@cct.lsu.edu)
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

#include "globus_gridftp_file_adaptor_dir.hpp"
#include "globus_gridftp_file_adaptor_connection.hpp"

using namespace globus_gridftp_file_adaptor;


///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_permissions_allow (saga::impl::void_t & ret, 
                                           saga::url tgt, 
                                           std::string id, int perm, int flags)
{
    SAGA_OSSTREAM strm;
    strm << "Altering permissions is not supported by the GridFTP protocol.";
    SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING(strm), saga::NotImplemented);
    
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_permissions_deny  (saga::impl::void_t & ret, 
                                           saga::url tgt, 
                                           std::string id, int perm, int flags)
{
    SAGA_OSSTREAM strm;
    strm << "Altering permissions is not supported by the GridFTP protocol.";
    SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING(strm), saga::NotImplemented);
    
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_permissions_check (bool & ret, std::string id, int perm)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    this->check_if_open ("dir_cpi_impl::sync_permissions_check", InstanceData->location_);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(saga::url(InstanceData->location_.get_url()));  
    
    try 
    {
        ret = ConnectionHandle->has_permission_for(InstanceData->location_.get_url(), perm);
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        error_package ep = globus_gridftp_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not determine permissions for [" << InstanceData->location_ << "]. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    }   
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_get_owner(std::string& out)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    this->check_if_open ("dir_cpi_impl::sync_get_owner", InstanceData->location_);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(saga::url(InstanceData->location_.get_url()));  
    
    try 
    {
        out = ConnectionHandle->get_owner( InstanceData->location_.get_url() );
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        error_package ep = globus_gridftp_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not determine owner for [" << InstanceData->location_ << "]. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    } 
}

///////////////////////////////////////////////////////////////////////////////
//
void dir_cpi_impl::sync_get_group(std::string& out)
{
    adaptor_data_t AdaptorData(this);
    directory_instance_data_t InstanceData(this);
    
    this->check_if_open ("dir_cpi_impl::sync_get_group", InstanceData->location_);
    
    GridFTPConnection * ConnectionHandle = 
    AdaptorData->getConnectionHandleForURL(saga::url(InstanceData->location_.get_url()));  
    
    try 
    {
        out = ConnectionHandle->get_group( InstanceData->location_.get_url() );
    }
    catch( globus_gridftp_file_adaptor::exception const & e )
    {
        error_package ep = globus_gridftp_file_adaptor
        ::error_default_redirect(e, InstanceData->location_.get_url());
        SAGA_OSSTREAM strm;
        strm << "Could not determine group for [" << InstanceData->location_ << "]. " 
        << ep.error_text;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), ep.saga_error);
    } 
}

