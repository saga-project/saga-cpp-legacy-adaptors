//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>


#include <saga/saga/adaptors/utils/utils.hpp>
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/attribute.hpp>

#include "aws_context_adaptor.hpp"

namespace aws_context_adaptor 
{
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  //
  // adaptor methods
  //
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////

  SAGA_ADAPTOR_REGISTER (context_adaptor);

  ///////////////////////////////////////////////////////////////////////////////
  // 
  // adaptor init function which checks the adaptor settings
  //
  bool context_adaptor::init (saga::impl::session  * s,
                              saga::ini::ini const & glob_ini, 
                              saga::ini::ini const & adap_ini)
  {
    try 
    {
      // will throw if no such section exists.  That is ok, this adaptor *needs*
      // preferences
      get_ini (adap_ini.get_section ("preferences"));
    }
    catch ( const saga::exception & e )
    {
      SAGA_LOG_DEBUG (e.what ());

      return false;
    }

    // create a default security context if this is a default session
    if ( s->is_default_session () )
    {

      {
        std::vector <std::pair <std::string, std::string> > entries;
        std::pair <std::string, std::string> entry (saga::attributes::context_type, "ec2");
        entries.push_back (entry);
        s->add_proto_context (entries);
      }


      {
        std::vector <std::pair <std::string, std::string> > entries;
        std::pair <std::string, std::string> entry (saga::attributes::context_type, "fgeuca");
        entries.push_back (entry);
        s->add_proto_context (entries);
      }

      // {
      //   std::vector <std::pair <std::string, std::string> > entries;
      //   std::pair <std::string, std::string> entry (saga::attributes::context_type, "eucalyptus");
      //   entries.push_back (entry);
      //   s->add_proto_context (entries);
      // }

      // {
      //   std::vector <std::pair <std::string, std::string> > entries;
      //   std::pair <std::string, std::string> entry (saga::attributes::context_type, "nimbus");
      //   entries.push_back (entry);
      //   s->add_proto_context (entries);
      // }

    }

    return true;
  }
  //
  ///////////////////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////////////////
  //
  // adaptor register
  //
  saga::impl::adaptor_selector::adaptor_info_list_type
    context_adaptor::adaptor_register (saga::impl::session * s)
    {
      // list of implemented cpi's
      saga::impl::adaptor_selector::adaptor_info_list_type infos;
      preference_type prefs; 

      context_cpi_impl::register_cpi (infos, prefs, adaptor_uuid_);

      return infos;
    }
  //
  ///////////////////////////////////////////////////////////////////////////////



  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  //
  // cpi impl methods
  //
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////
  //
  //  constructor
  //
  context_cpi_impl::context_cpi_impl (proxy                * p, 
                                      cpi_info       const & info,
                                      saga::ini::ini const & glob_ini, 
                                      saga::ini::ini const & adap_ini,
                                      TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags), 
      initialized_ (false)
  {
    check_type ();
  }
  //
  ///////////////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////////////
  //
  //  destructor
  //
  context_cpi_impl::~context_cpi_impl (void)
  {
  }
  //
  ///////////////////////////////////////////////////////////////////////////////



  ///////////////////////////////////////////////////////////////////////////////
  //
  // set defaults.  
  //
  // On the first call, the context gets initialized.  Subsequent calls do
  // nothing.
  //
  void context_cpi_impl::sync_set_defaults (saga::impl::void_t &)
  {    
    if ( ! initialized_ )
    {
      type_ = check_type ();

      adaptor_data              adata (this);
      saga::adaptors::attribute attr  (this);

      ini_ = adata->ini_[type_];

      env_["JAVA_HOME"]       = ini_["java_home"];
      env_["EC2_HOME"]        = ini_["ec2_home"];
      env_["EC2_GSG_KEY"]     = ini_["ec2_proxy"];
      env_["EC2_PRIVATE_KEY"] = ini_["ec2_key"];
      env_["EC2_CERT"]        = ini_["ec2_cert"];
      env_["EC2_URL"]         = ini_["ec2_url"];
    

      cert_info_t ci;

      // this call looks for a valid cert in a default location
      ci = get_cert_info();    

      if ( true == ci.success )
      {
        // found a valid cert, copy information over
        attr.set_attribute (saga::attributes::context_userkey,   ci.key);
        attr.set_attribute (saga::attributes::context_usercert,  ""); // lives in the cloud, only
        attr.set_attribute (saga::attributes::context_userid,    ci.userid);
      }
      else
      {
        SAGA_ADAPTOR_THROW (ci.errormessage, saga::NoSuccess);
      }

      initialized_ = true;
    }
  }
  //
  ///////////////////////////////////////////////////////////////////////////////


