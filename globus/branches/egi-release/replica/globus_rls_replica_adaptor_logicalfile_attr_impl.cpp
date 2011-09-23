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
    void logical_file_cpi_impl::sync_attribute_exists(bool& ret, std::string key)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_attribute_exists", instanceData->location_);
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            ret = RLSHandle->LFNAttributeExists(lfn_url.get_path(), key);
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not check for attribute ["<< key << "] for logical file ["  
            << instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_readonly(bool& ret, std::string key)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_attribute_is_readonly", instanceData->location_);
        
        THROW_IF_INVALID(lfn_url.get_path())
        ret = !(instanceData->mode_ & saga::replica::Read);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_writable(bool& ret, std::string key)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_attribute_is_writeable", instanceData->location_);
        
        THROW_IF_INVALID(lfn_url.get_path())
        ret = instanceData->mode_ & saga::replica::Write;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_attribute_is_vector(bool& ret, std::string key)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_attribute_is_vector", instanceData->location_);
        
        THROW_IF_INVALID(lfn_url.get_path())
        // RLS doesn't support vector attributes
        ret =  false;
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_get_attribute(std::string& ret, std::string key)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_get_attribute", instanceData->location_);
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            ret = RLSHandle->LFNAttributeGet(lfn_url.get_path(), key);
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not get attribute ["<< key << "] for logical file [" << 
            instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_set_attribute(saga::impl::void_t&, std::string key, 
                                                   std::string val)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_set_attribute", instanceData->location_);
        
        try {
            
            bool exists = false;
            sync_attribute_exists(exists, key);
            if(exists) {
                // modify an existing attribute
                RLSConnection * RLSHandle = 
				adaptorData->getConnectionHandle(instanceData->location_);
                RLSHandle->LFNAttributeModify(lfn_url.get_path(), key, val);
            }
            else {
                // create a new attribute
                RLSConnection * RLSHandle = 
				adaptorData->getConnectionHandle(instanceData->location_);
                RLSHandle->LFNAttributeCreate(lfn_url.get_path(), key, val);
            }
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not create/set attribute ["<< key << "] for logical file [" 
            << instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
        
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_get_vector_attribute(std::vector<std::string>& ret, 
                                                          std::string key)
    {
        // SAGA's vector attributeds can't be mapped to RLS attributes.
        SAGA_OSSTREAM strm;
        strm << "Method not implemented. Vector attributes are not supported " 
        << "by Globus RLS!";
        SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING(strm), saga::NotImplemented);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_set_vector_attribute(saga::impl::void_t&, std::string key, 
                                                          std::vector<std::string> val)
    {
        // SAGA's vector attributeds can't be mapped to RLS attributes.
        SAGA_OSSTREAM strm;
        strm << "Method not implemented. Vector attributes are not supported " 
        << "by Globus RLS!";
        SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING(strm), saga::NotImplemented);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_remove_attribute(saga::impl::void_t&, std::string key)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_remove_attribute", instanceData->location_);
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            RLSHandle->LFNAttributeRemove(lfn_url.get_path(), key);
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not remove attribute from logical file [" << 
            instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_list_attributes(std::vector<std::string>& keys)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_list_attributes", instanceData->location_);
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            keys = RLSHandle->LFNAttributeList(lfn_url.get_path());
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not retrieve attribute list for logical file [" << 
            instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_find_attributes(std::vector<std::string>& keys, 
                                                     std::string pattern)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_find_attributes", instanceData->location_);
        
        std::string kpat, vpat;
        helper::parse_find_pattern(this, pattern, kpat, vpat); // can throw BadParameter
        
        //FIXME: hardly implementable on top of RLS... 
        SAGA_ADAPTOR_THROW ("Not implemented yet!", saga::NotImplemented);
    }    
}
