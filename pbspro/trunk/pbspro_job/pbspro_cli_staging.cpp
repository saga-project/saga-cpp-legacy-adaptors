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

#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include <saga/impl/exception.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>
#include <saga/saga/url.hpp>

#include "pbspro_cli_staging.hpp"

namespace pbspro_job { namespace cli {

  //////////////////////////////////////////////////////////////////////
  //
  class file_transfer_impl : public file_transfer {

  private:
    transfer_ptr t;
    staging type;
    creation mode;

  public:
    //
    file_transfer_impl(std::string source, std::string target,
		       staging s, creation c = overwrite) :
      t(new transfer(source, target)), type(s), mode(c) {}
    //
    DESTRUCTOR(file_transfer_impl);
    //
    staging get_type() const { return type; }
    //
    transfer_ptr get_transfer() const { return t; }
    //
    std::string get_source() const { return t->first; }
    //
    std::string get_target() const { return t->second; }
  };

  //////////////////////////////////////////////////////////////////////
  //
  bool file_transfer_parser_impl::check_filename(std::string name) const {
    saga::url file(name);

    if (!file.get_scheme().empty() || !file.get_host().empty()) {
      return false;
    }

    SAGA_VERBOSE (SAGA_VERBOSE_LEVEL_DEBUG) 
    {
      std::cout << "path=" << file.get_path() << std::endl;

      boost::filesystem::path l(name);
      std::cout << "l.root_name=" << l.root_name() << std::endl;
      std::cout << "l.root_directory=" << l.root_directory() << std::endl;
      std::cout << "l.relative_path=" << l.relative_path() << std::endl;
    }

    return true;
  }

  //////////////////////////////////////////////////////////////////////
  //
  file_transfer_ptr file_transfer_parser_impl::parse(std::string spec) const {
    std::string left, right;
    saga::adaptors::file_transfer_operator mode;

    if (!parse_file_transfer_specification(spec,
					   left, mode, right)) {
      SAGA_OSSTREAM strm;
      strm << "Parse failed: "
	   << "(FileTransfer entry: '" << spec << "').";
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
				    saga::BadParameter);
      throw;
    }
#if 0
    if (!check_filename(left) || !check_filename(right)) {
      SAGA_OSSTREAM strm;
      strm << "Cannot handle the specified URL "
	   << "(FileTransfer entry: '" << spec << "').";
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
				    saga::IncorrectURL);
    }
#endif

    file_transfer_ptr p;
    switch (mode) {
    case saga::adaptors::copy_local_remote:
      p = file_transfer_ptr(new file_transfer_impl(left, right,
						   file_transfer::in));
      break;
    case saga::adaptors::copy_remote_local:
      p = file_transfer_ptr(new file_transfer_impl(right, left,
						   file_transfer::out));
      break;
    case saga::adaptors::append_local_remote:
    case saga::adaptors::append_remote_local:
      {
	SAGA_OSSTREAM strm;
	strm << "Append operation is unsupported "
	     << "(FileTransfer entry: '" << spec << "').";
	SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
				      saga::NotImplemented);
      }
      break;
    case saga::adaptors::unknown_mode:
    default:
      {
	// from condor
	SAGA_OSSTREAM strm;
	strm << "Unknown FileTransfer operator "
	     << "(FileTransfer entry: '" << spec << "').";
	SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm),
				      saga::BadParameter);
      }
      break;
    }

    return p;
  }

  //////////////////////////////////////////////////////////////////////
  // resolve path for job execution host
  //
  std::string staging_path_builder::resolve_local(const std::string& path)
  {
    boost::filesystem::path _path(path);
#if 0
    if (_path.empty()) {
      // throw
    }
#endif
    if (_path.has_root_directory()) {
      return path;
    }

    boost::filesystem::path x = workdir / _path;
    x.normalize();
    return x.string();
  }

  // resolve path for qsub execution host
  std::string staging_path_builder::resolve_remote(const std::string& path)
  {
    boost::filesystem::path _path(path);

    std::ostringstream os;
    os << remotehost << ":";
#if 0
    if (_path.empty()) {
      // throw
    }
#endif

    if (_path.has_root_directory()) {
      os << path;
    } else {
      boost::filesystem::path x = boost::filesystem::current_path();
      x /= _path;
      x.normalize();
      os << x.string();
    }
    return os.str();
  }

  //
  std::string staging_path_builder::get_cwd()
  {
    boost::filesystem::path cwd = boost::filesystem::current_path();
    return cwd.string();
  }

}}
