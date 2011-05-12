//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)
 

// saga includes
#include <saga/saga.hpp>

// adaptor includes
#include "sql_fast_advert_advert.hpp"


////////////////////////////////////////////////////////////////////////
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
  advert_cpi_impl::advert_cpi_impl	(
										proxy                           	*p, 
                                    	cpi_info const                  	&info,
                                    	saga::ini::ini const           		&glob_ini, 
                                    	saga::ini::ini const          	  	&adap_ini,
                                    	TR1::shared_ptr <saga::adaptor>		adaptor
									)
    : saga::adaptors::v1_0::advert_cpi <advert_cpi_impl> (p, info, adaptor, cpi::Noflags)
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
	boost::filesystem::path path = normalize_boost_path(boost::filesystem::path(url.get_path()));
	
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
				dir_node = dbc->insert_node(parent_node, (*(--path.end())).string(), false);
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
	
	// 
	// The node must be a Entry
	//

	if (dir_node.dir == "t")
	{
		SAGA_ADAPTOR_THROW ("This node is not an entry", saga::IncorrectURL);
	}
	
    //SAGA_ADAPTOR_THROW ("Not Implemented advert.cpp", saga::NotImplemented);
  }


  ////////////////////////////////////////////////////////////////////////
  //  destructor
 	advert_cpi_impl::~advert_cpi_impl (void)
  	{
  	}

  ////////////////////////////////////////////////////////////////////////
  //  Create Parents

	void advert_cpi_impl::create_parents(boost::filesystem::path path)
	{
		node parent_node = dbc->find_node((path.parent_path()).string());
		
		// 
		// The parent node must be a Directory
		//
		if (parent_node.dir == "f")
		{
			std::cout << " Dir : " << parent_node.dir << std::endl;
			SAGA_ADAPTOR_THROW ((parent_node.name + " is not a directoy"), saga::IncorrectURL);
		}
		
		if (parent_node.id == 0)
		{
			create_parents(path.parent_path());
			
			parent_node = dbc->find_node((path.parent_path()).string());
			dir_node = dbc->insert_node(parent_node, (*(--path.end())).string());
		}
		
		else
		{
			dir_node = dbc->insert_node(parent_node, (*(--path.end())).string(), false);
		}
	}

  	////////////////////////////////////////////////////////////////////////
  	//  SAGA CPI functions 
	////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////
 	// attribute functions
	////////////////////////////////////////////////////////////////////////

	void advert_cpi_impl::sync_attribute_exists (bool &ret, std::string key)
	{
		LOG
		ret = dbc->attribute_exists(dir_node, key);
  	}

 	void advert_cpi_impl::sync_attribute_is_readonly (bool &ret, std::string key)
  	{
		instance_data idata(this);
		ret = !(idata->mode_ & saga::advert::Read);
  	}

  	void advert_cpi_impl::sync_attribute_is_writable (bool &ret, std::string key)
  	{
		instance_data idata(this);
		ret = (idata->mode_ & saga::advert::Write);
  	}

  	void advert_cpi_impl::sync_attribute_is_vector (bool &ret, std::string key)
  	{
		ret = dbc->attribute_is_vector(dir_node, key);
  	}

  	void advert_cpi_impl::sync_attribute_is_extended (bool &ret, std::string key)
  	{
    	instance_data idata(this);
		ret = (idata->mode_ & saga::advert::Write);
  	}

  	////////////////////////////////////////////////////////////////////////
  	void advert_cpi_impl::sync_get_attribute (std::string &ret, std::string key)
  	{
    	ret = dbc->get_attribute(dir_node, key);
  	}

  	////////////////////////////////////////////////////////////////////////
  	void advert_cpi_impl::sync_get_vector_attribute (std::vector <std::string> &ret, std::string key)
  	{
		dbc->get_vector_attribute(dir_node, ret, key);
  	}

  	////////////////////////////////////////////////////////////////////////
  	void advert_cpi_impl::sync_set_attribute (saga::impl::void_t &ret, std::string key, std::string val)
	{
		LOG
   		dbc->set_attribute(dir_node, key, val);
	}

  	////////////////////////////////////////////////////////////////////////
  	void advert_cpi_impl::sync_set_vector_attribute (saga::impl::void_t &ret, std::string key, std::vector <std::string> val)
  	{
		dbc->set_vector_attribute(dir_node, key, val);
  	}

  	////////////////////////////////////////////////////////////////////////
  	void advert_cpi_impl::sync_remove_attribute (saga::impl::void_t &ret, std::string key)
  	{
		dbc->remove_attribute(dir_node, key);
  	}

  	////////////////////////////////////////////////////////////////////////
  	void advert_cpi_impl::sync_list_attributes (std::vector <std::string> &ret)
  	{
		dbc->list_attributes(ret, dir_node);
  	}

  	////////////////////////////////////////////////////////////////////////
  	void advert_cpi_impl::sync_find_attributes (std::vector<std::string> &ret, std::string pattern)
  	{
    	SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  	}

	////////////////////////////////////////////////////////////////////////
	// namespace_entry functions
	////////////////////////////////////////////////////////////////////////
	
	void advert_cpi_impl::sync_get_url (saga::url & url)
	{
		instance_data idata(this);
		url = idata->location_.clone();
	}

	void advert_cpi_impl::sync_get_cwd(saga::url& url)
  	{
    	instance_data idata(this);
		url = idata->location_.clone();
  	}

  	void advert_cpi_impl::sync_get_name (saga::url & url)
  	{
		url.set_path(dir_node.name);
  	}

 	void advert_cpi_impl::sync_read_link (saga::url & url)
  	{
    	SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  	}

 	void advert_cpi_impl::sync_is_dir (bool & ret)
  	{
    	if (dir_node.dir == "t")
		{
			ret = true;
		}
		
		else
		{
			ret = false;
		}
  	}

 	void advert_cpi_impl::sync_is_entry (bool & ret)
  	{
		if (dir_node.dir == "f")
		{
			ret = true;
		}
		
		else 
		{
			ret = false;
		}
  	}

	void advert_cpi_impl::sync_is_link (bool & ret)
  	{
    	SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  	}

  	void advert_cpi_impl::sync_copy (saga::impl::void_t & ret, saga::url target, int flags)
  	{
    	SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  	}

  	void advert_cpi_impl::sync_link (saga::impl::void_t & ret, saga::url target, int flags)
  	{
    	SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  	}

  	void advert_cpi_impl::sync_move (saga::impl::void_t & ret, saga::url target, int flags)
  	{
    	SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  	}

  	void advert_cpi_impl::sync_remove (saga::impl::void_t & ret, int flags)
  	{
		dbc->remove_node(dir_node);
  	}

 	void advert_cpi_impl::sync_close (saga::impl::void_t & ret, double timeout)
  	{
    	SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  	}

//  ////////////////////////////////////////////////////////////////////////
//  // advert functions
//  void advert_cpi_impl::sync_store_object (saga::impl::void_t & ret, 
//                                           saga::object         obj)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void advert_cpi_impl::sync_retrieve_object (saga::object & ret, 
//                                              saga::session  s)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void advert_cpi_impl::sync_store_string (saga::impl::void_t & ret, 
//                                           std::string          str)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }
//
//  void advert_cpi_impl::sync_retrieve_string (std::string & ret)
//  {
//    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
//  }

} // namespace sql_fast_advert
//////////////////////////////////////////////////////////////////////////

