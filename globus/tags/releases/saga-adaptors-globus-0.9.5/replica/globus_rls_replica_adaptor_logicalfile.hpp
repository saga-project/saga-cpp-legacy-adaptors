//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_RLS_REPLICA_FILE_HPP
#define ADAPTORS_GLOBUS_RLS_REPLICA_FILE_HPP

#include <fstream>
#include <string>

#include <saga/saga/util.hpp>
#include <saga/saga/types.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/packages/logical_file_cpi_instance_data.hpp>

#include <saga/saga/adaptors/instance_data.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/packages/replica/logical_file_cpi.hpp>

#include "globus_rls_replica_adaptor.hpp"

///////////////////////////////////////////////////////////////////////////////
//
namespace globus_rls_replica_adaptor
{
    class logical_file_cpi_impl 
        : public saga::adaptors::v1_0::logical_file_cpi<logical_file_cpi_impl>
    {
    private:
	
        typedef saga::adaptors::v1_0::logical_file_cpi<logical_file_cpi_impl> 
            base_cpi;

        /// adaptor data //////////////////////////////////////////////////////
        typedef saga::adaptors::adaptor_data
        <globus_rls_replica_adaptor::adaptor> adaptor_data_t;
        
        void check_permissions(saga::replica::flags flags,
                               char const* name, std::string const& lfn);
        
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

    public:
	
        typedef base_cpi::mutex_type mutex_type;
        
        // constructor  
        logical_file_cpi_impl (proxy* p, cpi_info const& info, 
            saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
            TR1::shared_ptr<saga::adaptor> adaptor);

        // destructor  
        ~logical_file_cpi_impl  (void);

        ///////////////////////////////////////////////////////////////////////////
        /////////////////////////// ATTRIBUTE INTERFACE ///////////////////////////
        ///////////////////////////////////////////////////////////////////////////
		
        void sync_attribute_exists      (bool&, std::string key);             
        
		void sync_attribute_is_readonly (bool&, std::string key);                  
        void sync_attribute_is_writable (bool&, std::string key);								
        void sync_attribute_is_vector   (bool&, std::string key);									
		void sync_attribute_is_removable(bool&, std::string key);                 
		
		void sync_get_attribute         (std::string&, std::string key);	
        void sync_set_attribute         (saga::impl::void_t&, std::string key, 
                                         std::string val);			
        
		void sync_get_vector_attribute  (std::vector<std::string>&, std::string key);			
        void sync_set_vector_attribute  (saga::impl::void_t&, std::string,
                                         std::vector<std::string>);	
        
		void sync_remove_attribute      (saga::impl::void_t&, std::string key);						
		
        void sync_list_attributes       (std::vector<std::string>& keys);							
        void sync_find_attributes       (std::vector<std::string>&, std::string);					

        ///////////////////////////////////////////////////////////////////////////
        ///////////////////////// NAMESPACE::ENTRY METHODS ////////////////////////
        ///////////////////////////////////////////////////////////////////////////
        
        void sync_get_url  (saga::url & url); 
        void sync_get_cwd  (saga::url & cwd); 
        void sync_get_name (saga::url & name); 
        
        void sync_is_dir    (bool & is_dir);      
        void sync_is_entry  (bool & is_file);   
        void sync_is_link   (bool & is_link);      
        //void sync_read_link (saga::url & target); 
        
        /*void sync_copy    (saga::impl::void_t & ret,   
                           saga::url target, 
                           int flags = saga::replica::None);
        void sync_link    (saga::impl::void_t & ret,    
                           saga::url dest, 
                           int flags = saga::replica::None);
        void sync_move    (saga::impl::void_t & ret,   
                           saga::url dest, 
                        int flags = saga::replica::None);*/
        void sync_remove  (saga::impl::void_t & ret,
                           int flags = saga::replica::None);  
        
        void sync_close   (saga::impl::void_t & ret, double timeout = 0.0); 

        ///////////////////////////////////////////////////////////////////////////
        ////////////////////////// REPLICA::LOGICAL_FILE //////////////////////////
        ///////////////////////////////////////////////////////////////////////////
		
        void sync_list_locations    (std::vector<saga::url>& locations);
     
        void sync_add_location      (saga::impl::void_t&, saga::url location);	
 
        void sync_remove_location   (saga::impl::void_t&, saga::url location);
      
        void sync_update_location   (saga::impl::void_t&, saga::url oldlocation, 
                                     saga::url newlocation);	
        void sync_replicate         (saga::impl::void_t&, saga::url, int);
        
    };  // class logical_file_cpi_impl

///////////////////////////////////////////////////////////////////////////////
}   // namespace logical_file

#endif 

