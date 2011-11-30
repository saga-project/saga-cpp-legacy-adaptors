/*
 * Copyright (C) 2008-2011 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_iRODS_REPLICA_DIRECTORY_HPP
#define ADAPTORS_iRODS_REPLICA_DIRECTORY_HPP

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


#include "irods_replica_adaptor.hpp"
#include "irods_file_adaptor_dir.hpp"
#include "irods_file_adaptor_file.hpp"
#include "irods_file_adaptor_helper.hpp"

///////////////////////////////////////////////////////////////////////////////
//
namespace irods_replica_adaptor
{
    class logical_directory_cpi_impl
    : public saga::adaptors::v1_0::logical_directory_cpi<logical_directory_cpi_impl>
    {

	private:

		typedef saga::adaptors::v1_0::logical_directory_cpi<logical_directory_cpi_impl>
            base_cpi;

        /// adaptor data //////////////////////////////////////////////////////
        typedef saga::adaptors::adaptor_data
        <irods_replica_adaptor::adaptor> adaptor_data_t;

        void check_permissions(saga::replica::flags flags,
            char const* name, std::string const& lfn);


        /// SIA customization//////////////////////////////////////////////////
        irods_file_adaptor::api::irods_dir  irdsdir;
        irods_file_adaptor::api::irods_file irdfile;
        bool is_opened;
        boost::filesystem::path path;
        std::string path_str;

        void check_state() {
          if (!is_opened) {
            SAGA_ADAPTOR_THROW ("this logical dir is not opened.", saga::IncorrectState);
          }
        }

        void set_path(saga::url& url);
        void set_path(boost::filesystem::path _path) {
          path = _path;
        }
        void set_path_str(saga::url& url);
        void set_path_str(std::string _path_str) {
          path_str = _path_str;
        }
        /// SIA customization end /////////////////////////////////////////////

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

        void sync_exists(bool&, saga::url);
        void sync_is_dir(bool&, saga::url);
        void sync_is_entry(bool&, saga::url);
        void sync_is_link(bool&, saga::url);
        void sync_get_num_entries(std::size_t&);
        void sync_get_entry(saga::url&, std::size_t);

        void sync_remove(saga::impl::void_t&, saga::url, int);
        void sync_make_dir(saga::impl::void_t&, saga::url, int);

        // logical_directory functions
        void sync_is_file(bool&, saga::url);
        void sync_open(saga::replica::logical_file&, saga::url, int);
        void sync_open_dir(saga::replica::logical_directory&, saga::url, int);
        void sync_find(std::vector<saga::url>&, std::string, std::vector<std::string>, int);

    };

///////////////////////////////////////////////////////////////////////////////

    inline void logical_directory_cpi_impl::set_path(saga::url& url) {
      path = boost::filesystem::path(url.get_path());

      if (!path.has_root_directory()) {
        // contains an invalid entry name ?
        SAGA_OSSTREAM out;
        out << "Invalid pathname:\"" << url.get_path() << "\"" << std::endl;
        SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING(out).c_str(),
                            saga::BadParameter);
      }

      std::string leaf = saga::detail::leaf(path);
      if (!(leaf == "." || leaf == "/")) {
        SAGA_ADAPTOR_THROW ("url is not a directory.", saga::BadParameter);
      }
    }

    //////////////////////////////////////////////////////////////////////
    inline void logical_directory_cpi_impl::set_path_str(saga::url& url) {
      if (!url.get_url().empty()) {
    	  irods_file_adaptor::helper::check_scheme(url, false);
      }
      set_path(url);
      path_str = path.branch_path().string(); // remove last '/'.
    }

}   // namespace

#endif

