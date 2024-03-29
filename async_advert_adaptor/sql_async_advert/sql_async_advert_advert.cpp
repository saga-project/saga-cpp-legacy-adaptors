//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)
 

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/serialization.hpp>

// adaptor includes
#include "sql_async_advert_advert.hpp"


////////////////////////////////////////////////////////////////////////
namespace sql_async_advert
{
  // ========================
  // = Normalize boost path =
  // ========================
  
  static boost::filesystem::path normalize_boost_path(const boost::filesystem::path path)
  {
    boost::filesystem::path normalized_path;

    for (boost::filesystem::path::iterator i = path.begin(); i != path.end(); i++)
    {
      if (*i == ".")
      {
        continue;
      }

      else if (*i == "..")
      {
        normalized_path = normalized_path.parent_path();
      }

      else 
      {
        normalized_path /= *i;
      }
    }

    if (normalized_path.string() == "")
    {
      normalized_path /= "/";
    }

    return normalized_path;
  }
  

  ////////////////////////////////////////////////////////////////////////
  //  constructor
  advert_cpi_impl::advert_cpi_impl (proxy                           * p, 
                                    cpi_info const                  & info,
                                    saga::ini::ini const            & glob_ini, 
                                    saga::ini::ini const            & adap_ini,
                                    TR1::shared_ptr <saga::adaptor>   adaptor)
    : saga::adaptors::v1_0::advert_cpi <advert_cpi_impl> (p, info, adaptor, cpi::Noflags)
  {
    instance_data idata(this);
    adaptor_data  adata(this);
    
    saga::url url(idata->location_);
    _path = normalize_boost_path(boost::filesystem::path(url.get_path()));
    
    // ==============================================
    // = We understand only 'sqlasyncadvert' scheme =
    // ==============================================
    
    if (url.get_scheme() != "sqlasyncadvert")
    {
      SAGA_ADAPTOR_THROW("cannot handle advert directory name : " + url.get_string(), saga::adaptors::AdaptorDeclined);
    }
    
    // =====================================
    // = We understand absolute paths only =
    // =====================================
    
    if ('/' != url.get_path()[0])
    {
      SAGA_ADAPTOR_THROW("cannot handle relative advert directory name : " + url.get_string(), saga::IncorrectURL);
    }
    
    // ======================================
    // = Connect to the Async Advert Server =
    // ======================================
    
    _connection = adata->get_server_connection(url);
    
    // ===============
    // = Check Flags =
    // ===============

    saga::advert::flags mode = (saga::advert::flags) idata->mode_;
    
    // =============
    // = Exclusive =
    // =============
    
    if ( ((mode & saga::advert::Create ) || (mode & saga::advert::CreateParents)) && (mode & saga::advert::Exclusive) )
    {
      if (_connection->exists_directory(_path.string()))
      {
        SAGA_ADAPTOR_THROW("advert already exists : " + url.get_string(), saga::AlreadyExists);
      }
    }
    
    // ===========================
    // = Create || CreateParents =
    // ===========================
    
    if ( (mode & saga::advert::Create) || (mode & saga::advert::CreateParents) )
    {
      if (mode & saga::advert::CreateParents)
      {
        _connection->create_parents_directory(_path.string(), false);
      }
      
      else 
      {     
        _connection->create_directory(_path.string(), false);
      }
    }

    // ===================================
    // = Without Create || CreateParents =
    // ===================================
    
    if ( !(mode & saga::advert::Create) || !(mode & saga::advert::CreateParents) )
    {
      _connection->open_directory(_path.string());
    }
    
    // ===============================
    // = make sure directory exists  =
    // ===============================
    
    if (!_connection->get_state(_path.string()))
    {
      SAGA_ADAPTOR_THROW("advert does not exists : " + url.get_string(), saga::DoesNotExist); 
    }
    
    _opened = true;
  }


  ////////////////////////////////////////////////////////////////////////
  //  destructor
  advert_cpi_impl::~advert_cpi_impl (void)
  {
    //_connection->close_directory(_path.string());
  }

// =========================================================================
// = Helper functions                                                      =
// =========================================================================

