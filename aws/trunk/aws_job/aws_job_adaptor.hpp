//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_AWS_JOB_ADAPTOR_HPP
#define ADAPTORS_AWS_JOB_ADAPTOR_HPP

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

////////////////////////////////////////////////////////////////////////
namespace aws_job
{
  class adaptor : public saga::adaptor
  {
    public:
      typedef saga::impl::v1_0::op_info         op_info;  
      typedef saga::impl::v1_0::cpi_info        cpi_info;
      typedef saga::impl::v1_0::preference_type preference_type;

      std::map <std::string, std::map <std::string, std::string> > ini_;

      // This function registers the adaptor with the factory
      // @param factory the factory where the adaptor registers
      //        its maker function and description table
      saga::impl::adaptor_selector::adaptor_info_list_type 
        adaptor_register (saga::impl::session * s);

      bool init (saga::impl::session  * s,
                 saga::ini::ini const & glob_ini, 
                 saga::ini::ini const & adap_ini);

      std::string get_name (void) const
      { 
        return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
      }

      ///////////////////////////////////////////////////////////////////////////////
      //
      // parse the adaptor ini, and extract all information into a map hierarchy
      //
      void get_ini (saga::ini::ini const adap_ini);
  };

} // namespace aws_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_AWS_JOB_ADAPTOR_HPP

