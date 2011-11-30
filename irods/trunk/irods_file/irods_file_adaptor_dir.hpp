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

#ifndef ADAPTORS_IRODS_FILE_ADAPTOR_DIR_HPP
#define ADAPTORS_IRODS_FILE_ADAPTOR_DIR_HPP


#include <saga/saga/adaptors/adaptor.hpp>
//#include <saga/impl/packages/file/directory_cpi.hpp>	// SAGA < 1.5
#include <saga/impl/packages/filesystem/directory_cpi.hpp>
#include <boost/system/system_error.hpp>

#include "irods_file_adaptor.hpp"

#include "irods_api_dir.hpp"
#include "irods_file_adaptor_helper.hpp"

///////////////////////////////////////////////////////////////////////////////
//
namespace irods_file_adaptor
{

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
      typedef saga::adaptors::v1_0::directory_cpi_instance_data instance_data_type;

      friend class saga::adaptors::instance_data <instance_data_type>;
      typedef      saga::adaptors::instance_data <instance_data_type> directory_instance_data_t;


      /* adaptor data */
      typedef saga::adaptors::adaptor_data <file_adaptor> adaptor_data_t;


      boost::shared_ptr<dir_cpi_impl> shared_from_this (void)
      {
        return boost::shared_ptr <dir_cpi_impl> (this->base_type::shared_from_this (),
                                                 boost::detail::static_cast_tag ());
      }

      void sync_init (void);


      boost::filesystem::path path;
      std::string path_str;
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
          SAGA_ADAPTOR_THROW ("this dir is not opened.", saga::IncorrectState);
        }
      }


    public:

      dir_cpi_impl  (proxy                * p,
                     cpi_info       const & info,
                     saga::ini::ini const & glob_ini,
                     saga::ini::ini const & adap_ini,
                     boost::shared_ptr <saga::adaptor> adaptor);

      ~dir_cpi_impl (void);

       ///////////////////////////////////////////////////////////////////////////
       ///////////////////////// NAMESPACE::ENTRY METHODS ////////////////////////
       ///////////////////////////////////////////////////////////////////////////

       void sync_get_url		  (saga::url                   & url);
       void sync_get_cwd		  (saga::url                   & url);
       void sync_get_name		(saga::url                   & url);

       void sync_is_dir		  (bool                        & is_dir);
       void sync_is_entry		(bool                        & is_file);
       void sync_is_link		  (bool                        & is_link);
       void sync_read_link		(saga::url                   & path);

       void sync_copy			  (saga::impl::void_t          & ret,
                              saga::url                     target,
                              int                           flags = saga::filesystem::None);
       void sync_link			  (saga::impl::void_t          & ret,
                              saga::url                     dest,
                              int                           flags = saga::filesystem::None);
       void sync_move			  (saga::impl::void_t          & ret,
                              saga::url                     dest,
                              int                           flags = saga::filesystem::None);
       void sync_remove		  (saga::impl::void_t          & ret,
                              int                           flags = saga::filesystem::None);

       void sync_close			  (saga::impl::void_t          & ret,
                              double                        timeout = 0.0);



       ///////////////////////////////////////////////////////////////////////////
       /////////////////////// NAMESPACE::DIRECTORY METHODS //////////////////////
       ///////////////////////////////////////////////////////////////////////////

       void sync_change_dir      (saga::impl::void_t      & ret,
                                  saga::url                 new_dir);
       void sync_list            (std::vector <saga::url> & list,
                                  std::string               pattern,
                                  int                       flags);
       void sync_find            (std::vector <saga::url> & list,
                                  std::string               entry,
                                  int                       flags);

       void sync_exists		      (bool                    & exists,
                                  saga::url                 url);
       void sync_is_dir		      (bool                    & is_dir,
                                  saga::url                 url);
       void sync_is_entry		    (bool                    & is_file,
                                  saga::url                 url);
       void sync_is_link		      (bool                    & is_link,
                                  saga::url                 url);
       void sync_read_link		    (saga::url               & ret,
                                  saga::url                 source);

       void sync_get_num_entries (std::size_t             & num);
       void sync_get_entry       (saga::url               & ret,
                                  std::size_t               entry );

       void sync_copy			      (saga::impl::void_t      & ret,
                                  saga::url                 source,
                                  saga::url                 destination,
                                  int                       flags);
       void sync_link		 	      (saga::impl::void_t      & ret,
                                  saga::url                 source,
                                  saga::url                 url,
                                  int                       flags);
       void sync_move			      (saga::impl::void_t      & ret,
                                  saga::url                 source,
                                  saga::url                 destination,
                                  int                       flags);
       void sync_remove		      (saga::impl::void_t      & ret,
                                  saga::url                 url,
                                  int                       flags);

       void sync_copy_wildcard   (saga::impl::void_t      & ret,
                                  std::string               source,
                                  saga::url                 destination,
                                  int                       flags);
       void sync_link_wildcard   (saga::impl::void_t      & ret,
                                  std::string               source,
                                  saga::url                 url,
                                  int                       flags);
       void sync_move_wildcard   (saga::impl::void_t      & ret,
                                  std::string               source,
                                  saga::url                 destination,
                                  int                       flags);
       void sync_remove_wildcard (saga::impl::void_t      & ret,
                                  std::string               url,
                                  int                       flags);

       void sync_make_dir        (saga::impl::void_t      & ret,
                                  saga::url                 url,
                                  int                       flags);
//
//    // void sync_open            (...); // overloaded in FILESYSTEM::DIRECTORY
//    // void sync_open_dir        (...); // overloaded in FILESYSTEM::DIRECTORY
//
//
       ///////////////////////////////////////////////////////////////////////////
       ////////////////////// FILESYSTEM::DIRECTORY METHODS //////////////////////
       ///////////////////////////////////////////////////////////////////////////

//       void sync_get_size        (saga::off_t             & ret,
//                                  saga::url                 name);
       void sync_open			      (saga::filesystem::file  & entry,
                                  saga::url                 name,
                                  int                       openmode);
       void sync_open_dir		    (saga::filesystem::directory & entry,
                                  saga::url                 name,
                                  int                       openmode);
       void sync_is_file		      (bool                    & is_file,
                                  saga::url                 name);


       ///////////////////////////////////////////////////////////////////////////
       /////////////////////// PERMISSION INTERFACE METHODS //////////////////////
       ///////////////////////////////////////////////////////////////////////////

       void sync_permissions_allow (saga::impl::void_t    & ret,
                                    saga::url               tgt,
                                    std::string             id,
                                    int                     perm,
                                    int                     flags);
       void sync_permissions_deny  (saga::impl::void_t    & ret,
                                    saga::url               tgt,
                                    std::string             id,
                                    int                     perm,
                                    int                     flags);
       void sync_permissions_check (bool                  & ret,
                                    std::string             id,
                                    int                     perm);
       void sync_get_owner         (std::string           & out);
       void sync_get_group         (std::string           & out);


  };  // class dir_cpi_impl

  //////////////////////////////////////////////////////////////////////

  inline void dir_cpi_impl::set_path(saga::url& url) {
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
  inline void dir_cpi_impl::set_path_str(saga::url& url) {
    if (!url.get_url().empty()) {
      helper::check_scheme(url, false);
    }
    set_path(url);
    path_str = path.branch_path().string(); // remove last '/'.
  }

} // namespace irods_file_adaptor

#endif // ADAPTORS_IRODS_FILE_ADAPTOR_DIR_HPP

