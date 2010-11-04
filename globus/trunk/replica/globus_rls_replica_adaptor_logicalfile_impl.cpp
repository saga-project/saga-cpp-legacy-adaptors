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

#include "../shared/globus_gsi_cert_utils.hpp"
#include "../loader/globus_global_loader.hpp"

namespace globus_rls_replica_adaptor
{
    
    void logical_file_cpi_impl::check_permissions(saga::replica::flags flags,
                                                  char const* name, std::string const& adname)
    {
        instance_data data (this);
        
        this->check_if_open ("logical_file_cpi_impl::check_permissions", data->location_);
        
        if (!(data->mode_ & flags)) {
            SAGA_OSSTREAM strm;
            strm << name << " could not access (" 
            << ((flags == saga::replica::Read) ? "read" : "write")
            << ") : " << adname;
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), 
                               saga::PermissionDenied);
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    logical_file_cpi_impl::logical_file_cpi_impl (proxy* p, cpi_info const& info,
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
        
        if (scheme != "any" && scheme != "lfn" && 
            scheme != GLOBUS_RLS_URL_SCHEME && scheme != GLOBUS_RLS_URL_SCHEME_NOAUTH)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not open logical file [" << instanceData->location_ << "]. " 
            << "Supported URL schemes are: any:// and lfn://";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined);
        }
        
        // and we always fall back to 'localhost' if the hostname is empty
        if( host.empty() )
        {
            instanceData->location_.set_host("localhost"); 
        }
        
        // If we've made it here, it should be safe to load
        // the GRAM modules now. The loader employs a sigleton mechanism,
        // so ut doesn't matter if we call this method multiple times.
        globus_module_loader::globus_init ();
        
        // try to create/retreive a connection handle for the given host 
        bool exists = false;
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            exists = RLSHandle->LFNExists(lfn_url.get_path());
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not open logical file [" << instanceData->location_ << "]. " 
            << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
        
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
        } 
        
        is_open_ = true;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    logical_file_cpi_impl::~logical_file_cpi_impl (void)
    {
        
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    // logical_file functions
    void logical_file_cpi_impl::sync_list_locations(std::vector<saga::url>& locations)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_list_locations", instanceData->location_);
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            locations = RLSHandle->LFNGetPFNList(lfn_url.get_path());
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not list locations for logical file [" << 
            instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_add_location(saga::impl::void_t&, saga::url location)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_add_location", instanceData->location_);
        
        check_permissions(saga::replica::Write, "add_location", lfn_url.get_url());
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            RLSHandle->LFNAddPFN(lfn_url.get_path(), location.get_url());
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not add location to logical file [" << 
            instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_remove_location(saga::impl::void_t&, saga::url location)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_remove_location", instanceData->location_);
        
        check_permissions(saga::replica::Write, "remove_location", lfn_url.get_url());
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            RLSHandle->LFNRemovePFN(lfn_url.get_path(), location.get_url());
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            SAGA_OSSTREAM strm;
            strm << "Could not remove location from logical file [" << 
            instanceData->location_ << "]. " << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_update_location(saga::impl::void_t& ret, 
                                                     saga::url oldlocation, saga::url newlocation)
    {
        adaptor_data_t  adaptorData(this);
        instance_data   instanceData (this);
        saga::url lfn_url(instanceData->location_);
        
        this->check_if_open ("logical_file_cpi_impl::sync_update_location", instanceData->location_);
        
        check_permissions(saga::replica::Write, "update_location", lfn_url.get_url());
        
        bool oldExists = false; // worst case - update will fail
        bool newExists = true;  //
        
        SAGA_OSSTREAM strm;
        strm << "Could not update location for logical file ["
        << instanceData->location_ << "]. ";
        
        try {
            RLSConnection * RLSHandle = 
			adaptorData->getConnectionHandle(instanceData->location_);
            oldExists = RLSHandle->LFNtoPFNMappingExists(lfn_url.get_path(), 
                                                         oldlocation.get_url());
            newExists = RLSHandle->LFNtoPFNMappingExists(lfn_url.get_path(), 
                                                         newlocation.get_url());
        }
        catch(globus_rls_replica_adaptor::exception const & e)
        {
            strm << e.RLSErrorText();
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), e.SAGAError()); 
        }
        
        if(!oldExists) {
            strm << "PFN: [" << oldlocation << "] doesn't exist!";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::DoesNotExist);
        }
        
        if(newExists) {
            strm << "PFN: [" << newlocation << "] already exist!";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
        }
        
        // everyting seems to be ok. let's update the LFN
        sync_add_location(ret, newlocation);
        sync_remove_location(ret, oldlocation);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    void logical_file_cpi_impl::sync_replicate(saga::impl::void_t&, saga::url location, 
                                               int mode)
    {
        SAGA_ADAPTOR_THROW ("Not implemented yet!", saga::NotImplemented);
    }
    
    ///////////////////////////////////////////////////////////////////////////////
}   // namespace logicalfile

