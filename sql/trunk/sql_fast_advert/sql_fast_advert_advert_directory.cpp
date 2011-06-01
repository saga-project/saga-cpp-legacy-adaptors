//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>

// adaptor includes
#include "sql_fast_advert_advert_directory.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace sql_fast_advert
{	
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
  advertdirectory_cpi_impl::advertdirectory_cpi_impl (proxy * p, 
    cpi_info const                  & info,
    saga::ini::ini const            & glob_ini, 
    saga::ini::ini const            & adap_ini,
    TR1::shared_ptr <saga::adaptor>   adaptor)
    : saga::adaptors::v1_0::advert_directory_cpi <advertdirectory_cpi_impl> (p, info, adaptor, cpi::Noflags) 
  {
    instance_data idata(this);
    adaptor_data  adata(this);

  //
  // Path definition
  // the path retrieved form a saga::url seems not to be consistent. 
  // So this will be the path structure the sqlfastadvert adaptor will work with.
  // 
  // Examples
  // root : /
  // path : /usr
  // path : /var/www 
  //

    saga::url url(idata->location_);
    path = normalize_boost_path(boost::filesystem::path(url.get_path()));

  //
  // Decode flags
  // Debug purpose only and should be removed later 
  //

    if (idata->mode_ & saga::advert::Unknown)
    {
    //std::cout << "Unkown" << std::endl;
    }

    if (idata->mode_ & saga::advert::None)
    {
    //std::cout << "None" << std::endl;
    }

    if (idata->mode_ & saga::advert::Overwrite)
    {
    //std::cout << "Overwrite" << std::endl;
    }

    if (idata->mode_ & saga::advert::Recursive)
    {
    //std::cout << "Recursive" << std::endl;
    }

    if (idata->mode_ & saga::advert::Dereference)
    {
    //std::cout << "Dereference" << std::endl;
    }

    if (idata->mode_ & saga::advert::Create)
    {
    //std::cout << "Create" << std::endl;
    }

    if (idata->mode_ & saga::advert::Exclusive)
    {
    //std::cout << "Exclusive" << std::endl;
    }

    if (idata->mode_ & saga::advert::Lock)
    {
    //std::cout << "Lock" << std::endl;
    }

    if (idata->mode_ & saga::advert::CreateParents)
    {
    //std::cout << "CreateParents" << std::endl;
    }

    if (idata->mode_ & saga::advert::Read)
    {
    //std::cout << "Read" << std::endl;
    }

    if (idata->mode_ & saga::advert::Write)
    {
    //std::cout << "Write" << std::endl;
    }

    if (idata->mode_ & saga::advert::ReadWrite)
    {
    //std::cout << "ReadWrite" << std::endl;
    }

    //
    // scheme must be sqlfastadvert
    //

    if (url.get_scheme() != "sqlfastadvert")
    {
      SAGA_OSSTREAM strm;
      strm << "cannot handle url scheme: " << url.get_scheme()
        << ":// - this adaptor only supports sqlfastadvert:// schemes.";

      SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined);
    }


  //
  // Try to connect to the database
  //

    try
    {
      dbc = adata->get_database_connection(url, adap_ini);
    }
    catch(std::runtime_error e)
    {
      SAGA_ADAPTOR_THROW (e.what(), saga::BadParameter);
    }

  // 
  //  Try to find the directory node
  // 

    dir_node = dbc->find_node(path.string());

  // 
  // If there is no node we work through the flags 
  //

    if (dir_node.id == 0)
    {
    // 
    // No Create or CreateParents
    //

      if ( !(idata->mode_ & saga::advert::Create) & !(idata->mode_ & saga::advert::CreateParents))
      {
        SAGA_ADAPTOR_THROW ("Directory dose not exist", saga::IncorrectURL);
      }

    //
    // Create but no CreateParents
    //

      if ( (idata->mode_ & saga::advert::Create) & !(idata->mode_ & saga::advert::CreateParents) )
      {
        node parent_node = dbc->find_node((path.parent_path()).string());

        if (parent_node.id == 0)
        {
          SAGA_ADAPTOR_THROW ("Parent directory dose not exist", saga::IncorrectURL);
        }

        else
        {
          #if BOOST_FILESYSTEM_VERSION > 2
            dir_node = dbc->insert_node(parent_node, (*(--path.end())).string());
          #else
            dir_node = dbc->insert_node(parent_node, (*(--path.end())));
          #endif
        }
      }


    //
    // CreateParents
    //

      if ( idata->mode_ & saga::advert::CreateParents )
      {
        create_parents(path);
      }

    }

  }


  ////////////////////////////////////////////////////////////////////////
  //  destructor
  advertdirectory_cpi_impl::~advertdirectory_cpi_impl (void)
  {
  }

  ////////////////////////////////////////////////////////////////////////
  //  Create Parents

  void advertdirectory_cpi_impl::create_parents(boost::filesystem::path path_)
  {
    node parent_node = dbc->find_node((path_.parent_path()).string());

    // 
    // The parent node must be a Directory
    //
    if (parent_node.dir == "f")
    {
      SAGA_ADAPTOR_THROW ((parent_node.name + " is not a directoy "), saga::IncorrectURL);
    }

    if (parent_node.id == 0)
    {
      create_parents(path_.parent_path());

      parent_node = dbc->find_node((path_.parent_path()).string());
      
      #if BOOST_FILESYSTEM_VERSION > 2
        dir_node = dbc->insert_node(parent_node, (*(--path_.end())).string());
      #else
        dir_node = dbc->insert_node(parent_node, (*(--path_.end())));
      #endif

    }

    else
    {
      #if BOOST_FILESYSTEM_VERSION > 2
        dir_node = dbc->insert_node(parent_node, (*(--path_.end())).string());
      #else
        dir_node = dbc->insert_node(parent_node, (*(--path_.end())));
      #endif
    }
  }

