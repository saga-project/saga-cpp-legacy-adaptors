//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include "globus_gridftp_file_adaptor.hpp"
#include "globus_gridftp_file_adaptor_dir.hpp"
#include "globus_gridftp_file_adaptor_file.hpp"

#include "../loader/globus_global_loader.hpp"

using namespace globus_gridftp_file_adaptor;

SAGA_ADAPTOR_REGISTER (file_adaptor);

file_adaptor::file_adaptor() : ConnectionPool_(NULL)
{    
    // load the required globus modules
    globus_module_loader::globus_init ();
}

file_adaptor::~file_adaptor()
{
    delete ConnectionPool_;
}

GridFTPConnection*  file_adaptor::getConnectionHandleForURL (const saga::url url,
                                                             bool enable_log, 
                                                             std::string logfile_name)
{
  if( ConnectionPool_ == NULL )
  {
    ConnectionPool_ = new connection_pool_type_;
  }
      
  // extract protocol, server & port from given url
  std::string location("");
  location.append(url.get_scheme());
  location.append("://");
  location.append(url.get_host());
  
  connection_pool_type_::const_iterator FindIterator;
  
  FindIterator = ConnectionPool_->find( location );
  if( FindIterator == ConnectionPool_->end() )
  {
    // Add the url to the pool and create a new globus_ftp_handle
    std::pair< connection_pool_type_::const_iterator, bool > InsertResult;
    InsertResult = ConnectionPool_->
      insert( make_pair( location, new GridFTPConnection(location, enable_log, logfile_name) ) );
    
    return (InsertResult.first)->second;
  }
  else
  {
    return FindIterator->second;
  }
}


void  file_adaptor::removeConnectionHandle (const saga::url url)
{
  // extract protocol, server & port from given url
  std::string location("");
  location.append(url.get_scheme());
  location.append("://");
  location.append(url.get_host());
  
  if( ConnectionPool_ == NULL )
  {
    return;
  }
  else
  {
    ConnectionPool_->erase(location);
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
    dir_cpi_impl::register_cpi  (infos, prefs, adaptor_uuid_);

    
    //permissions_cpi_impl::register_cpi(infos, prefs, adaptor_uuid_);

    //// create a default security context if this is a default session
    //
    /*if (s->is_default_session())
    {
        typedef std::pair<std::string, std::string> entry_type;
        
        std::vector<entry_type> entries;
        
        entries.push_back( entry_type(saga::attributes::context_type,
                                      "Globus X.509") );
        s->add_proto_context(entries);
    }*/
    
    return infos;
}

