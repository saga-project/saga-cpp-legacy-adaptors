//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_DIR_HPP
#define ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_DIR_HPP

#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/instance_data.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>
#include <saga/saga/adaptors/packages/directory_cpi_instance_data.hpp>
#include <saga/saga/adaptors/packages/namespace_dir_cpi_instance_data.hpp>


#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/packages/filesystem/directory_cpi.hpp>
//#include <saga/impl/packages/file/namespace_dir_cpi.hpp>


#include "globus_gridftp_file_adaptor.hpp"

///////////////////////////////////////////////////////////////////////////////
//
namespace globus_gridftp_file_adaptor 
{
 #if 0
    class namespace_dir_cpi_impl : public saga::adaptors::v1_0::namespace_entry_cpi<namespace_dir_cpi_impl>
    {
    private:
        
        typedef saga::adaptors::v1_0::namespace_entry_cpi<namespace_dir_cpi_impl> namespace_directory_cpi;
        
        /* instance data */
        /////////////////////////////////////////////////////////////////////////////
        typedef saga::adaptors::v1_0::namespace_entry_cpi_instance_data
        instance_data_type;
        
        friend class saga::adaptors::instance_data <instance_data_type>;
        typedef      saga::adaptors::instance_data <instance_data_type> 
        namespace_directory_instance_data_t;
        
        
        void sync_open_dir(saga::name_space::directory & new_dir_instance, 
                                         saga::url name_to_open, int openmode)
        {
            /*namespace_directory_instance_data_t InstanceData(this);
            
            this->check_if_open ("dir_cpi_impl::sync_open_dir", InstanceData->location_);
            
            saga::url url = merge_urls(InstanceData->location_.get_url(), name_to_open);
            
            new_dir_instance = saga::filesystem::directory (this->get_proxy()->get_session(), 
                                                            url.get_url(), 
                                                            openmode);*/
            std::cout << "HELLO" << std::endl;
        };
        
    };*/
    #endif
        
/**
  * This adaptor class implements the functionality of the Saga API "file".
  * It defines the functions declared in its base class, file_cpi.
  *
  * @note some notes
  *
  * @see The documentation of the Saga API 
  */  
class dir_cpi_impl : public saga::adaptors::v1_0::directory_cpi<dir_cpi_impl>
{
private:

  typedef saga::adaptors::v1_0::directory_cpi<dir_cpi_impl> directory_cpi;
  
  /* instance data */
  /////////////////////////////////////////////////////////////////////////////
  typedef saga::adaptors::v1_0::directory_cpi_instance_data
    instance_data_type;
  
  friend class saga::adaptors::instance_data <instance_data_type>;
  typedef      saga::adaptors::instance_data <instance_data_type> 
    directory_instance_data_t;

    bool is_open_;
    
    inline void
    check_if_open (std::string const& functionname, saga::url const& location)
    {
        if (!is_open_)
        {
            SAGA_OSSTREAM strm;
            strm << functionname << ": entry is not in open state: " 
            << location.get_url();
            SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING (strm), saga::IncorrectState);
        }
    }
    
  /* adaptor data */
  /////////////////////////////////////////////////////////////////////////////
  typedef saga::adaptors::adaptor_data
    <globus_gridftp_file_adaptor::file_adaptor> adaptor_data_t;
  
  boost::shared_ptr<dir_cpi_impl> shared_from_this()
  {
    return boost::shared_ptr<dir_cpi_impl>(this->base_type::shared_from_this(),
                                           boost::detail::static_cast_tag());
  }
  
  void sync_init();
  
public:
    
	dir_cpi_impl  (proxy                * p, 
                   cpi_info       const & info,
                   saga::ini::ini const & glob_ini,
                   saga::ini::ini const & adap_ini,
                   boost::shared_ptr<saga::adaptor> adaptor);
  
    ~dir_cpi_impl (void);
  
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////// NAMESPACE::ENTRY METHODS ////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    void sync_get_url		  (saga::url & url); 
	
    void sync_get_cwd		  (saga::url & url); 
	
    void sync_get_name		  (saga::url & url); 
    
    void sync_is_dir		  (bool & is_dir);
	      
