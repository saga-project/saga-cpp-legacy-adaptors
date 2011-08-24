//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>

// adaptor includes
#include "sql_async_advert_advert_directory.hpp"

///////////////////////////////////////////////////////////////////////////////
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
  
  
  // ===============
  // = Constructor =
  // ===============
  
  advertdirectory_cpi_impl::advertdirectory_cpi_impl (
        proxy                           * p, 
        cpi_info const                  & info,
        saga::ini::ini const            & glob_ini, 
        saga::ini::ini const            & adap_ini,
        TR1::shared_ptr <saga::adaptor>   adaptor ) : saga::adaptors::v1_0::advert_directory_cpi <advertdirectory_cpi_impl> (p, info, adaptor, cpi::Noflags) 
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


  // ==============
  // = Destructor =
  // ==============
  
  advertdirectory_cpi_impl::~advertdirectory_cpi_impl (void)
  {
  }

// =========================================================================
// = Helper functions                                                      =
// =========================================================================

  void
    advertdirectory_cpi_impl::check_if_open(const bool state, std::string const &functionname)
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

// =========================================================================
// = SAGA CPI Functions                                                    =
// =========================================================================

// =========================================================================
// = Attribute functions                                                   =
// =========================================================================

  void 
    advertdirectory_cpi_impl::sync_attribute_exists (bool        & ret, 
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

////////////////////////////////////////////////////////////////////////////

  void 
    advertdirectory_cpi_impl::sync_attribute_is_readonly (bool        & ret, 
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

////////////////////////////////////////////////////////////////////////////

  void 
    advertdirectory_cpi_impl::sync_attribute_is_writable (bool        & ret, 
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
  
////////////////////////////////////////////////////////////////////////////

  void 
    advertdirectory_cpi_impl::sync_attribute_is_vector (bool        & ret, 
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
    
    ret = i->second.isArray();
  }
  
////////////////////////////////////////////////////////////////////////////

  void 
    advertdirectory_cpi_impl::sync_attribute_is_extended (bool        & ret, 
                                                          std::string   key)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_get_attribute (std::string & ret, 
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
    
    if (i->second.isArray())
    {
      SAGA_ADAPTOR_THROW ("Attribute " + key + " is a vector attribute", saga::IncorrectState);
    }
    
    ret = i->second.getString();
  }

////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_get_vector_attribute (std::vector <std::string> & ret, 
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

////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_set_attribute (saga::impl::void_t & ret, 
                                                  std::string    key, 
                                                  std::string    val)
  {
    bool state = _connection->get_state(_path.string()); 
    check_if_open(state, "advertdirectory_cpi_impl::sync_set_attribute");
    
    _connection->set_attribute(_path.string(), key, val);
  }

////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_set_vector_attribute (saga::impl::void_t       & ret, 
                                                         std::string                 key, 
                                                         std::vector <std::string>   val)
  {
    bool state = _connection->get_state(_path.string()); 
    check_if_open(state, "advertdirectory_cpi_impl::sync_set_vector_attribute");
    
    _connection->set_vector_attribute(_path.string(), key, val);
  }

////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_remove_attribute (saga::impl::void_t & ret, 
                                                     std::string    key)
  {
    bool state = _connection->get_state(_path.string()); 
    check_if_open(state, "advertdirectory_cpi_impl::sync_remove_attribute");
    
    _connection->remove_attribute(_path.string(), key);
  }

////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_list_attributes (std::vector <std::string> & ret)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_list_attributes");
    
    JsonBox::Object obj         = value.getObject();
    JsonBox::Object attributes  = obj["attributes"].getObject();
    
    for (std::map<std::string, JsonBox::Value>::iterator i = attributes.begin(); i != attributes.end(); ++i)
    {
      ret.push_back(i->first);
    }
  }

////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_find_attributes (std::vector <std::string> & ret, 
                                                    std::string                 pattern)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

