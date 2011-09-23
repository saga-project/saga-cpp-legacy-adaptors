//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_RLS_REPLICA_DIRECTORY_HPP
#define ADAPTORS_GLOBUS_RLS_REPLICA_DIRECTORY_HPP

#include <fstream>
#include <string>

#include <saga/saga/util.hpp>
#include <saga/saga/types.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/packages/logical_file_cpi_instance_data.hpp>

#include <saga/saga/adaptors/instance_data.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/packages/replica/logical_directory_cpi.hpp>

#include "globus_rls_replica_adaptor.hpp"

///////////////////////////////////////////////////////////////////////////////
//
namespace globus_rls_replica_adaptor
{
    class logical_directory_cpi_impl 
    : public saga::adaptors::v1_0::logical_directory_cpi<logical_directory_cpi_impl>
    {
 
	private:
    
		typedef saga::adaptors::v1_0::logical_directory_cpi<logical_directory_cpi_impl> 
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
	    
        // constructor of the file adaptor 
        logical_directory_cpi_impl (proxy* p, cpi_info const& info, 
            saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
            TR1::shared_ptr<saga::adaptor> adaptor);

        // destructor of the file adaptor 
        ~logical_directory_cpi_impl  (void);

        // attribute functions
        void sync_attribute_exists(bool&, std::string key);
        void sync_attribute_is_readonly(bool&, std::string key);
        void sync_attribute_is_writable(bool&, std::string key);
        void sync_attribute_is_vector(bool&, std::string key);
        void sync_attribute_is_extended(bool&, std::string key);
  
		void sync_get_attribute(std::string&, std::string key);
        void sync_set_attribute(saga::impl::void_t&, std::string key, std::string val);
        void sync_get_vector_attribute(std::vector<std::string>&, std::string key);
        void sync_set_vector_attribute(saga::impl::void_t&, std::string, std::vector<std::string>);
        
		void sync_remove_attribute(saga::impl::void_t&, std::string key);
        void sync_list_attributes(std::vector<std::string>& keys);
        void sync_find_attributes(std::vector<std::string>&, std::string);

        // namespace_entry functions
        void sync_get_url(saga::url& url);
        void sync_get_cwd(saga::url&);
        void sync_get_name(saga::url&);

        void sync_is_dir(bool&);
        void sync_is_entry(bool&);
        void sync_is_link(bool&);

        void sync_remove(saga::impl::void_t&, int);
        void sync_close(saga::impl::void_t&, double);

        // namespace_dir functions
        void sync_change_dir(saga::impl::void_t&, saga::url);
        void sync_list(std::vector<saga::url>&, std::string, int);
        void sync_find(std::vector<saga::url>&, std::string, int);

        void sync_exists(bool&, saga::url);
        void sync_is_dir(bool&, saga::url);
        void sync_is_entry(bool&, saga::url);
        void sync_is_link(bool&, saga::url);
        void sync_get_num_entries(std::size_t&);
        void sync_get_entry(saga::url&, std::size_t);

        void sync_remove(saga::impl::void_t&, saga::url, int);
        void sync_make_dir(saga::impl::void_t&, saga::url, int);
        void sync_open(saga::name_space::entry&, saga::url, int);
        void sync_open_dir(saga::name_space::directory&, saga::url, int);

        // logical_directory functions
        void sync_is_file(bool&, saga::url);
        void sync_open(saga::replica::logical_file&, saga::url, int);
        void sync_open_dir(saga::replica::logical_directory&, saga::url, int);
        void sync_find(std::vector<saga::url>&, std::string, std::vector<std::string>, int);
        
    };
	
///////////////////////////////////////////////////////////////////////////////
}   // namespace 

#endif 