//////////////////////////////////////////////////////////////////////////
////  SAGA CPI functions 
//////////////////////////////////////////////////////////////////////////

//
//   attribute functions
//

  void advertdirectory_cpi_impl::sync_attribute_exists (bool &ret, std::string key)
  {
    ret = dbc->attribute_exists(dir_node, key);
  }

  void advertdirectory_cpi_impl::sync_attribute_is_readonly (bool &ret, std::string key)
  {
    instance_data idata(this);
    ret = !(idata->mode_ & saga::advert::Read);
  }

  void advertdirectory_cpi_impl::sync_attribute_is_writable (bool &ret, std::string key)
  {
    instance_data idata(this);
    ret = (idata->mode_ & saga::advert::Write);
  }

  void advertdirectory_cpi_impl::sync_attribute_is_vector (bool &ret, std::string key)
  {
    ret = dbc->attribute_is_vector(dir_node, key);
  }

  void advertdirectory_cpi_impl::sync_attribute_is_extended (bool &ret, std::string key)
  {
    instance_data idata(this);
    ret = (idata->mode_ & saga::advert::Write);
  }

//
// attribute getter, setter 
// 

///////////////////////////////////////////////////////////////////////////////////////////

  void advertdirectory_cpi_impl::sync_get_attribute (std::string &ret, std::string   key)
  {
    ret = dbc->get_attribute(dir_node, key);
  }

  void advertdirectory_cpi_impl::sync_set_attribute (saga::impl::void_t &ret, std::string key, std::string val)
  {
    dbc->set_attribute(dir_node, key, val);
  }

///////////////////////////////////////////////////////////////////////////////////////////

  void advertdirectory_cpi_impl::sync_get_vector_attribute (std::vector <std::string> &ret, std::string key)
  {
    dbc->get_vector_attribute(dir_node, ret, key);
  }

  void advertdirectory_cpi_impl::sync_set_vector_attribute (saga::impl::void_t &ret, std::string key, std::vector <std::string> val)
  {
    dbc->set_vector_attribute(dir_node, key, val);
  }

////////////////////////////////////////////////////////////////////////
  void advertdirectory_cpi_impl::sync_remove_attribute (saga::impl::void_t &ret, std::string key)
  {
    dbc->remove_attribute(dir_node, key);
  }

////////////////////////////////////////////////////////////////////////
  void advertdirectory_cpi_impl::sync_list_attributes (std::vector <std::string> &ret)
  {
    dbc->list_attributes(ret, dir_node);
  }