  ///////////////////////////////////////////////////////////////////////////////
  //
  // init a cert, either from a given key, or from a default location
  //
  context_cpi_impl::cert_info_t context_cpi_impl::get_cert_info (void)
  {
    cert_info_t ci;

    ci.success      = false;
    ci.errormessage = "";

    adaptor_data adata (this);

    saga::adaptors::utils::process proc (env_);    

    SAGA_LOG_DEBUG ("getting cert info for");
    SAGA_LOG_DEBUG (ini_["ec2_keypair"]);

    SAGA_LOG_DEBUG ("checking proxy at");
    SAGA_LOG_DEBUG (ini_["ec2_proxy"]);

    // check if file is not yet present
    struct stat buf;
    if ( 0 != ::stat (ini_["ec2_proxy"].c_str (), &buf) &&
         ( errno == ENOENT  ||
           errno == ENOTDIR )                  )
    {
      proc.set_cmd (ini_["ec2_home"] + "/bin/ec2-delete-keypair");
      
      proc.add_arg (ini_["ec2_keypair"]);

      proc.run_sync ();

      // we don't care if that fails, really

      proc.set_cmd    (ini_["ec2_home"] + "/bin/ec2-add-keypair");
      proc.clear_args ();


      if ( type_ == "nimbus" )
      {
        // well, nimbus is different...
        proc.add_arg (ini_["ec2_keypair"] + "||" + ini_["ec2_ssh_pubkey"]);
      }
      else
      {
        proc.add_arg (ini_["ec2_keypair"]);
      }

      proc.run_sync ();

      if ( proc.fail () )
      {
        SAGA_ADAPTOR_THROW ("Could not add keypair", saga::NoSuccess);
      }

      std::fstream ec2_proxy_file (ini_["ec2_proxy"].c_str (), std::fstream::out);

      ec2_proxy_file << proc.get_out_s () << std::endl;

      ec2_proxy_file.close ();

      ::chmod (ini_["ec2_proxy"].c_str (), S_IRUSR | S_IWUSR);
    }

    // FIXME: that idendity key works only for EC2!  For _my_ account!! ;-)
    // Any way to retrieve that dynamically?
    ci.userid   = "root";
    ci.key      = ini_["ec2_proxy"];
    ci.success  = true;

    return ci;
  }
  //
  ///////////////////////////////////////////////////////////////////////////////



  ///////////////////////////////////////////////////////////////////////////////
  //
  //  check context type
  //
  std::string context_cpi_impl::check_type (void)
  {
    adaptor_data adata (this);

    saga::adaptors::attribute attr (this);
    std::vector <std::string> types = saga::adaptors::utils::split (adata->ini_["defaults"]["cloud_names"], ' ');

    if ( attr.attribute_exists (saga::attributes::context_type) )
    {
      for ( unsigned int i = 0; i < types.size (); i++ )
      {
        if ( types[i] == attr.get_attribute (saga::attributes::context_type) )
        {
          return types[i];
        }
      }
    }

    // FIXME
    SAGA_ADAPTOR_THROW ((std::string ("Can't handle context types others than ")
                         + adata->ini_["defaults"]["cloud_names"] + " - found "
                         + attr.get_attribute (saga::attributes::context_type)).c_str (), 
                        saga::BadParameter);

    return "Unknown";
  }
  //
  ///////////////////////////////////////////////////////////////////////////////



  ///////////////////////////////////////////////////////////////////////////////
  //
  // parse the adaptor ini, and extract all information into a map hierarchy
  //
  void context_adaptor::get_ini (saga::ini::ini const adap_ini)
  {
    if ( ! adap_ini.has_entry ("cloud_names" ) ||
         ! adap_ini.has_entry ("ec2_keystore") || 
         ! adap_ini.has_entry ("java_home"   ) ||
         ! adap_ini.has_entry ("ec2_home"    ) )
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT ("incomplete ini section for ec2.",
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
        // sub_prefs.dump ();
        SAGA_ADAPTOR_THROW_NO_CONTEXT ((std::string ("Settings incomplete for cloud type ") + name).c_str (),
                                       saga::NoSuccess);
      }


      ini_[name]["cloud_names"  ] = ini_["defaults"]["cloud_names"  ];
      ini_[name]["ec2_keystore" ] = ini_["defaults"]["ec2_keystore" ];
      ini_[name]["java_home"    ] = ini_["defaults"]["java_home"    ];
      ini_[name]["ec2_home"     ] = ini_["defaults"]["ec2_home"     ];
      ini_[name]["ec2_keepalive"] = ini_["defaults"]["ec2_keepalive"];

      
      std::map <std::string, std::string> settings = sub_prefs.get_entries ();
      std::map <std::string, std::string> :: iterator it;

      for ( it = settings.begin (); it != settings.end (); it++ )
      {
        ini_[name][(*it).first] = (*it).second;
      }
    }
  }

} // namespace aws_context_adaptor 

