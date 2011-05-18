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

	adaptor::adaptor()
	{
		database_connection_map = new std::map<std::string, database_connection*>();
	}
	
	adaptor::~adaptor()
	{
		delete database_connection_map;
	}
	
	database_connection* adaptor::get_database_connection(saga::url url,  saga::ini::ini const &adap_ini)
	{
		database_connection_map_t::iterator it = database_connection_map->find(url.get_host());
		
		if (it == database_connection_map->end())
		{
			std::map<std::string, std::string> ini_file_options;
			
			// Read some stuff from the .ini file
		  	saga::ini::ini prefs = adap_ini.get_section ("preferences");

			if (prefs.has_entry("dbname")) 
			{
				std::string db_user = prefs.get_entry("dbname");
				ini_file_options["dbname"] = db_user;
			}
			
			if (prefs.has_entry("host")) 
			{
				std::string db_user = prefs.get_entry("host");
				ini_file_options["host"] = db_user;
			}

			if (prefs.has_entry("port")) 
			{
				std::string db_user = prefs.get_entry("port");
				ini_file_options["port"] = db_user;
			}
			
		   	if (prefs.has_entry("user")) 
		   	{
		    	std::string db_user = prefs.get_entry("user");
				ini_file_options["user"] = db_user;
		   	}
		
			if (prefs.has_entry("password"))
			{
				std::string db_pass = prefs.get_entry("password");
				ini_file_options["password"] = db_pass;
			}
			
			if (prefs.has_entry("connection_pool_size"))
			{
				std::string connection_pool_size = prefs.get_entry("connection_pool_size");
				ini_file_options["connection_pool_size"] = connection_pool_size;
			}
			
			if (prefs.has_entry("batch_size"))
			{
				std::string batch_size = prefs.get_entry("batch_size");
				ini_file_options["batch_size"] = batch_size;
			}
			
			if (prefs.has_entry("check_db"))
			{
				std::string batch_size = prefs.get_entry("check_db");
				ini_file_options["check_db"] = batch_size;
			}
			
			(*database_connection_map)[url.get_host()] = new database_connection(url, ini_file_options);
		}
		
		it = database_connection_map->find(url.get_host());
		return it->second;
	}

} // namespace sql_fast_advert
////////////////////////////////////////////////////////////////////////

