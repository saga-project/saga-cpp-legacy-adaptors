//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_AWS_CONTEXT_ADAPTOR_HPP
#define ADAPTORS_AWS_CONTEXT_ADAPTOR_HPP

#include <map>
#include <vector>

#include <saga/saga/util.hpp>
#include <saga/saga/types.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/context_cpi_instance_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/context_cpi.hpp>

#include <aws_suite.hpp>

namespace aws_context_adaptor 
{
  ///////////////////////////////////////////////////////////////////////////
  //    
  class context_adaptor : public saga::adaptor
  {
    public:
      std::map <std::string, std::map <std::string, std::string> > ini_;

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

      bool init (saga::impl::session  * s,
                 saga::ini::ini const & glob_ini, 
                 saga::ini::ini const & adap_ini);

      saga::impl::adaptor_selector::adaptor_info_list_type 
        adaptor_register (saga::impl::session * s);


      ///////////////////////////////////////////////////////////////////////////////
      //
      // parse the adaptor ini, and extract all information into a map hierarchy
      //
      void get_ini (saga::ini::ini const adap_ini);
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
      typedef saga::adaptors::adaptor_data      <context_adaptor>  adaptor_data;


      struct cert_info_t {
        bool         success;
        std::string  errormessage;
        std::string  userid;
        std::string  key;
      };

      cert_info_t get_cert_info (void);
      std::string check_type    (void);

      std::string type_;
      bool initialized_;

      std::map <std::string, std::string> ini_;
      std::map <std::string, std::string> env_;


    public:    
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

      void init_ (void);
  };  
  //
  ///////////////////////////////////////////////////////////////////////////

}

#endif // ADAPTORS_AWS_CONTEXT_ADAPTOR_HPP

