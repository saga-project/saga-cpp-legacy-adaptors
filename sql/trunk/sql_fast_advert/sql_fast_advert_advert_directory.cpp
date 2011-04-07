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

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace sql_fast_advert
{
  ////////////////////////////////////////////////////////////////////////
  //  constructor
  advertdirectory_cpi_impl::advertdirectory_cpi_impl (proxy                           * p, 
                                                      cpi_info const                  & info,
                                                      saga::ini::ini const            & glob_ini, 
                                                      saga::ini::ini const            & adap_ini,
                                                      TR1::shared_ptr <saga::adaptor>   adaptor)
    : saga::adaptors::v1_0::advert_directory_cpi <advertdirectory_cpi_impl> (p, info, adaptor, cpi::Noflags) 
  {
  	instance_data idata(this);
	adaptor_data  adata(this);
	
  	saga::url url(idata->location_);

	std::cout << url.get_path() << std::endl;
	
	std::string input = url.get_path();
	std::vector<std::string> result;
	boost::split(result, input, boost::is_any_of("/"));
	
	for(std::vector<std::string>::iterator i = result.begin(); i != result.end(); i++)
	{
		std::cout << *i << std::endl;
	}

  	if (idata->mode_ & saga::advert::Unknown)
	{
		std::cout << "Unkown" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::None)
	{
		std::cout << "None" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Overwrite)
	{
		std::cout << "Overwrite" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Recursive)
	{
		std::cout << "Recursive" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Dereference)
	{
		std::cout << "Dereference" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Create)
	{
		std::cout << "Create" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Exclusive)
	{
		std::cout << "Exclusive" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Lock)
	{
		std::cout << "Lock" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::CreateParents)
	{
		std::cout << "CreateParents" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Read)
	{
		std::cout << "Read" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::Write)
	{
		std::cout << "Write" << std::endl;
	}
	
	if (idata->mode_ & saga::advert::ReadWrite)
	{
		std::cout << "ReadWrite" << std::endl;
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
		dbc = adata->get_database_connection(url);
  	}
  	catch(std::runtime_error e)
  	{
  		SAGA_ADAPTOR_THROW (e.what(), saga::BadParameter);
  	}
  	
  	dir_node = dbc->find_node(url.get_path());

	// Directory not found -> create 
	if (dir_node.id == 0)
	{
		node root_node = dbc->find_node("");
		
		std::cout << "lft : " << root_node.lft << " rgt : " << root_node.rgt << std::endl;
		
		//dir_node = dbc->insert_node(root_node, result[1]);
	}

	//std::cout << result[1] << std::endl;
	std::cout << dir_node.id << std::endl;
	std::cout << dbc->get_path(dir_node) << std::endl;
	
	
	//std::vector<node> node_vector = dbc->get_child_nodes(dir_node);
	//std::cout << node_vector.size() << std::endl;
  	
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  ////////////////////////////////////////////////////////////////////////
  //  destructor
  advertdirectory_cpi_impl::~advertdirectory_cpi_impl (void)
  {
  }


//  ////////////////////////////////////////////////////////////////////////
//  //  SAGA CPI functions 
//
//  ////////////////////////////////////////////////////////////////////////
//  // attribute functions
//  void 
//    advertdirectory_cpi_impl::sync_attribute_exists (bool        & ret, 
//                                                     std::string   key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_attribute_is_readonly (bool        & ret, 
//                                                          std::string   key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_attribute_is_writable (bool        & ret, 
//                                                          std::string   key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_attribute_is_vector (bool        & ret, 
//                                                        std::string   key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_attribute_is_extended (bool        & ret, 
//                                                          std::string   key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  void 
//    advertdirectory_cpi_impl::sync_get_attribute (std::string & ret, 
//                                                  std::string   key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  void 
//    advertdirectory_cpi_impl::sync_get_vector_attribute (std::vector <std::string> & ret, 
//                                                         std::string                 key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  void 
//    advertdirectory_cpi_impl::sync_set_attribute (saga::impl::void_t & ret, 
//                                                  std::string    key, 
//                                                  std::string    val)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  void 
//    advertdirectory_cpi_impl::sync_set_vector_attribute (saga::impl::void_t              & ret, 
//                                                         std::string                 key, 
//                                                         std::vector <std::string>   val)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  void 
//    advertdirectory_cpi_impl::sync_remove_attribute (saga::impl::void_t & ret, 
//                                                     std::string    key)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  void 
//    advertdirectory_cpi_impl::sync_list_attributes (std::vector <std::string> & ret)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  void 
//    advertdirectory_cpi_impl::sync_find_attributes (std::vector <std::string> & ret, 
//                                                    std::string                 pattern)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  ////////////////////////////////////////////////////////////////////////
//  // namespace_entry functions
//  void 
//    advertdirectory_cpi_impl::sync_get_url (saga::url & url)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_get_cwd (saga::url & url)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_get_name (saga::url & url)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
  void 
    advertdirectory_cpi_impl::sync_is_dir (bool & ret)
  {
		ret = true;
		//SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }
//
//  void 
//    advertdirectory_cpi_impl::sync_is_entry (bool & ret)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_is_link (bool & ret)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_read_link (saga::url & url)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_copy (saga::impl::void_t & ret, 
//                                         saga::url      target, 
//                                         int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_link (saga::impl::void_t & ret, 
//                                         saga::url      target, 
//                                         int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_move (saga::impl::void_t & ret, 
//                                         saga::url      target, 
//                                         int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, 
//                                           int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_close (saga::impl::void_t & ret, 
//                                          double         timeout)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//
//  ////////////////////////////////////////////////////////////////////////
//  //  namespace_dir functions
//  ////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_list (std::vector <saga::url> & ret, 
                                         std::string               pattern, 
                                         int                       flags)
	{
		std::vector<node> node_vector = dbc->get_child_nodes(dir_node);
		std::cout << "Pattern : " << pattern << std::endl;
		std::cout << "Vector size : " << node_vector.size() << std::endl;
		
		for (std::vector<node>::iterator i = node_vector.begin(); i != node_vector.end(); i++)
		{
			saga::url url(i->name);
			ret.push_back(url);
		}
		
		
    	//SAGA_ADAPTOR_THROW ("Not Implemented sync_list", saga::NotImplemented);
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
//  void 
//    advertdirectory_cpi_impl::sync_exists (bool      & ret, 
//                                           saga::url   entry)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
  void 
    advertdirectory_cpi_impl::sync_is_dir (bool      & ret, 
                                           saga::url   entry)
  {
	ret = true;
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
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
//  void 
//    advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, 
//                                           saga::url      entry, 
//                                           int            flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_open (saga::name_space::entry & ret, 
//                                         saga::url                 entry, 
//                                         int                       flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_open_dir (saga::name_space::directory & ret, 
//                                             saga::url                     entry,
//                                             int                           flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void 
//    advertdirectory_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
//                                               saga::url      dir)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
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
//  void 
//    advertdirectory_cpi_impl::sync_open_dir (saga::advert::directory & ret, 
//                                             saga::url                 entry, 
//                                             int                       flags)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
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

