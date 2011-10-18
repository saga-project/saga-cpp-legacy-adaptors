/*
 * Copyright (C) 2008-2009 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2009 National Institute of Informatics in Japan.
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

#ifndef STAGING_HPP
#define STAGING_HPP

#include <map>
#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>

#include "debug.hpp"

namespace torque_job { namespace cli {

  typedef std::pair<std::string, std::string> transfer;
  typedef boost::shared_ptr<transfer> transfer_ptr;

  class file_transfer;
  typedef boost::shared_ptr<file_transfer> file_transfer_ptr;
  class file_transfer_parser;
  typedef boost::shared_ptr<file_transfer_parser> file_transfer_parser_ptr;

  //////////////////////////////////////////////////////////////////////
  //
  class file_transfer {

  public:
    enum staging { in, out };
    enum creation { overwrite, append };

  public:
    //
    VDESTRUCTOR(file_transfer);
    //
    virtual staging get_type() const = 0;
    //
    virtual transfer_ptr get_transfer() const = 0;
    //
    virtual std::string get_source() const = 0;
    //
    virtual std::string get_target() const = 0;
  };

  //////////////////////////////////////////////////////////////////////
  //
  class file_transfer_parser {
  public:
    VDESTRUCTOR(file_transfer_parser);
    virtual file_transfer_ptr parse(std::string spec) const = 0;
  };

}}

#endif // STAGING_HPP
