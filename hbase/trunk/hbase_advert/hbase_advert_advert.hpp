//  Copyright (c) 2005-2008 Hartmut Kaiser 
//  Copyright (c) 2005-2008 Michael Miceli   (mmicel2@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_HBASE_ADVERT_ADVERT_HPP
#define ADAPTORS_HBASE_ADVERT_ADVERT_HPP

// saga includes
#include <saga/saga.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

// advert package includes
#include <saga/impl/packages/advert/advert_cpi.hpp>
#include "Hbase.h"


////////////////////////////////////////////////////////////////////////
namespace hbase_advert
{
  //////////////////////////////////////////////////////////////////////
  //  This adaptor implements the functionality of the Saga API "advert".
  //  It defines the functions declared in its base class, advert_file_cpi.
  class advert_cpi_impl 
    : public saga::adaptors::v1_0::advert_cpi <advert_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::advert_cpi <advert_cpi_impl> base_cpi;
      apache::hadoop::hbase::thrift::HbaseClient *client_;
      boost::shared_ptr<facebook::thrift::transport::TTransport> transport_;
    public:    
      // constructor of the advert adaptor 
      advert_cpi_impl (proxy                           * p, 
                       cpi_info const                  & info, 
                       saga::ini::ini const            & glob_ini, 
                       saga::ini::ini const            & adap_ini,
                       TR1::shared_ptr <saga::adaptor>   adaptor);

      // destructor of the advert adaptor 
      ~advert_cpi_impl (void);

      //////////////////////
      // attribute functions
      void sync_attribute_exists      (bool & ret, std::string key);
      void sync_attribute_is_readonly (bool & ret, std::string key);
      void sync_attribute_is_writable (bool & ret, std::string key);
      void sync_attribute_is_vector   (bool & ret, std::string key);
      void sync_attribute_is_extended (bool & ret, std::string key);

      void sync_get_attribute         (std::string               & ret,
                                       std::string                 key);
      void sync_set_attribute         (saga::impl::void_t        & ret, 
                                       std::string                 key, 
                                       std::string                 val);
      void sync_get_vector_attribute  (std::vector <std::string> & ret, 
                                       std::string                 key);
      void sync_set_vector_attribute  (saga::impl::void_t        & ret, 
                                       std::string                 key, 
                                       std::vector <std::string>   ret);
      void sync_remove_attribute      (saga::impl::void_t        & ret,
                                       std::string                 key);
      void sync_list_attributes       (std::vector <std::string> & ret);
      void sync_find_attributes       (std::vector <std::string> & ret, 
                                       std::string                 pattern);

      ////////////////////////////
      // namespace_entry functions
      void sync_get_url   (saga::url    & ret);
      void sync_get_cwd   (saga::url    & ret);
      void sync_get_name  (saga::url    & ret);

      void sync_read_link (saga::url    & ret);
      void sync_is_dir    (bool         & ret);
      void sync_is_entry  (bool         & ret);
      void sync_is_link   (bool         & ret);
      void sync_copy      (saga::impl::void_t & ret, saga::url target, int flags);
      void sync_link      (saga::impl::void_t & ret, saga::url target, int flags);
      void sync_move      (saga::impl::void_t & ret, saga::url target, int flags);
      void sync_remove    (saga::impl::void_t & ret, int       flags);    
      void sync_close     (saga::impl::void_t & ret, double    timeout);

      // advert functions
      void sync_store_object    (saga::impl::void_t & ret, saga::object  obj);
      void sync_retrieve_object (saga::object       & ret, saga::session s);
      void sync_store_string    (saga::impl::void_t & ret, std::string   str);
      void sync_retrieve_string (std::string        & ret);


      // This adaptor also implements the async functions.

      //////////////////////
      // attribute functions
      saga::task async_attribute_exists      (std::string                 key);
      saga::task async_attribute_is_readonly (std::string                 key);
      saga::task async_attribute_is_writable (std::string                 key);
      saga::task async_attribute_is_vector   (std::string                 key);
      saga::task async_attribute_is_extended (std::string                 key);
      saga::task async_get_attribute         (std::string                 key);
      saga::task async_set_attribute         (std::string                 key, 
                                              std::string                 val);
      saga::task async_get_vector_attribute  (std::string                 key);
      saga::task async_set_vector_attribute  (std::string                 key, 
                                              std::vector <std::string>   val);
      saga::task async_remove_attribute      (std::string                 key);
      saga::task async_list_attributes       (void);
      saga::task async_find_attributes       (std::string                 ret);

      ////////////////////////////
      // namespace_entry functions
      saga::task async_get_url               (void);
      saga::task async_get_cwd               (void);
      saga::task async_get_name              (void);
      saga::task async_read_link             (void);
      saga::task async_is_dir                (void);
      saga::task async_is_entry              (void);
      saga::task async_is_link               (void);
      saga::task async_copy                  (saga::url      target,
                                              int            flags);
      saga::task async_link                  (saga::url      target,
                                              int            flags);
      saga::task async_move                  (saga::url      target,
                                              int            flags);
      saga::task async_remove                (int            flags);
      saga::task async_close                 (double         timeout);

      ///////////////////
      // advert functions
      saga::task async_store_object          (saga::object   obj);
      saga::task async_retrieve_object       (saga::session  s);
      saga::task async_store_string          (std::string    str);
      saga::task async_retrieve_string       (void);

  }; // class advert_cpi_impl

} // namespace hbase_advert
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_HBASE_ADVERT_ADVERT_HPP

