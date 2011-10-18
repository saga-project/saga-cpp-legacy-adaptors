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

#ifndef TORQUE_CLI_STAGING_HPP
#define TORQUE_CLI_STAGING_HPP

#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>

#include "staging.hpp"

namespace pbspro_job { namespace cli {

  //////////////////////////////////////////////////////////////////////
  //
  class file_transfer_parser_impl : public file_transfer_parser {

    //boost::shared_ptr<file_transfer_factory> factory;

    bool check_filename(std::string name) const;

  public:
    //
    //file_transfer_parser_impl(std::string localhost);
    file_transfer_parser_impl() {};
    //
    DESTRUCTOR(file_transfer_parser_impl);
    //
    file_transfer_ptr parse(std::string spec) const;
  };

  //////////////////////////////////////////////////////////////////////
  //
  class staging_path_builder {
    std::string remotehost;
    boost::filesystem::path home;
    boost::filesystem::path workdir;

  protected:
    //
    std::string resolve_local(const std::string& path);
    std::string resolve_remote(const std::string& path);
    //
    std::string get_cwd();

  public:
    staging_path_builder(std::string localhost) : remotehost(localhost) {
      home = std::getenv("HOME");
    }
    //
    DESTRUCTOR(staging_path_builder);
    //
    void set_workdir(std::string path) {
      boost::filesystem::path _path(path);

      if (!_path.empty()) {
	if (_path.has_root_directory()) {
	  workdir = _path;
	} else {
	  workdir = home / _path;
	  workdir.normalize();
	}
      } else {
	workdir = home;
      }
    }

    //
    std::string get_workdir() {
      return workdir.string();
    }
    //
    std::string get_stagein_source(transfer t) {
      // TODO t.first != URL
      return resolve_remote(t.first);
    }
    //
    std::string get_stagein_target(transfer t) {
      // TODO t.second != URL
      return resolve_local(t.second);
    }
    //
    std::string get_stageout_source(transfer t) {
      // TODO t.first != URL
      return resolve_local(t.first);
    }
    //
    std::string get_stageout_target(transfer t) {
      // TODO t.second != URL
      return resolve_remote(t.second);
    }
  };

}}

#endif // TORQUE_CLI_STAGING_HPP
