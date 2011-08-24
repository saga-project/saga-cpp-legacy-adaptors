//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)
 

// saga includes
#include <saga/saga.hpp>

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

    
    // =====================================
    // = We understand absolute paths only =
    // =====================================
    
    if ('/' != url.get_path()[0])
    {
      SAGA_ADAPTOR_THROW("cannot handle relative advert directory name : " + url.get_string(), saga::IncorrectURL);
    }
    
    // ==============================================
    // = We understand only 'sqlasyncadvert' scheme =
    // ==============================================
    
    if (url.get_scheme() != "sqlasyncadvert")
    {
      SAGA_ADAPTOR_THROW("cannot handle advert directory name : " + url.get_string(), saga::adaptors::AdaptorDeclined);
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
    
    if ( (mode & saga::advert::Create ) || (mode & saga::advert::CreateParents) && (mode & saga::advert::Exclusive) )
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
        _connection->create_parents_directory(_path.string());
      }
      
      else 
      {     
        _connection->create_directory(_path.string());
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
    
    if (!_connection->exists_directory(_path.string()))
    {
      SAGA_ADAPTOR_THROW("advert does not exists : " + url.get_string(), saga::DoesNotExist); 
    }
  }


  ////////////////////////////////////////////////////////////////////////
  //  destructor
  advert_cpi_impl::~advert_cpi_impl (void)
  {
  }

// =========================================================================
// = Helper functions                                                      =
// =========================================================================

  void
    advert_cpi_impl::check_if_open(const bool state, std::string const &functionname)
    {
      instance_data idata(this);
      
      if (!state)
      {
        SAGA_OSSTREAM strm;
        strm << functionname << ": advert directory is not in open state: "
             << idata->location_;
        SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING (strm), saga::IncorrectState);
      }
    }


  ////////////////////////////////////////////////////////////////////////
  //  SAGA CPI functions 

//  ////////////////////////////////////////////////////////////////////////
//  // attribute functions

  void advert_cpi_impl::sync_attribute_exists (bool        & ret, 
                                               std::string   key)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_attribute_exists");

    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    std::map<std::string, JsonBox::Value>::iterator i = attributes.find(key);
    
    if (i != attributes.end())
    {
      ret = true;
    }
    
    else
    {
      ret = false;
    }
  }

  void advert_cpi_impl::sync_attribute_is_readonly (bool        & ret, 
                                                    std::string   key)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_attribute_is_readonly");
    
    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    std::map<std::string, JsonBox::Value>::iterator i = attributes.find(key);
    if (i == attributes.end())
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
    
    instance_data idata(this);
    ret = !(idata->mode_ & saga::advert::Read);
  }

  void advert_cpi_impl::sync_attribute_is_writable (bool        & ret, 
                                                    std::string   key)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_attribute_is_readonly");
    
    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    std::map<std::string, JsonBox::Value>::iterator i = attributes.find(key);
    if (i == attributes.end())
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
    
    instance_data idata(this);
    ret = (idata->mode_ & saga::advert::Write);
  }

  void advert_cpi_impl::sync_attribute_is_vector (bool        & ret, 
                                                  std::string   key)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_attribute_exists");

    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    std::map<std::string, JsonBox::Value>::iterator i = attributes.find(key);
    
    if (i == attributes.end())
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not exists", saga::DoesNotExist);
    }
    
    ret = i->second.isArray();
  }

  void advert_cpi_impl::sync_attribute_is_extended (bool        & ret, 
                                                    std::string   key)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

//  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_get_attribute (std::string & ret, 
                                            std::string   key)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_attribute_exists");

    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    std::map<std::string, JsonBox::Value>::iterator i = attributes.find(key);
    
    if (i == attributes.end())
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not exists", saga::DoesNotExist);
    }
    
    ret = i->second.getString();
  }

//  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_get_vector_attribute (std::vector <std::string> & ret, 
                                                   std::string                 key)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_attribute_is_readonly");
    
    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    std::map<std::string, JsonBox::Value>::iterator i = attributes.find(key);
    if (i == attributes.end())
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " dose not Exists", saga::DoesNotExist);
    }
    
    if (!(i->second.isArray()))
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " is not a vector attribute", saga::IncorrectState);
    }
    
    JsonBox::Array array = i->second.getArray();
    
    for (std::deque<JsonBox::Value>::iterator j = array.begin(); j != array.end(); ++j)
    {
      ret.push_back(j->getString());
    }
  }

//  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_set_attribute (saga::impl::void_t & ret, 
                                            std::string    key, 
                                            std::string    val)
  {
    _connection->set_attribute(_path.string(), key, val);
  }

//  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_set_vector_attribute (saga::impl::void_t            & ret, 
                                                   std::string               key, 
                                                   std::vector <std::string> val)
  {
     _connection->set_vector_attribute(_path.string(), key, val);
  }

//  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_remove_attribute (saga::impl::void_t & ret,
                                               std::string    key)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

//  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_list_attributes (std::vector <std::string> & ret)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_attribute_exists");

    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    for (std::map<std::string, JsonBox::Value>::iterator i = attributes.begin(); i != attributes.end(); ++i)
    {
      ret.push_back(i->first);
    }
    
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

//  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_find_attributes (std::vector<std::string> & ret, 
                                              std::string                pattern)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

//  ////////////////////////////////////////////////////////////////////////
//  // namespace_entry functions
  void advert_cpi_impl::sync_get_url (saga::url & url)
  {
     bool state = _connection->get_state(_path.string()); 
     check_if_open(state, "advertdirectory_cpi_impl::sync_get_url");
     
     instance_data idata(this);
     url = idata->location_.clone();
  }

  void advert_cpi_impl::sync_get_cwd(saga::url& url)
  {
    bool state = _connection->get_state(_path.string()); 
    check_if_open(state, "advertdirectory_cpi_impl::sync_get_url");
    
    url.set_path(_path.parent_path().string());
  }

  void advert_cpi_impl::sync_get_name (saga::url & url)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_get_name");
    
    JsonBox::Object obj = value.getObject();
    url.set_path(obj["name"].getString());
  }

  void advert_cpi_impl::sync_read_link (saga::url & url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_is_dir (bool & ret)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_is_dir");
    
    JsonBox::Object obj = value.getObject();
    ret = obj["dir"].getBoolean();
  }

  void advert_cpi_impl::sync_is_entry (bool & ret)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_is_entry");
    
    JsonBox::Object obj = value.getObject();
    ret = !(obj["dir"].getBoolean());
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
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_close (saga::impl::void_t & ret,
                                    double               timeout)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


//  ////////////////////////////////////////////////////////////////////////
//  // advert functions
  void advert_cpi_impl::sync_store_object (saga::impl::void_t & ret, 
                                           saga::object         obj)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_retrieve_object (saga::object & ret, 
                                              saga::session  s)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_store_string (saga::impl::void_t & ret, 
                                           std::string          str)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_retrieve_string (std::string & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace sql_async_advert
//////////////////////////////////////////////////////////////////////////

