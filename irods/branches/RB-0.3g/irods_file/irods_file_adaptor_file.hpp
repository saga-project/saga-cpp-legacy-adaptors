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


//  Copyright (c) 2008 Andre Merzky <andre@merzky.net>
//
//  Distributed under the Boost Software License,
//  Version 1.0. (See accompanying LICENSE file
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_IRODS_FILE_ADAPTOR_FILE_HPP
#define ADAPTORS_IRODS_FILE_ADAPTOR_FILE_HPP

#include <saga/saga/adaptors/adaptor.hpp>
//#include <saga/impl/packages/file/file_cpi.hpp>	// SAGA < 1.5
#include <saga/impl/packages/filesystem/file_cpi.hpp>
#include <boost/system/system_error.hpp>

#include "irods_file_adaptor.hpp"

#include "irods_api_file.hpp"
#include "irods_file_adaptor_helper.hpp"
#include "irods_file_adaptor_dir.hpp"

////////////////////////////////////////////////////////////////////////////////
//
namespace irods_file_adaptor
{
  class file_cpi_impl : public saga::adaptors::v1_0::file_cpi<file_cpi_impl>
  {
    private:

      typedef saga::adaptors::v1_0::file_cpi<file_cpi_impl> file_cpi;


      /* instance data */
      typedef saga::adaptors::v1_0::file_cpi_instance_data instance_data_type;

      friend class saga::adaptors::instance_data<instance_data_type>;
      typedef      saga::adaptors::instance_data<instance_data_type> file_instance_data_t;


      /* adaptor data */
      typedef saga::adaptors::adaptor_data <file_adaptor> adaptor_data_t;

      boost::filesystem::path path;
      std::string path_str;
      api::irods_file irdfile;
      api::irods_dir irdsdir;
      rodsEnv myEnv;

      //
      void set_path(saga::url& url);
      //
      void set_path(boost::filesystem::path _path) {
        path = _path;
      }

      //
      void set_path_str(saga::url& url);
      //
      void set_path_str(std::string _path_str) {
        path_str = _path_str;
      }

      bool is_opened;

      // throws : IncorrectState
      void check_state() {
        if (!is_opened) {
            SAGA_ADAPTOR_THROW ("this file is not opened.", saga::IncorrectState);
        }
      }

      typedef struct {
        saga::off_t offset;
        int seek_mode;
      } irdSeek_t;

      irdSeek_t irdseek;

    public:

      /*! constructor for a file */
      file_cpi_impl (proxy                * p,
                     cpi_info       const & info,
                     saga::ini::ini const & glob_ini,
                     saga::ini::ini const & adap_ini,
                     boost::shared_ptr <saga::adaptor> adaptor);

      /*! destructor of the file adaptor */
      ~file_cpi_impl (void);


      ///////////////////////////////////////////////////////////////////////////
      ///////////////////////// NAMESPACE::ENTRY METHODS ////////////////////////
      ///////////////////////////////////////////////////////////////////////////

      void sync_get_url   (saga::url                   & url);
      void sync_get_cwd   (saga::url                   & cwd);
      void sync_get_name  (saga::url                   & name);

      void sync_is_dir    (bool                        & is_dir);
      void sync_is_entry  (bool                        & is_file);
      void sync_is_link   (bool                        & is_link);
      void sync_read_link (saga::url                   & target);

      void sync_copy      (saga::impl::void_t          & ret,
                           saga::url                     target,
                           int                           flags = saga::filesystem::None);
      void sync_link      (saga::impl::void_t          & ret,
                           saga::url                     dest,
                           int                           flags = saga::filesystem::None);
      void sync_move      (saga::impl::void_t          & ret,
                           saga::url                     dest,
                           int                           flags = saga::filesystem::None);
      void sync_remove    (saga::impl::void_t          & ret,
                           int                           flags = saga::filesystem::None);

      void sync_close     (saga::impl::void_t          & ret,
                           double                        timeout = 0.0);

      ///////////////////////////////////////////////////////////////////////////
      ////////////////////////// FILESYSTEM::FILE METHODS ///////////////////////
      ///////////////////////////////////////////////////////////////////////////

      void sync_get_size  (saga::off_t                 & size_out);
      void sync_read      (saga::ssize_t               & len_out,
                           saga::mutable_buffer          data,
                           saga::ssize_t                 len_in);
      void sync_write     (saga::ssize_t               & len_out,
                           saga::const_buffer            data,
                           saga::ssize_t                 len_in);
      void sync_seek      (saga::off_t                 & out,
                           saga::off_t                   offset,
                           saga::filesystem::seek_mode   whence);


      ///////////////////////////////////////////////////////////////////////////
      /////////////////////// PERMISSION INTERFACE METHODS //////////////////////
      ///////////////////////////////////////////////////////////////////////////

      void sync_permissions_allow (saga::impl::void_t  & ret,
                                   std::string           id,
                                   int                   perm,
                                   int                   flags);
      void sync_permissions_deny  (saga::impl::void_t & ret,
                                   std::string          id,
                                   int                  perm,
                                   int                  flags);
      void sync_permissions_check (bool               & ret,
                                   std::string          id,
                                   int                  perm);
      void sync_get_owner         (std::string        & out);
      void sync_get_group         (std::string        & out);

  }; // class file_cpi_impl


  //////////////////////////////////////////////////////////////////////
  inline void file_cpi_impl::set_path(saga::url& url) {
    path = boost::filesystem::path(url.get_path());

    if (!path.has_root_directory()) {
      // contains an invalid entry name ?
      SAGA_OSSTREAM out;
      out << "Invalid pathname:\"" << url.get_path() << "\"" << std::endl;
      SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING(out).c_str(),
                          saga::BadParameter);
    }

    std::string leaf = saga::detail::leaf(path);
    if (leaf == "." || leaf == ".." || leaf == "/") {
      SAGA_ADAPTOR_THROW ("url is a directory.", saga::IncorrectURL);
    }
  }

  //////////////////////////////////////////////////////////////////////
  inline void file_cpi_impl::set_path_str(saga::url& url) {
    if (!url.get_url().empty()) {
      helper::check_scheme(url, false);
    }
    set_path(url);
    path_str = path.root_path().string() + path.relative_path().string();
  }

} // namespace irods_file_adaptor

#endif // ADAPTORS_IRODS_FILE_ADAPTOR_FILE_HPP

