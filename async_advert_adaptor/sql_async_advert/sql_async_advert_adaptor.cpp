//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

// adaptor includes
#include "sql_async_advert_adaptor.hpp"
#include "sql_async_advert_advert.hpp"
#include "sql_async_advert_advert_directory.hpp"

SAGA_ADAPTOR_REGISTER (sql_async_advert::adaptor);


////////////////////////////////////////////////////////////////////////
namespace sql_async_advert
{

  // register function for the SAGA engine
  saga::impl::adaptor_selector::adaptor_info_list_type
  adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type list;

    // create empty preference list
    // these list should be filled with properties of the adaptor, 
    // which can be used to select adaptors with specific preferences.
    // Example:
    //   'security' -> 'gsi'
    //   'logging'  -> 'yes'
    //   'auditing' -> 'no'
    preference_type prefs;

    // create advert and advert directory cpi infos (each adaptor 
    // instance gets its own uuid) and add cpi_infos to list
    advert_cpi_impl::register_cpi          (list, prefs, adaptor_uuid_);
    advertdirectory_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);

    // and return list
    return list;
  }
  
  adaptor::adaptor() : work(work_ptr(new boost::asio::io_service::work(io_service))), thread(boost::thread(boost::bind(&boost::asio::io_service::run, &io_service)))
  {
    connection_map = new connection_map_t();
  }
  
  adaptor::~adaptor()
  {
    work.reset();
    io_service.stop();
    thread.join();
    
    delete connection_map;
  }
  
  server_connection* adaptor::get_server_connection(saga::url url)
  {
    
    // Read some stuff from the .ini file
     //     saga::ini::ini prefs = adap_ini.get_section ("preferences");

    connection_map_t::iterator i = connection_map->find(url.get_host());
    
    if (i == connection_map->end())
    {
      (*connection_map)[url.get_host()] = new server_connection(url, io_service);
    }
    
    return (*connection_map)[url.get_host()];
  }

} // namespace sql_async_advert
////////////////////////////////////////////////////////////////////////