    void sync_is_entry		  (bool & is_file);
	   
    void sync_is_link		  (bool & is_link); 
	     
    void sync_read_link		  (saga::url & path); 
    
    void sync_copy			  (saga::impl::void_t & ret, saga::url target, 
							   int flags = saga::filesystem::None);
    void sync_link			  (saga::impl::void_t & ret, saga::url dest, 
							   int flags = saga::filesystem::None);
    void sync_move			  (saga::impl::void_t & ret, saga::url dest, 
							   int flags = saga::filesystem::None);
    void sync_remove		  (saga::impl::void_t & ret,
							   int flags = saga::filesystem::None);  
	void sync_close			  (saga::impl::void_t & ret, double timeout = 0.0); 

    ///////////////////////////////////////////////////////////////////////////
    /////////////////////// NAMESPACE::DIRECTORY METHODS //////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    void sync_change_dir      (saga::impl::void_t &, saga::url new_dir);
	
    void sync_list            (std::vector <saga::url> & list, 
						       std::string pattern, int flags);
    void sync_find            (std::vector <saga::url> & list, 
						       std::string entry, int flags );  
    void sync_exists		  (bool & exists, saga::url url);
	
    void sync_is_dir		  (bool & is_dir, saga::url url);
	
    void sync_is_entry		  (bool & is_file, saga::url url);
	
    void sync_is_link		  (bool & is_link, saga::url url);
	
    void sync_read_link		  (saga::url & ret, saga::url source); 
    
    void sync_get_num_entries (std::size_t & num);
	
    void sync_get_entry       (saga::url & ret, std::size_t entry );

    void sync_copy			  (saga::impl::void_t & ret, saga::url source, 
							   saga::url destination, int flags);                
    void sync_link		 	  (saga::impl::void_t & ret, saga::url source, 
							   saga::url url, int flags);
    void sync_move			  (saga::impl::void_t & ret, saga::url source, 
							   saga::url destination, int flags);
    void sync_remove		  (saga::impl::void_t & ret, saga::url url, int flags);

    void sync_copy_wildcard   (saga::impl::void_t & ret, std::string source, 
							   saga::url destination, int flags);                
    void sync_link_wildcard   (saga::impl::void_t & ret, std::string source, 
							   saga::url url, int flags);
    void sync_move_wildcard   (saga::impl::void_t & ret, std::string source, 
							   saga::url destination, int flags);
    void sync_remove_wildcard (saga::impl::void_t & ret, std::string url, int flags);
    
    void sync_make_dir        (saga::impl::void_t & ret, saga::url url, int flags);
	
    //void sync_open()          // overloaded in FILESYSTEM::DIRECTORY
	
    //void sync_open_dir()      // overloaded in FILESYSTEM::DIRECTORY
    
    ///////////////////////////////////////////////////////////////////////////
    ////////////////////// FILESYSTEM::DIRECTORY METHODS //////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    void sync_get_size         (saga::off_t&, saga::url name, int flags); 
	
    void sync_open			   (saga::filesystem::file& entry, 
								saga::url name_to_open, int openmode);
    void sync_open_dir		   (saga::filesystem::directory & entry, 
								saga::url name_to_open, int openmode);
    void sync_is_file		   (bool & is_file, saga::url name); 
    
    ///////////////////////////////////////////////////////////////////////////
    /////////////////////// PERMISSION INTERFACE METHODS //////////////////////
    ///////////////////////////////////////////////////////////////////////////
    
    void sync_permissions_allow (saga::impl::void_t & ret,
                                 saga::url tgt, 
                                 std::string id,
                                 int perm, 
                                 int flags);
    
    void sync_permissions_deny  (saga::impl::void_t & ret,
                                 saga::url tgt, 
                                 std::string id,
                                 int perm, 
                                 int flags);
    
    void sync_permissions_check (bool & ret,
                                 std::string id,
                                 int perm);
    
    void sync_get_owner         (std::string& out);
    
    
    void sync_get_group         (std::string& out); 
    
    
  //FIXME: get_size() method missing
  
};  // class dir_cpi_impl

}   // namespace globus_gridftp_file_adaptor

#endif // ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_DIR_HPP