  void
    advert_cpi_impl::check_if_open(std::string const &functionname)
    {
      instance_data idata(this);
      
      if (!_opened)
      {
        SAGA_OSSTREAM strm;
        strm << functionname << ": advert entry is not in open state: "
             << idata->location_;
        SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING (strm), saga::IncorrectState);
      }
    }

  void
     advert_cpi_impl::check_permissions(saga::advert::flags flags, std::string const &functionname)
    {
      instance_data idata(this);

      if (!(idata->mode_ & flags))
      {
        SAGA_OSSTREAM strm;
        strm << functionname << ": could not access ("
             << ((flags & saga::advert::Read) ? "read" : "write")
             << ") attribute for advert: " 
             << idata->location_;
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING (strm), saga::PermissionDenied);
      }
    }

  ////////////////////////////////////////////////////////////////////////
  //  SAGA CPI functions 

//  ////////////////////////////////////////////////////////////////////////
//  // attribute functions

  void advert_cpi_impl::sync_attribute_exists (bool        & ret, 
                                               std::string   key)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_attribute_exists");
    
    if (value["attributes"].isMember(key))
    {
        ret = true;
    }
    else
    {
      ret = false;
    }
  }

////////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_attribute_is_readonly (bool        & ret, 
                                                    std::string   key)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_attribute_is_readonly");
    
    if (!value["attributes"].isMember(key))
    {
     SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
     
    instance_data idata(this);
    ret = (idata->mode_ & saga::advert::Read) ? false : true;
  }

////////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_attribute_is_writable (bool        & ret, 
                                                    std::string   key)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
     
    check_if_open("advertdirectory_cpi_impl::sync_attribute_is_readonly");

    if (!value["attributes"].isMember(key))
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
       
    instance_data idata(this);
    ret = (idata->mode_ & saga::advert::Write) ? true : false;
  }

////////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_attribute_is_vector (bool        & ret, 
                                                  std::string   key)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
     
    check_if_open("advertdirectory_cpi_impl::sync_attribute_is_vector");
    check_permissions(saga::advert::Read, "advertdirectory_cpi_impl::sync_attribute_is_vector");
     
    if (!value["attributes"].isMember(key))
    {
     SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
     
    ret = value["attributes"][key].isArray();
  }
  
////////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_attribute_is_extended (bool        & ret, 
                                                    std::string   key)
  {
    instance_data idata(this);
    
    _opened = _connection->get_state(_path.string()); 
    check_if_open("advertdirectory_cpi_impl::sync_attribute_is_extended");
    
    ret = (idata->mode_ & saga::advert::Write) ? true : false;
  }

////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_get_attribute (std::string & ret, 
                                            std::string   key)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_get_attribute");
    check_permissions(saga::advert::Read, "advertdirectory_cpi_impl::sync_get_attribute");
    
    if (!value["attributes"].isMember(key))
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
    
    if (value["attributes"][key].isArray())
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " is a vector attribute", saga::IncorrectState);
    }
    
    ret = value["attributes"][key].asString();
  }

////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_get_vector_attribute (std::vector <std::string> & ret, 
                                                   std::string                 key)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
     
    check_if_open("advertdirectory_cpi_impl::sync_get_vector_attribute");
    check_permissions(saga::advert::Read, "advertdirectory_cpi_impl::sync_get_vector_attribute");
     
    if (!value["attributes"].isMember(key))
    {
     SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
     
    if (!value["attributes"][key].isArray())
    {
     SAGA_ADAPTOR_THROW ("Attribute " + key + " is not a vector attribute", saga::IncorrectState);
    }
    
    for (unsigned int i = 0; i < value["attributes"][key].size(); ++i)
    {
      ret.push_back(value["attributes"][key][i].asString());
    }
  }

////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_set_attribute (saga::impl::void_t & ret, 
                                            std::string    key, 
                                            std::string    val)
  {
    _opened = _connection->get_state(_path.string()); 
    
    check_if_open("advertdirectory_cpi_impl::sync_set_attribute");
    check_permissions(saga::advert::Write, "advertdirectory_cpi_impl::sync_get_vector_attribute");
    
    _connection->set_attribute(_path.string(), key, val);
  }

////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_set_vector_attribute (saga::impl::void_t            & ret, 
                                                   std::string               key, 
                                                   std::vector <std::string> val)
  {
    _opened = _connection->get_state(_path.string()); 
    
    check_if_open("advertdirectory_cpi_impl::sync_set_vector_attribute");
    check_permissions(saga::advert::Write, "advertdirectory_cpi_impl::sync_set_vector_attribute");
    
    _connection->set_vector_attribute(_path.string(), key, val);
  }

