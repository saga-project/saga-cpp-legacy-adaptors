//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/adaptors/adaptor.hpp>

#include "globus_gridftp_file_adaptor_connection.hpp"
#include "globus_gridftp_file_adaptor_test_helper_utils.hpp"

SAGA_TEST_HELPER_REGISTER(test_helper_utils_impl); 

///////////////////////////////////////////////////////////////////////////////
//
void test_helper_utils_impl::delete_temp_file (saga::url path)
{
    mutex_type::scoped_lock lock(mtx_);
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
void test_helper_utils_impl::delete_temp_dir  (saga::url path)
{
    mutex_type::scoped_lock lock(mtx_);
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
saga::url
test_helper_utils_impl::create_temp_file_for_exception( const saga::error & e )
{
  switch (e) {
    
    case((saga::error)saga::adaptors::Success):
      // create a file and return the url
      break;
      
    case(saga::DoesNotExist):
      // return url of a non-existing file
      break;
      
    case(saga::PermissionDenied):
      // create a file with insufficent permissions and return the url
      break;
      
    default:
      break;
  }
  
  return std::string("");
}

///////////////////////////////////////////////////////////////////////////////
//
saga::url
test_helper_utils_impl::create_temp_dir_for_exception( const saga::error & e )
{
    switch (e) {
    
    case((saga::error)saga::adaptors::Success):
      // create a file and return the url
      break;
      
    case(saga::DoesNotExist):
      // return url of a non-existing file
      break;
      
    case(saga::PermissionDenied):
      // create a file with insufficent permissions and return the url
      break;
      
    default:
      break;
    }
    
    return std::string("");
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
saga::url
test_helper_utils_impl::create_temp_file_name (bool create)
{
  std::string unique_path = get_unique_path_name();
  saga::url unique_url(unique_path);
  unique_url.set_scheme("gsiftp");
  unique_url.set_host("gridhub.cct.lsu.edu");
  
  if(create) {
    globus_gridftp_file_adaptor::
      GridFTPConnection ggc(unique_url.get_url());
    ggc.write_to_file(unique_url.get_url(),"",0);
  }
  
  return unique_url.get_url();
}
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
saga::url
test_helper_utils_impl::create_temp_dir_name (bool create)
{
  std::string unique_path = get_unique_path_name();
  saga::url unique_url(unique_path);
  unique_url.set_scheme("gsiftp");
  unique_url.set_host("gridhub.cct.lsu.edu"); 

  if(create) {
    globus_gridftp_file_adaptor::
      GridFTPConnection ggc(unique_url.get_url());
    ggc.make_directory(unique_url.get_url());
  }
  
  return unique_url.get_url();
}
//
///////////////////////////////////////////////////////////////////////////////