////////////////////////////////////////////////////////////////////////
  void advertdirectory_cpi_impl::sync_find_attributes (std::vector <std::string> &ret, std::string pattern)
  {
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

////////////////////////////////////////////////////////////////////////
// namespace_entry functions
////////////////////////////////////////////////////////////////////////

    void advertdirectory_cpi_impl::sync_get_url (saga::url & url)
    {
      instance_data idata(this);
      url = idata->location_.clone();
    }

    void advertdirectory_cpi_impl::sync_get_cwd (saga::url & url)
    {
      instance_data idata(this);
      url = idata->location_.clone();

      // Set Path to current working directory
      url.set_path(path.parent_path().string());
    }

    void advertdirectory_cpi_impl::sync_get_name (saga::url & url)
    {
       url.set_path(dir_node.name);
    }

    void advertdirectory_cpi_impl::sync_is_dir (bool & ret)
    {	
       ret = (dir_node.dir == "t") ? true : false;
    }

    void advertdirectory_cpi_impl::sync_is_entry (bool & ret)
    {
      ret = (dir_node.dir == "f") ? true : false;
    }

    void advertdirectory_cpi_impl::sync_is_link (bool & ret)
    {
      // We Don't support Links in the Advert service 
      ret = false;
    }

    void advertdirectory_cpi_impl::sync_read_link (saga::url & url)
    {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    }

    void advertdirectory_cpi_impl::sync_copy (saga::impl::void_t & ret, saga::url target, int flags)
    {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    }

    void advertdirectory_cpi_impl::sync_link (saga::impl::void_t & ret,saga::url target, int flags)
    {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    }

    void advertdirectory_cpi_impl::sync_move (saga::impl::void_t & ret, saga::url target, int flags)
    {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    }

    void advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, int flags)
    {
      dbc->remove_node(dir_node);
    }

    void advertdirectory_cpi_impl::sync_close (saga::impl::void_t & ret, double timeout)
    {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
    }


////////////////////////////////////////////////////////////////////////
//  namespace_dir functions
////////////////////////////////////////////////////////////////////////

    void advertdirectory_cpi_impl::sync_list (std::vector <saga::url> &ret, std::string pattern, int flags)
	  {		
		  node_vector.clear();
		  dbc->get_child_nodes(node_vector, dir_node);
	
		  for (std::vector<node>::iterator i = node_vector.begin(); i != node_vector.end(); i++)
		  { 		
			  ret.push_back(saga::url(i->name));
		  }
    }
  
//
//  void 
//    advertdirectory_cpi_impl::sync_find (std::vector <saga::url> & ret, 
//                                         std::string               pattern, 
//                                         int                       flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
  void 
    advertdirectory_cpi_impl::sync_exists (bool      & ret, 
                                           saga::url   entry)
  {
    
     boost::filesystem::path entry_path = normalize_boost_path(boost::filesystem::path(entry.get_path()));

  	  // =================
      // = Relative path =
      // =================

      if (entry_path.string()[0] != '/')
      {
        entry_path = path / entry_path;
      }

  	  node db_node = dbc->find_node(entry_path.string());

  	//
  	// Check if entry exists 
  	//

  	if (db_node.id == 0)
  	{
      ret = false;
  	}

    else
    {
      ret = true;
    }
    
  }

  void 
    advertdirectory_cpi_impl::sync_is_dir (bool      & ret, 
                                           saga::url   entry)
  {
	for (std::vector<node>::iterator i = node_vector.begin(); i != node_vector.end(); i++)
	{
		if (i->name == entry.get_path())
		{
			if (i->dir == "t")
			{
				ret = true;
			}
			
			if (i->dir == "f")
			{
				ret = false;
			}
		
			break;
		}
	}
  }
