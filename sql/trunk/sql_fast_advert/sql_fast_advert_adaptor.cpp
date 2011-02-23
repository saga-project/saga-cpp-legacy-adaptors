//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

// adaptor includes
#include "sql_fast_advert_adaptor.hpp"
#include "sql_fast_advert_advert.hpp"
#include "sql_fast_advert_advert_directory.hpp"

SAGA_ADAPTOR_REGISTER (sql_fast_advert::adaptor);


////////////////////////////////////////////////////////////////////////
namespace sql_fast_advert
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

} // namespace sql_fast_advert
////////////////////////////////////////////////////////////////////////

