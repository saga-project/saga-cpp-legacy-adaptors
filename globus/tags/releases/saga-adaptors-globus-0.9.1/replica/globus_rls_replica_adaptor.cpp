//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/task.hpp>

#include "globus_rls_replica_adaptor.hpp"
#include "globus_rls_replica_adaptor_logicalfile.hpp"
#include "globus_rls_replica_adaptor_logicaldirectory.hpp"

#include "../loader/globus_global_loader.hpp"

SAGA_ADAPTOR_REGISTER (globus_rls_replica_adaptor::adaptor);

using namespace globus_rls_replica_adaptor;

adaptor::adaptor() : RLSConnectionPool_(NULL)
{
    // load the required globus modules
    globus_module_loader::globus_init ();
}

adaptor::~adaptor()
{
    delete RLSConnectionPool_;
}

//// register function for the SAGA engine ///////////////////////////////////
//
saga::impl::adaptor_selector::adaptor_info_list_type
adaptor::adaptor_register(saga::impl::session *s)
{
    saga::impl::adaptor_selector::adaptor_info_list_type list;
    preference_type prefs; 
    
    logical_file_cpi_impl::register_cpi(list, prefs, adaptor_uuid_);
    logical_directory_cpi_impl::register_cpi(list, prefs, adaptor_uuid_);
    
    return list;
}

//// connection pool handling - retrive or create a handle ///////////////////
//
RLSConnection* adaptor::getConnectionHandle(const saga::url url)
{
    if( RLSConnectionPool_ == NULL )
        RLSConnectionPool_ = new RLSConnectionPool_type_;
    
    RLSConnectionPool_type_::const_iterator findIterator;
    
    findIterator = RLSConnectionPool_->find(url.get_url());
    if(findIterator == RLSConnectionPool_->end() )
    {
        // add the url to the pool and create a new handle
        std::pair< RLSConnectionPool_type_::const_iterator, bool > insertResult;
        insertResult = RLSConnectionPool_->
        insert( make_pair( url.get_url(), new RLSConnection(url.get_url()) ) );
        
        return (insertResult.first)->second;
    }
    else
    {
        return findIterator->second;
    }
}

//// connection pool handling - remove a handle //////////////////////////////
//
void adaptor::removeConnectionHandle (const saga::url url)
{
    // extract protocol, server & port from given url
    std::string location("");
    location.append(url.get_scheme());
    location.append("://");
    location.append(url.get_host());
    
    if( RLSConnectionPool_ == NULL )
    {
        return;
    }
    else
    {
        RLSConnectionPool_->erase(location);
    }
}

