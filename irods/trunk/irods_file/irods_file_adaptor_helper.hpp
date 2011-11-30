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
#ifndef   __iRODS_FILE_ADAPTOR_HELPER_HPP__
#define   __iRODS_FILE_ADAPTOR_HELPER_HPP__

#include <string>

#include <saga/saga/url.hpp>
//#include <saga/saga/file.hpp>	// SAGA < 1.5
#include <saga/saga/filesystem.hpp>

#include "irods_api.hpp"

namespace irods_file_adaptor
{
  namespace helper {

//    std::string create_error_message(const std::string& msg,
//                                     const std::string& func,
//                                     irods_error_t e);
//
//    std::string create_error_message(const std::string& func,
//                                     irods_error_t e);

    void check_scheme(saga::url& rm, bool local_ok=true);

    //
//    int convert_fs_flags(saga::filesystem::flags flag);

    //
    int convert_fs_seek_mode(saga::filesystem::seek_mode whence);
  }
}

#endif // __iRODS_FILE_ADAPTOR_HELPER_HPP__
