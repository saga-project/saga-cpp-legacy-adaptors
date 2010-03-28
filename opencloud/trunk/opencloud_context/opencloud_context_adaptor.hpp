//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_OPENCLOUD_CONTEXT_ADAPTOR_HPP
#define ADAPTORS_OPENCLOUD_CONTEXT_ADAPTOR_HPP

#include <map>
#include <vector>

#include <saga/saga/util.hpp>
#include <saga/saga/types.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/context_cpi_instance_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/context_cpi.hpp>

#include <secsp_service.hpp>

namespace opencloud_context_adaptor 
{
  ///////////////////////////////////////////////////////////////////////////
  //    
  struct context_adaptor : public saga::adaptor
  {
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;


    context_adaptor (void) 
    {
    }

    ~context_adaptor (void)
    {
    }

    std::string get_name (void) const
    {
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }

    saga::impl::adaptor_selector::adaptor_info_list_type 
      adaptor_register (saga::impl::session * s);
  };
  //
  ///////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////
  //
  class context_cpi_impl 
    : public saga::adaptors::v1_0::context_cpi <context_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::context_cpi <context_cpi_impl> base_cpi;
      saga::ini::ini a_ini ; 
      saga::ini::ini g_ini ; 
      saga::session s_ ; 
      saga_sectorsphere::authenticator auth_ ; 
      bool authenticated_flag ; 

    public:    
      typedef base_cpi::mutex_type mutex_type;

      // constructor of the context CPI
      context_cpi_impl (proxy                * p, 
                        cpi_info       const & info, 
                        saga::ini::ini const & glob_ini, 
                        saga::ini::ini const & adap_ini,
                        TR1::shared_ptr <saga::adaptor> adaptor);

      // destructor of the file adaptor 
      ~context_cpi_impl (void);

      // context functions
      void sync_set_defaults (saga::impl::void_t &);


      int authenticate (const saga::session &s,
                                          saga::ini::ini const & glob_ini,
                                          saga::ini::ini const & adap_ini) ; 


      bool context_cpi_impl::check_ini_conf( std::string &path, saga::ini::entry_map &adap_ini) ; 
      bool check_ini_for_sec( std::string & sec, saga::ini::entry_map &adap_ini ) ; 
      bool check_ini_for_username( std::string & user, saga::ini::entry_map &adap_ini )   ; 
      bool check_ini_for_pass( std::string & pass, saga::ini::entry_map  &adap_ini )  ; 
      bool check_ini_for_server( std::string & serv, int & port, saga::ini::entry_map &adap_ini )  ; 


      bool populate_user_name( std::string &user, saga::adaptors::attribute const  & c_ , saga::ini::entry_map &adap_ini )  ; 
      bool populate_password( std::string &pass, saga::adaptors::attribute const  &c_ , saga::ini::entry_map &adap_ini )  ; 
      bool populate_server_info ( std::string &server, saga::adaptors::attribute const  & c_ , saga::ini::entry_map  &adap_ini, int &port ) ; 
      bool populate_sec_cert ( std::string &sec, saga::adaptors::attribute const  & c_ , saga::ini::entry_map &adap_ini ) ; 
      void get_authentication_error( std::string &err, int const & code ) ; 

  };  
  //
  ///////////////////////////////////////////////////////////////////////////

}

#endif // ADAPTORS_OPENCLOUD_CONTEXT_ADAPTOR_HPP