//
//  void 
//    advertdirectory_cpi_impl::sync_is_entry (bool      & ret, 
//                                             saga::url   entry)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_is_link (bool      & ret, 
//                                            saga::url   target)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_read_link (saga::url & url, 
//                                              saga::url   target)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_get_num_entries (std::size_t  & num_entries)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_get_entry (saga::url    & entry, 
//                                              std::size_t    idx)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_copy (saga::impl::void_t & ret, 
//                                         saga::url      source,
//                                         saga::url      target, 
//                                         int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_link (saga::impl::void_t & ret, 
//                                         saga::url      source,
//                                         saga::url      target, 
//                                         int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_move (saga::impl::void_t & ret, 
//                                         saga::url      source,
//                                         saga::url      target, 
//                                         int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
  void 
    advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                           saga::url      entry, 
                                           int            flags)
  {
	
	  boost::filesystem::path entry_path = normalize_boost_path(boost::filesystem::path(entry.get_path()));
	
	  // =================
    // = Relative path =
    // =================
    
    if (entry_path.string()[0] != '/')
    {
      entry_path = path / entry_path;
    }

	  node db_node = dbc->find_node(entry_path.string());
	
	//
	// Check if entry exists 
	//
	
	if (db_node.id == 0)
	{
		SAGA_ADAPTOR_THROW ("Directory dose not exist", saga::IncorrectURL);
	}
	
	//
	// Check if somebody tries to remove the root node
	//
	
	if (db_node.id == 1)
	{
		SAGA_ADAPTOR_THROW ("Impossible to remove the root node", saga::IncorrectURL);
	}
	
	//
	// Recursive remove
	//
	
	if (flags & saga::advert::Recursive)
	{
		dbc->remove_node(db_node);
	}
	
	//
	// Leaf remove
	//
	
	else
	{
		if (database_connection::node_is_leaf(db_node))
		{
			dbc->remove_node(db_node);
		}
		
		else
		{
			SAGA_ADAPTOR_THROW ("No recursive flag set", saga::BadParameter);
		}
	}
  }

//
//  void 
//    advertdirectory_cpi_impl::sync_open (saga::name_space::entry & ret, 
//                                         saga::url                 entry, 
//                                         int                       flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//

  void advertdirectory_cpi_impl::sync_open_dir (saga::name_space::directory &ret, saga::url entry, int flags)
  {
    boost::filesystem::path entry_path = normalize_boost_path(boost::filesystem::path(entry.get_path()));
	
	  // =================
    // = Relative path =
    // =================
    
    if (entry_path.string()[0] != '/')
    {
      entry_path = path / entry_path;
    }

    instance_data idata(this);
    saga::url adname(idata->location_);
    
    adname.set_path(entry_path.string());
    ret = saga::advert::directory(this->get_proxy()->get_session(), adname.get_url(), flags);
  }

  void 
    advertdirectory_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
    saga::url      dir)
  {
    instance_data idata(this);
    saga::url url(idata->location_);

    std::string dir_path = dir.get_path();

  //
  // Relative Path
  //

    if (dir_path[0] != '/')
    {
      instance_data idata(this);	
      boost::filesystem::path path = normalize_boost_path(boost::filesystem::path((url.get_path()) + (dir.get_path())));

      dir_node = dbc->find_node(path.string());
      if (dir_node.id == 0)
      {
        SAGA_ADAPTOR_THROW ("Directory not found", saga::IncorrectURL);
      }
      else
      {
        (idata->location_).set_url(path.string());
      }
    }

  //
  // Absolute path 
  //

    else
    {
      boost::filesystem::path path = normalize_boost_path(boost::filesystem::path(dir.get_path()));

      dir_node = dbc->find_node(path.string());
      if(dir_node.id == 0)
      {
        SAGA_ADAPTOR_THROW ("Directory not found", saga::IncorrectURL);
      }
      else
      {
        (idata->location_).set_url(path.string());
      }
    }

  }

//
//  void 
//    advertdirectory_cpi_impl::sync_make_dir (saga::impl::void_t & ret, 
//                                             saga::url      dir, 
//                                             int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//
//  ////////////////////////////////////////////////////////////////////////
//  //  directory functions
//  void 
//    advertdirectory_cpi_impl::sync_open (saga::advert::entry & ret, 
//                                         saga::url             entry, 
//                                         int                   flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//

  void advertdirectory_cpi_impl::sync_open_dir (saga::advert::directory &ret, saga::url entry, int flags)
  {
    sync_open_dir((saga::name_space::directory&)ret, entry, flags);
  }

//  void 
//    advertdirectory_cpi_impl::sync_find (std::vector<saga::url>  & ret, 
//                                         std::string               pattern, 
//                                         std::vector <std::string> patterns, 
//                                         int                       flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }

} // namespace sql_fast_advert
////////////////////////////////////////////////////////////////////////

