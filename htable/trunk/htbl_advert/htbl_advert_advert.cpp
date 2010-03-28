//  Copyright (c) 2008 Hartmut Kaiser 
//  Copyright (c) 2008 Michael Miceli   (mmicel2@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)
 
#include "Common/Compat.h"
#include <cstdio>
#include <iostream>
#include "Common/Error.h"
#include "Common/System.h"
#include "Hypertable/Lib/Client.h"

// saga includes
#include <saga/saga.hpp>

// adaptor includes
#include "htbl_advert_advert.hpp"
#include "common_helpers.hpp"

using namespace Hypertable;

namespace htbl_advert {
  //  constructor
  advert_cpi_impl::advert_cpi_impl (proxy                           * p, 
                                    cpi_info const                  & info,
                                    saga::ini::ini const            & glob_ini, 
                                    saga::ini::ini const            & adap_ini,
                                    TR1::shared_ptr <saga::adaptor>   adaptor)
    : saga::adaptors::v1_0::advert_cpi <advert_cpi_impl> (p, info, adaptor, cpi::Noflags) {
      std::cerr << "Entering constructor" << std::endl;
      saga::url advert_url;
      instance_data data (this);
      advert_url = data->location_.clone();
      std::string path;
      path = advert_url.get_path();
      if (path.empty())
          path = "/"; // root only
      std::string host(advert_url.get_host());
      if (host.empty()) {
         SAGA_OSSTREAM strm;
         strm << "advert::advert_cpi_impl::init: "
                 "cannot handle advert entry name: " 
              << advert_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::IncorrectURL);
      }
      std::string scheme(advert_url.get_scheme());
      if (!scheme.empty() && scheme != "htbl" && scheme != "any") {
         SAGA_OSSTREAM strm;
         strm << "advert::advert_cpi_impl::init: "
                 "cannot handle advert entry name: " << advert_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::IncorrectURL);
      }
      client_ = new Client("/work/mmicel2/hypertable/0.9.0.12");
      saga::advert::flags mode = (saga::advert::flags)data->mode_;
      if (((mode & saga::advert::Create) || (mode & saga::advert::CreateParents)) && 
          (mode & saga::advert::Exclusive)) {
          if(!url_exists(client_, advert_url.get_url())) {
             SAGA_OSSTREAM strm;
             strm << "advert::advert_cpi_impl::init: "
                     "advert already exists: " << advert_url.get_url();
             SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
          }
      }
      else if ((mode & saga::advert::Create) || (mode & saga::advert::CreateParents)) {
          if(!url_exists(client_, advert_url.get_url())) {
             if(!create_url(client_, advert_url.get_url(), false)) {
                // failed to create url
                SAGA_OSSTREAM strm;
                strm << "advert::advert_cpi_impl::init: "
                        "advert couldn't create url: " << advert_url.get_url();
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
             }
          }
         // no need to create this entry twice
         data->mode_ &= ~(saga::advert::Create | saga::advert::CreateParents);
      }
      if(!url_exists(client_, advert_url.get_url())) {
         SAGA_OSSTREAM strm;
         strm << "advert::advert_cpi_impl::init: "
                 "couldn't create advert : " << advert_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
      }
      std::cerr << "Leaving constructor" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   ////////////////////////////////////////////////////////////////////////
   //  destructor
   advert_cpi_impl::~advert_cpi_impl (void) {
   }

  ////////////////////////////////////////////////////////////////////////
  //  SAGA CPI functions 

  ////////////////////////////////////////////////////////////////////////
  // attribute functions
   void advert_cpi_impl::sync_attribute_exists (bool        & ret, 
                                               std::string   key) {
      std::cerr << "Entering sync_attibute_exists" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path = advert_url.get_path();
      std::string last = get_last(advert_url);
      std::string pidstring = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string(get_node_id(client_, last, pidstring));

      ScanSpecBuilder scan_spec_builder;
      TablePtr table_ptr;
      TableScannerPtr scanner_ptr;
      Cell cell;
      table_ptr = client_->open_table("metadata");

      scan_spec_builder.add_column("metakey"); //adds column family
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      try {
         while (scanner_ptr->next(cell)) {
            String rowString = (const char*)cell.row_key;
            String columnFamily = cell.column_family;
            String columnQualifier = cell.column_qualifier;
            if(rowString == last + "-" + node_id_string) {
               if(columnFamily == "metakey" && columnQualifier ==  key) {
                  ret = true;
                  return;
               }
            }
         }
      }
      catch (std::exception &e) {
         std::cerr << "error: " << e.what() << std::endl;
      }
      ret = false;
      std::cerr << "Leaving sync_attribute_exists" << std::endl;
   }

   void advert_cpi_impl::sync_attribute_is_readonly (bool        & ret, 
                                                    std::string   key) {
      std::cerr << "Entering sync_attribute_is_readonly" << std::endl;
      instance_data data (this);
      ret = !(data->mode_ & saga::advert::Read);
      ret = false;
      std::cerr << "Leaving sync_attribute_is_readonly" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void advert_cpi_impl::sync_attribute_is_writable (bool        & ret, 
                                                    std::string   key) {
      std::cerr << "Entering sync_attribute_is_writable" << std::endl;
      instance_data data (this);
      ret = (data->mode_ & saga::advert::Write) ? true : false;
      std::cerr << "Leaving sync_attribute_is_writable" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

  void advert_cpi_impl::sync_attribute_is_vector (bool        & ret, 
                                                  std::string   key) {
      std::cerr << "===========!sync_attribugte_is_vector!===========" << std::endl;
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

  void advert_cpi_impl::sync_attribute_is_extended (bool        & ret, 
                                                    std::string   key)
  {
      std::cerr << "Entering sync_attribute_is_extended" << std::endl;
      instance_data data (this);
      ret = (data->mode_ & saga::advert::Write) ? true : false;
      std::cerr << "Leaving sync_attribute_is_extended" << std::endl;
      //ret = true;
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_get_attribute (std::string & ret, 
                                            std::string   key)
  {
      std::cerr << "Entering sync_get_attribute" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);

      ScanSpecBuilder scan_spec_builder;
      TablePtr table_ptr;
      TableScannerPtr scanner_ptr;
      Cell cell;
      table_ptr = client_->open_table("metadata");

      scan_spec_builder.add_column("metakey"); //adds column family
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      try {
         while (scanner_ptr->next(cell)) {
            String rowString = (const char*)cell.row_key;
            String columnFamily = cell.column_family;
            String columnQualifier = cell.column_qualifier;
            if(rowString == last + "-" + node_id_string) {
               if(columnFamily == "metakey" && columnQualifier == key) {
                  ret = (const char *)cell.value;
                  return;
               }
            }
         }
      }
      catch (std::exception &e) {
         std::cerr << "error: " << e.what() << std::endl;
      }
      std::cerr << "Leaving sync_get_attribute" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_get_vector_attribute (std::vector <std::string> & ret, 
                                                   std::string                 key)
  {
      std::cerr << "=============!sync_get_vector_attribute!===============" << std::endl;
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_set_attribute (saga::impl::void_t & ret, 
                                            std::string    key, 
                                            std::string    val)
  {
      std::cerr << "Entering sync_set_attribute" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);

      TablePtr table_ptr;
      TableMutatorPtr mutator_ptr;
      KeySpec Entrykey;
      table_ptr = client_->open_table("metadata");
      mutator_ptr = table_ptr->create_mutator();
      String rowString(last + "-" + node_id_string);
      Entrykey.row = rowString.c_str();
      Entrykey.row_len = rowString.length();
      Entrykey.column_family = "metakey";
      Entrykey.column_qualifier = key.c_str();
      mutator_ptr->set(Entrykey, val.c_str());
      Entrykey.column_family = "entry";
      Entrykey.column_qualifier = "node_id";
      mutator_ptr->set(Entrykey, node_id_string.c_str());
      mutator_ptr->flush();
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
      std::cerr << "Leaving sync_set_attribute" << std::endl;
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_set_vector_attribute (saga::impl::void_t            & ret, 
                                                   std::string               key, 
                                                   std::vector <std::string> val)
  {
      std::cerr << "=============!sync_set_vector_attribute!===============" << std::endl;
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_remove_attribute (saga::impl::void_t & ret,
                                               std::string    key)
  {
/*      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);

      client_->deleteAll("metadata", last + "-" + node_id_string, "metakey:" + key);*/
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_list_attributes (std::vector <std::string> & ret)
  {
      std::cerr << "Entering sync_list_attributes" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);

      ScanSpecBuilder scan_spec_builder;
      TableScannerPtr scanner_ptr;
      TablePtr table_ptr;
      Cell cell;
      table_ptr = client_->open_table("metadata");

      scan_spec_builder.add_column("metakey"); //adds column family
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      try {
         while (scanner_ptr->next(cell)) {
            String rowString = (const char*)cell.row_key;
            String columnFamily = cell.column_family;
            String columnQualifier = cell.column_qualifier;
            if(rowString == last + "-" + node_id_string) {
               ret.push_back(columnQualifier);
            }
         }
      }
      catch (std::exception &e) {
         std::cerr << "error: " << e.what() << std::endl;
      }
      std::cerr << "Leaving sync_list_attributes" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_find_attributes (std::vector<std::string> & ret, 
                                              std::string                pattern)
  {
      std::cerr << "=============!sync_find_vector_attribute!===============" << std::endl;
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

   // namespace_entry functions
   void advert_cpi_impl::sync_get_url (saga::url & url) {
      std::cerr << "Entering sync_list_attributes" << std::endl;
      instance_data data (this);
      url = data->location_.clone();
      std::cerr << "Leaving sync_list_attributes" << std::endl;
   }

  void advert_cpi_impl::sync_get_cwd(saga::url& url) {
      std::cerr << "Entering sync_get_url" << std::endl;
      saga::url adname;
      instance_data data (this);
      adname = data->location_.clone();
      std::string path(adname.get_path());
      std::string::size_type p = path.find_last_of('/');
      if (p != std::string::npos)
          adname.set_path(path.substr(0, p+1));
      
      url = adname.get_url();
      std::cerr << "Leaving sync_get_url" << std::endl;
  }

  void advert_cpi_impl::sync_get_name (saga::url & url) {
      std::cerr << "Entering sync_get_name" << std::endl;
      std::string adname;
      instance_data data (this);
      saga::url adname_url;
      adname_url = data->location_.clone();
      adname = adname_url.get_string();
      saga::url result;
      std::string::size_type p = adname.find_last_of('/');
      if (p != std::string::npos)
          result = adname.substr(p);
      url = result;
      std::cerr << "Leaving sync_get_name" << std::endl;
  }

  void advert_cpi_impl::sync_read_link (saga::url & url)
  {
    std::cerr << "=============!sync_read_link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_is_dir (bool & ret)
  {
      std::cerr << "Entering sync_is_dir" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);

      ScanSpecBuilder scan_spec_builder;
      TableScannerPtr scanner_ptr;
      TablePtr table_ptr;
      Cell cell;
      table_ptr = client_->open_table("nodes");

      scan_spec_builder.add_column("entry"); //adds column family
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      try {
         while (scanner_ptr->next(cell)) {
            String rowString = (const char*)cell.row_key;
            String columnFamily = cell.column_family;
            String columnQualifier = cell.column_qualifier;

            if(rowString == last + "-" + node_id_string) {
               if(columnFamily == "entry" && columnQualifier == "is_dir") {
                  if((const char *)cell.value == "false") 
                     ret = false;
                  else
                     ret = true;
               }
            }
         }
      }
      catch (std::exception &e) {
         std::cerr << "error: " << e.what() << std::endl;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
      std::cerr << "Leaving sync_is_dir" << std::endl;
  }

  void advert_cpi_impl::sync_is_entry (bool & ret)
  {
      std::cerr << "Entering sync_is_entry" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);

      ScanSpecBuilder scan_spec_builder;
      TableScannerPtr scanner_ptr;
      TablePtr table_ptr;
      Cell cell;
      table_ptr = client_->open_table("nodes");

      scan_spec_builder.add_column("entry"); //adds column family
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      try {
         while (scanner_ptr->next(cell)) {
            String rowString = (const char*)cell.row_key;
            String columnFamily = cell.column_family;
            String columnQualifier = cell.column_qualifier;
            if(rowString == last + "-" + node_id_string) {
               if(columnFamily == "entry" && columnQualifier == "is_dir") {
                  if((const char*)cell.value == "false") 
                     ret = true;
                  else
                     ret = false;
               }
            }
         }
      }
      catch (std::exception &e) {
         std::cerr << "Error: " << e.what() << std::endl;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
      std::cerr << "Leaving sync_is_entry" << std::endl;
  }

  void advert_cpi_impl::sync_is_link (bool & ret)
  {
    std::cerr << "============!is_link!=======" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                   saga::url            target,
                                   int                  flags)
  {
    std::cerr << "============!copy!=======" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                   saga::url            target,
                                   int                  flags)
  {
    std::cerr << "============!sync_link!=======" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                   saga::url            target,
                                   int                  flags)
  {
    std::cerr << "============!sync_move!=======" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                     int                  flags)
  {
      /*instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);
      client_->deleteAll("nodes",    last + "-" + node_id_string, "entry:node_id");
      client_->deleteAll("nodes",    last + "-" + node_id_string, "entry:parent_id");
      client_->deleteAll("nodes",    last + "-" + node_id_string, "entry:node_name");
      client_->deleteAll("nodes",    last + "-" + node_id_string, "entry:is_dir");

      std::vector<std::string> columnNames;
      columnNames.push_back("metakey:");
      int scanner = client_->scannerOpen("metadata", "", columnNames);
      try {
         while(true) {
            TRowResult value;
            client_->scannerGet(value, scanner);
            if(value.row == last + "-" + node_id_string) {
               std::map<std::string,TCell>::const_iterator it;
               for(it = value.columns.begin(); it != value.columns.end(); it++) {
                  client_->deleteAll("metadata", last + "-" + node_id_string,
                    it->first);
               }
            }
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }

      client_->deleteAll("metadata", last + "-" + node_id_string, "entry:node_id");
      client_->deleteAll("metadata", last + "-" + node_id_string, "entry:store_string");*/
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_close (saga::impl::void_t & ret,
                                    double               timeout)
  {
    std::cerr << "============!sync_close!=======" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  ////////////////////////////////////////////////////////////////////////
  // advert functions
  void advert_cpi_impl::sync_store_object (saga::impl::void_t & ret, 
                                           saga::object         obj)
  {
    std::cerr << "============!store_object!=======" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_retrieve_object (saga::object & ret, 
                                              saga::session  s)
  {
      std::cerr << "============!retrieve_object!=======" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_store_string (saga::impl::void_t & ret, 
                                           std::string          str)
  {
      std::cerr << "Entering sync_store_string" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);
   
      TablePtr table_ptr;
      TableMutatorPtr mutator_ptr;
      KeySpec key;
   
      table_ptr = client_->open_table("metadata");
      mutator_ptr = table_ptr->create_mutator();
      String rowString(last + "-" + node_id_string);
      key.row = rowString.c_str();
      key.row_len = rowString.length();
      key.column_family = "entry";
      key.column_qualifier = "store_string";
      mutator_ptr->set(key, str.c_str());
      mutator_ptr->flush();
      std::cerr << "Leaving sync_store_string" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_retrieve_string (std::string & ret)
  {
      std::cerr << "Entering sync_retrieve_string" << std::endl;
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(client_, advert_url);
      std::string node_id_string = get_node_id(client_, last, pidstring);

      std::string store_string("");
     
      ScanSpecBuilder scan_spec_builder;
      TablePtr table_ptr;
      TableScannerPtr scanner_ptr;
      Cell cell;
      table_ptr = client_->open_table("metadata");

      scan_spec_builder.add_column("entry"); //adds column family
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      try {
         while (scanner_ptr->next(cell)) {
            String rowString = (const char*)cell.row_key;
            String columnFamily = cell.column_family;
            String columnQualifier = cell.column_qualifier;
            if(rowString == last + "-" + node_id_string) {
               if(columnFamily == "entry" && columnQualifier == "store_string") {
                  store_string = (const char *)cell.value;
                  ret = store_string;
                  return;
               }
            }
         }
      }
      catch (std::exception &e) {
         std::cerr << "error: " << e.what() << std::endl;
      }
      std::cerr << "Leaving sync_retrieve_string" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace hbase_advert