////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_remove_attribute (saga::impl::void_t & ret,
                                               std::string    key)
  {
    _opened = _connection->get_state(_path.string()); 
    
    check_if_open("advertdirectory_cpi_impl::sync_remove_attribute");
    check_permissions(saga::advert::Write, "advertdirectory_cpi_impl::sync_remove_attribute");
    
    _connection->remove_attribute(_path.string(), key);
  }

////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_list_attributes (std::vector <std::string> & ret)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_list_attributes");
    check_permissions(saga::advert::Read, "advertdirectory_cpi_impl::sync_list_attributes");
    
    ret = value["attributes"].getMemberNames();
  }

////////////////////////////////////////////////////////////////////////

  void advert_cpi_impl::sync_find_attributes (std::vector<std::string> & ret, 
                                              std::string                pattern)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

////////////////////////////////////////////////////////////////////////
// namespace_entry functions

  void advert_cpi_impl::sync_get_url (saga::url & url)
  {
    _opened = _connection->get_state(_path.string()); 
    check_if_open("advertdirectory_cpi_impl::sync_get_url");
    
    instance_data idata(this);
    url = idata->location_;
  }

  void advert_cpi_impl::sync_get_cwd(saga::url& url)
  {
    _opened = _connection->get_state(_path.string()); 
    check_if_open("advertdirectory_cpi_impl::sync_get_cwd");
    
    url.set_path(_path.parent_path().string());
  }

  void advert_cpi_impl::sync_get_name (saga::url & url)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_get_name");

    url.set_path(value["name"].asString());
  }

  void advert_cpi_impl::sync_read_link (saga::url & url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_is_dir (bool & ret)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_is_dir");

    ret = value["dir"].asBool();
  }

  void advert_cpi_impl::sync_is_entry (bool & ret)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_is_dir");

    ret = !value["dir"].asBool();
  }

  void advert_cpi_impl::sync_is_link (bool & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                   saga::url            target,
                                   int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                   saga::url            target,
                                   int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                   saga::url            target,
                                   int                  flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                     int                  flags)
  {
    _opened = _connection->get_state(_path.string()); 
    
    check_if_open("advertdirectory_cpi_impl::sync_remove");
    check_permissions(saga::advert::Write, "advertdirectory_cpi_impl::sync_remove");
    
    if (!(flags & saga::advert::Recursive))
    {
      SAGA_ADAPTOR_THROW(
                  "advert::advertdirectory_cpi_impl::sync_remove: "
                  "Recursive flag was not specified while attempting to delete a "
                  "directory", saga::BadParameter);
    }
    
    _connection->remove_directory(_path.string());
    _opened = false;
  }

  void advert_cpi_impl::sync_close (saga::impl::void_t & ret,
                                    double               timeout)
  {
    _connection->close_directory(_path.string());
    _opened = false;
  }


//  ////////////////////////////////////////////////////////////////////////
//  // advert functions
  void advert_cpi_impl::sync_store_object (saga::impl::void_t & ret, 
                                           saga::object         obj)
  {
    _opened = _connection->get_state(_path.string()); 
    
    check_if_open("advertdirectory_cpi_impl::sync_store_object");
    check_permissions(saga::advert::Write, "advertdirectory_cpi_impl::sync_store_object");
    
    _connection->set_string(_path.string(), saga::adaptors::serialize(obj));
    
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_retrieve_object (saga::object & ret, 
                                              saga::session  s)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_retrieve_object");
    check_permissions(saga::advert::Read, "advertdirectory_cpi_impl::sync_retrieve_object");
    
    
    ret = saga::adaptors::deserialize(s, value["data"].asString());
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_store_string (saga::impl::void_t & ret, 
                                           std::string          str)
  {
    _opened = _connection->get_state(_path.string()); 
    
    check_if_open("advertdirectory_cpi_impl::sync_store_string");
    check_permissions(saga::advert::Write, "advertdirectory_cpi_impl::sync_store_string");
    
    _connection->set_string(_path.string(), str);
    
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_retrieve_string (std::string & ret)
  {
    Json::Value value;
    _opened = _connection->get_value(_path.string(), value);
    
    check_if_open("advertdirectory_cpi_impl::sync_retrieve_string");
    check_permissions(saga::advert::Read, "advertdirectory_cpi_impl::sync_retrieve_string");
    
    
    ret = value["data"].asString();
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace sql_async_advert
//////////////////////////////////////////////////////////////////////////

