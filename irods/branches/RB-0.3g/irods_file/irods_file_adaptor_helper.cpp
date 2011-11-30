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
#include <sstream>

#include <saga/saga/adaptors/adaptor.hpp>

#include "irods_file_adaptor_helper.hpp"

namespace irods_file_adaptor
{

//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  std::string helper::create_error_message(const std::string& msg,
//                                   const std::string& func,
//                                   gfarm_error_t e)
//  {
//    std::ostringstream ost;
//    ost << msg << std::endl;
//    ost << create_error_message(func,e);
//    return ost.str();
//  }
//
//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  std::string helper::create_error_message(const std::string& func,
//                                   irods_error_t e)
//  {
//    std::ostringstream ost;
//    ost << func << " failed error=" << e << ":";
////    ost << gfarm_error_string(e);
//    return ost.str();
//  }

  ///////////////////////////////////////////////////////////////////////////////
  // throws : BadParameter
  void helper::check_scheme(saga::url& rm, bool local_ok)
  {
    std::string scheme(rm.get_scheme());

    if (scheme.empty() || scheme == "file") {
      if (local_ok) {
        SAGA_LOG_DEBUG("local file.");
        return;
      } else {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize file for [" << rm << "]. "
             << "Only any:// and irods:// "
             << "schemes are supported.";
        SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
                       saga::BadParameter);
      }
    }

    //SGFA : irods
    if (!(scheme == "irods" ||
          scheme == "any")) {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize file for [" << rm << "]. "
           << "Only any:// and irods:// "
           << "schemes are supported.";
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
                     saga::BadParameter);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
//  int helper::convert_fs_flags(saga::filesystem::flags flag)
//  {
//    SAGA_VERBOSE(SAGA_VERBOSE_LEVEL_DEBUG) {
//      SAGA_OSSTREAM strm;
//      strm << "helper::convert_fs_flags" << std::endl;
//      strm << "flags:" << std::oct << flag;
//      SAGA_LOG_DEBUG(SAGA_OSSTREAM_GETSTRING(strm).c_str());
//    }
//
//    int gf_flags = 0;
//
//    if (flag & saga::filesystem::Read) {
//      SAGA_LOG_DEBUG("GFARM_FILE_RDONLY set.");
//      gf_flags |= GFARM_FILE_RDONLY;
//    }
//    if (flag & saga::filesystem::Write) {
//      SAGA_LOG_DEBUG("GFARM_FILE_WRONLY set.");
//      gf_flags |= GFARM_FILE_WRONLY;
//    }
//    if (flag & saga::filesystem::Truncate) {
//      SAGA_LOG_DEBUG("GFARM_FILE_TRUNC set.");
//      gf_flags |= GFARM_FILE_TRUNC;
//    }
//    if (flag & saga::filesystem::Create) {
//      SAGA_LOG_DEBUG("GFARM_FILE_WRONLY set.");
//      gf_flags |= GFARM_FILE_WRONLY;
//      SAGA_LOG_DEBUG("GFARM_FILE_TRUNC set.");
//      gf_flags |= GFARM_FILE_TRUNC;
//    }
//#ifdef GFARM_FILE_EXCLUSIVE
//    if (flag & saga::filesystem::Exclusive) {
//      SAGA_LOG_DEBUG("GFARM_FILE_EXCLUSIVE set.");
//      gf_flags |= GFARM_FILE_EXCLUSIVE;
//    }
//#endif // GFARM_FILE_EXCLUSIVE
//    return gf_flags;
//  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  int helper::convert_fs_seek_mode(saga::filesystem::seek_mode whence)
  {
    switch (whence) {
    case saga::filesystem::Start:
      return SEEK_SET;
    case saga::filesystem::Current:
      return SEEK_CUR;
    case saga::filesystem::End:
      return SEEK_END;
    default:
      // TODO
      throw std::exception();
    }
  }
}
