//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

// adaptor includes
#include "aws_job_adaptor.hpp"
#include "aws_job_service.hpp"

SAGA_ADAPTOR_REGISTER (aws_job::adaptor);


////////////////////////////////////////////////////////////////////////
//
// This adaptor only implements the job service API - all jobs are actually
// managed by the ssh job service adaptor
//
namespace aws_job
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

    // create file adaptor infos (each adaptor instance gets its own uuid)
    // and add cpi_infos to list
    job_service_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);

    // and return list
    return (list);
  }


  bool adaptor::init (saga::impl::session  * s,
                      saga::ini::ini const & glob_ini, 
                      saga::ini::ini const & adap_ini)
  {
    try 
    {
      // the following throws if no such section exists - thats ok, this adaptor
      // *needs* preferences
      get_ini (adap_ini.get_section ("preferences"));
    }
    catch ( const saga::exception & e )
    {
      SAGA_LOG_DEBUG (e.what ());
      return false;
    }

    return true;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // parse the adaptor ini, and extract all information into a map hierarchy
  //
  void adaptor::get_ini (saga::ini::ini const adap_ini)
  {
    if ( ! adap_ini.has_entry ("cloud_names" ) ||
         ! adap_ini.has_entry ("ec2_keystore") || 
         ! adap_ini.has_entry ("java_home"   ) ||
         ! adap_ini.has_entry ("ec2_home"    ) )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("iws-job ni preferences incomplete - abort",
                                     saga::NoSuccess);
    }

    std::map <std::string, std::string> defaults = adap_ini.get_entries ();

    ini_["defaults"] = defaults;


    // on default, don't keep VMs alive
    if ( ! adap_ini.has_entry ("ec2_keepalive" ) )
    {
      ini_["defaults"]["ec2_keepalive"] = "false";
    }


    std::vector <std::string> cloud_names = saga::adaptors::utils::split (ini_["defaults"]["cloud_names"], ' ');

    for ( unsigned int i = 0; i < cloud_names.size (); i++ )
    {
      std::string name = cloud_names[i];

      if ( ! adap_ini.has_section (name) )
      {
        SAGA_ADAPTOR_THROW_NO_CONTEXT ((std::string ("Adaptor needs preferences to be set."
                                                     " for that cloud type: ") + name).c_str (),
                                       saga::NoSuccess);
      }

      saga::ini::ini sub_prefs = adap_ini.get_section (name);

      if ( ! sub_prefs.has_entry ("ec2_proxy"       ) ||
           ! sub_prefs.has_entry ("ec2_key"         ) ||
           ! sub_prefs.has_entry ("ec2_cert"        ) ||
           ! sub_prefs.has_entry ("ec2_url"         ) ||
           ! sub_prefs.has_entry ("ec2_keypair"     ) ||
           ! sub_prefs.has_entry ("ec2_image_id"    ) )
      {
        SAGA_ADAPTOR_THROW_NO_CONTEXT ((std::string ("Settings incomplete for cloud type ") + name).c_str (),
                                       saga::NoSuccess);
      }


      ini_[name]["cloud_names"  ] = ini_["defaults"]["cloud_names"  ];
      ini_[name]["ec2_keystore" ] = ini_["defaults"]["ec2_keystore" ];
      ini_[name]["java_home"    ] = ini_["defaults"]["java_home"    ];
      ini_[name]["ec2_home"     ] = ini_["defaults"]["ec2_home"     ];
      ini_[name]["ec2_scripts"  ] = ini_["defaults"]["ec2_scripts"  ];
      ini_[name]["ec2_keepalive"] = ini_["defaults"]["ec2_keepalive"];

      
      std::map <std::string, std::string> settings = sub_prefs.get_entries ();
      std::map <std::string, std::string> :: iterator it;

      for ( it = settings.begin (); it != settings.end (); it++ )
      {
        ini_[name][(*it).first] = (*it).second;
      }
    }
  }

} // namespace aws_job
////////////////////////////////////////////////////////////////////////

