//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <fstream>

#include <boost/assert.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <saga/url.hpp>
#include <saga/exception.hpp>
#include <saga/adaptors/task.hpp>

#include <saga/impl/config.hpp>

#include "curl_file_adaptor_file.hpp"
#include "curl_file_adaptor_connection.hpp"

using namespace curl_file_adaptor;

