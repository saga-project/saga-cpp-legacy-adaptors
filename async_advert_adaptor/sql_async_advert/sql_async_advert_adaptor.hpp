//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_SQL_ASYNC_ADVERT_ADAPTOR_HPP
#define ADAPTORS_SQL_ASYNC_ADVERT_ADAPTOR_HPP

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

// boost includes
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

// STL includes
#include <string>
#include <map>

// server_connection
#include "sql_async_server_connection.hpp"

////////////////////////////////////////////////////////////////////////
namespace sql_async_advert
{
  struct adaptor : public saga::adaptor
  {
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    // This function registers the adaptor with the factory
    // @param factory the factory where the adaptor registers
    //        its maker function and description table
    saga::impl::adaptor_selector::adaptor_info_list_type 
          adaptor_register (saga::impl::session * s);

    std::string get_name (void) const
    { 
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }
    
    // ===============
    // = Constructor =
    // ===============
    
    adaptor();
    
    // ==============
    // = Destructor =
    // ==============
    
    ~adaptor();
    
    // ==========
    // = members =
    // ==========
    
    boost::asio::io_service   io_service;
    
    // =================================================
    // = Dummy work item to keep the io_service allive =
    // =================================================
    typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
    work_ptr                  work;
        
    // =========================
    // = ASIO execution thread =
    // =========================
    boost::thread             thread;
    
    // ===================
    // = Connection Map  =
    // ===================
    typedef std::map<std::string, server_connection*> connection_map_t;
    connection_map_t*      connection_map;
    
    // ====================================================
    // = Get server connection or create a new connection =
    // ====================================================
    server_connection* get_server_connection(saga::url url);
    
  };

} // namespace sql_async_advert
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_SQL_ASYNC_ADVERT_ADAPTOR_HPP

