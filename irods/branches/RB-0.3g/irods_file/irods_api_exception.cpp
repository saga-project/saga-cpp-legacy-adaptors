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

#include "irods_api_exception.hpp"
#include "irods_file_adaptor_helper.hpp"

namespace irods_file_adaptor { namespace api {

  std::string exception::what() const
  {
    return _msg;
  }

//  std::string func_exception::what() const
//  {
//    return helper::create_error_message(_msg, e);
//  }

  std::ostream& operator<<(std::ostream& s, api::exception& e)
  {
    e.put(s);
    return s;
  }
}}
