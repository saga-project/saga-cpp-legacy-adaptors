//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_TEST_HELPER_UTILS_HPP
#define ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_TEST_HELPER_UTILS_HPP

#include <test/cpp/unit_tests/interface/test_helper_utils.hpp>

class test_helper_utils_impl : public saga::test::test_helper_utils
{
  typedef boost::recursive_mutex mutex_type;
  mutable mutex_type mtx_;

public:   
  test_helper_utils_impl() {}
  
  ~test_helper_utils_impl() {} 
  
  saga::url create_temp_file_name (bool create = true);
  saga::url create_temp_dir_name  (bool create = true);
  
  saga::url create_temp_file_for_exception( const saga::error & e );
  saga::url create_temp_dir_for_exception( const saga::error & e );
 
  void delete_temp_file (saga::url path);
  void delete_temp_dir  (saga::url path);
};


#endif //ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_TEST_HELPER_UTILS_HPP