// ===========================================================================
// = namespace_entry functions                                               =
// ===========================================================================

  void 
    advertdirectory_cpi_impl::sync_get_url (saga::url & url)
  {
     bool state = _connection->get_state(_path.string()); 
     check_if_open(state, "advertdirectory_cpi_impl::sync_get_url");
     
     instance_data idata(this);
     url = idata->location_.clone();
  }

  void 
    advertdirectory_cpi_impl::sync_get_cwd (saga::url & url)
  {
    bool state = _connection->get_state(_path.string()); 
    check_if_open(state, "advertdirectory_cpi_impl::sync_get_url");
    
    url.set_path(_path.parent_path().string());
  }

  void 
    advertdirectory_cpi_impl::sync_get_name (saga::url & url)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_get_name");
    
    JsonBox::Object obj = value.getObject();
    url.set_path(obj["name"].getString());
  }

  void 
    advertdirectory_cpi_impl::sync_is_dir (bool & ret)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_is_dir");
    
    JsonBox::Object obj = value.getObject();
    ret = obj["dir"].getBoolean();
  }

  void 
    advertdirectory_cpi_impl::sync_is_entry (bool & ret)
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_is_entry");
    
    JsonBox::Object obj = value.getObject();
    ret = !(obj["dir"].getBoolean());
  }

  void 
    advertdirectory_cpi_impl::sync_is_link (bool & ret)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_read_link (saga::url & url)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                         saga::url      target, 
                                         int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                         saga::url      target, 
                                         int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                         saga::url      target, 
                                         int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                           int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                          double         timeout)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  ////////////////////////////////////////////////////////////////////////
  //  namespace_dir functions
  ////////////////////////////////////////////////////////////////////////

  void advertdirectory_cpi_impl::sync_list
  (
    std::vector <saga::url> &ret, 
    std::string             pattern, 
    int                     flags
  )
  
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_list");
    
    JsonBox::Object obj     = value.getObject();
    JsonBox::Array nodes    = obj["nodes"].getArray();
    
    for (std::deque<JsonBox::Value>::iterator i = nodes.begin(); i != nodes.end(); ++i)
    {
      JsonBox::Object node = i->getObject();
      ret.push_back(node["name"].getString());
    }   
  }


  void 
    advertdirectory_cpi_impl::sync_find (std::vector <saga::url> & ret, 
                                         std::string               pattern, 
                                         int                       flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_exists (bool      & ret, 
                                           saga::url   entry)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advertdirectory_cpi_impl::sync_is_dir
  (
    bool        &ret, 
    saga::url   entry
  )
  
  {
    JsonBox::Value value;
    bool state = _connection->get_value(_path.string(), value);
    
    check_if_open(state, "advertdirectory_cpi_impl::sync_list");
    
    JsonBox::Object obj     = value.getObject();
    JsonBox::Array nodes    = obj["nodes"].getArray();
    
    bool exists = false;
    for (std::deque<JsonBox::Value>::iterator i = nodes.begin(); i != nodes.end(); ++i)
    {
      JsonBox::Object node = i->getObject();
      
      if (node["name"].getString() == entry.get_string())
      {
        exists = true;
        ret = node["dir"].getBoolean();
      }
    }
    
    if (!exists)
    {
      SAGA_ADAPTOR_THROW ("Entry " + entry.get_string() + " doses not exist", saga::DoesNotExist);
    }
  }


  void 
    advertdirectory_cpi_impl::sync_is_entry (bool      & ret, 
                                             saga::url   entry)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_is_link (bool      & ret, 
                                            saga::url   target)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_read_link (saga::url & url, 
                                              saga::url   target)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_get_num_entries (std::size_t  & num_entries)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_get_entry (saga::url    & entry, 
                                              std::size_t    idx)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                         saga::url      source,
                                         saga::url      target, 
                                         int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                         saga::url      source,
                                         saga::url      target, 
                                         int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                         saga::url      source,
                                         saga::url      target, 
                                         int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                           saga::url      entry, 
                                           int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_open (saga::name_space::entry & ret, 
                                         saga::url                 entry, 
                                         int                       flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_open_dir (saga::name_space::directory & ret, 
                                             saga::url                     entry,
                                             int                           flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
                                               saga::url      dir)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_make_dir (saga::impl::void_t & ret, 
                                             saga::url      dir, 
                                             int            flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


//  ////////////////////////////////////////////////////////////////////////
//  //  directory functions
  void 
    advertdirectory_cpi_impl::sync_open (saga::advert::entry & ret, 
                                         saga::url             entry, 
                                         int                   flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_open_dir (saga::advert::directory & ret, 
                                             saga::url                 entry, 
                                             int                       flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_find (std::vector<saga::url>  & ret, 
                                         std::string               pattern, 
                                         std::vector <std::string> patterns, 
                                         int                       flags)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace sql_async_advert
////////////////////////////////////////////////////////////////////////

