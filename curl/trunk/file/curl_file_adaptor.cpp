//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include "curl_file_adaptor.hpp"
#include "curl_file_adaptor_connection.hpp"

//#include "curl_file_adaptor_dir.hpp"
#include "curl_file_adaptor_file.hpp"
//#include "curl_file_adaptor_context.hpp"

//#include "curl_file_adaptor_namespace_dir.hpp"
//#include "curl_file_adaptor_namespace_entry.hpp"

using namespace curl_file_adaptor;

SAGA_ADAPTOR_REGISTER (file_adaptor);

///////////////////////////////////////////////////////////////////////////////
//
cURLconnection* file_adaptor::getConnectionHandle(const saga::url url)
{
    if( HandlePool_ == NULL )
        HandlePool_ = new handle_pool_type_;
    
    // extract protocol, server & port from given url
    // we use that as key for the connection pool...
    std::string key(url.get_scheme());
    key.append("://");
    key.append(url.get_host());
    if(url.get_port() > 0) {
        key.append(":");
        key.append(boost::lexical_cast<std::string>(url.get_port()));
    }
    
    handle_pool_type_::const_iterator findIterator;
    
    findIterator = HandlePool_->find( key );
    if( findIterator == HandlePool_->end() )
    {
        // Add the url to the pool and create a new globus_ftp_handle
        std::pair< handle_pool_type_::const_iterator, bool > InsertResult;
        InsertResult = HandlePool_->
        insert( make_pair( key, new cURLconnection(url.get_string()) ) );
        
        return (InsertResult.first)->second;
    }
    else
        return findIterator->second;
}


void  file_adaptor::removeConnectionHandle (const saga::url url)
{
    // extract protocol, server & port from given url
    std::string location("");
    location.append(url.get_scheme());
    location.append("://");
    location.append(url.get_host());
    
    if( HandlePool_ == NULL )
    {
        return;
    }
    else
    {
        HandlePool_->erase(location);
    }
}

///////////////////////////////////////////////////////////////////////////////
//
saga::impl::adaptor_selector::adaptor_info_list_type
    file_adaptor::adaptor_register(saga::impl::session *s)
{
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type infos;
    preference_type prefs; 

    file_cpi_impl::register_cpi (infos, prefs, adaptor_uuid_);
    //dir_cpi_impl::register_cpi  (infos, prefs, adaptor_uuid_);
    //context_cpi_impl::register_cpi(infos, prefs, adaptor_uuid_);

    //// create a default security context if this is a default session
    //
    if (s->is_default_session())
    {
        typedef std::pair<std::string, std::string> entry_type;
        
        std::vector<entry_type> entries;
        
        entries.push_back( entry_type(saga::attributes::context_type,
                                      "cURL ?!?") );
        s->add_proto_context(entries);
    }
    
    return infos;
}

