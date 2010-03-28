//  Copyright (c) 2005-2008 Hartmut Kaiser 
//  Copyright (c) 2005-2008 Michael Miceli   (mmicel2@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)
 

// saga includes
#include <saga/saga.hpp>

// adaptor includes
#include "hbase_advert_advert.hpp"
#include <protocol/TBinaryProtocol.h>
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>
#include "common_helpers.hpp"
#include "Hbase.h"

using namespace facebook::thrift;
using namespace facebook::thrift::protocol;
using namespace facebook::thrift::transport;
using namespace apache::hadoop::hbase::thrift;

////////////////////////////////////////////////////////////////////////
namespace hbase_advert {
  ////////////////////////////////////////////////////////////////////////
  //  constructor
  advert_cpi_impl::advert_cpi_impl (proxy                           * p, 
                                    cpi_info const                  & info,
                                    saga::ini::ini const            & glob_ini, 
                                    saga::ini::ini const            & adap_ini,
                                    TR1::shared_ptr <saga::adaptor>   adaptor)
    : saga::adaptors::v1_0::advert_cpi <advert_cpi_impl> (p, info, adaptor, cpi::Noflags) {
      saga::url advert_url;
      instance_data data (this);
      advert_url = data->location_.clone();
      std::string path;
      int port;
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
      if (!scheme.empty() && scheme != "hbase" && scheme != "any") {
         SAGA_OSSTREAM strm;
         strm << "advert::advert_cpi_impl::init: "
                 "cannot handle advert entry name: " << advert_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::IncorrectURL);
      }
      port = advert_url.get_port();
      boost::shared_ptr<TTransport> socket(new TSocket(host, port));
      transport_ = boost::shared_ptr<TTransport>(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport_));
      client_ = new HbaseClient(protocol);
      transport_->open();
      saga::advert::flags mode = (saga::advert::flags)data->mode_;
      if (((mode & saga::advert::Create) || (mode & saga::advert::CreateParents)) && 
          (mode & saga::advert::Exclusive)) {
          if(!url_exists(*client_, advert_url.get_url())) {
             SAGA_OSSTREAM strm;
             strm << "advert::advert_cpi_impl::init: "
                     "advert already exists: " << advert_url.get_url();
             SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
          }
      }
      else if ((mode & saga::advert::Create) || (mode & saga::advert::CreateParents)) {
          if(!url_exists(*client_, advert_url.get_url())) {
             if(!create_url(*client_, advert_url.get_url(), false)) {
                // failed to create url
                SAGA_OSSTREAM strm;
                strm << "advert::advert_cpi_impl::init: "
                        "advert couldn't create url: " << advert_url.get_url();
                SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
             }
          }
          /*else {
             SAGA_OSSTREAM strm;
             strm << "advert::advert_cpi_impl::init: "
                     "advert already exists: " << advert_url.get_url();
             SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::AlreadyExists);
          }*/
         // no need to create this entry twice
         data->mode_ &= ~(saga::advert::Create | saga::advert::CreateParents);
      }
      if(!url_exists(*client_, advert_url.get_url())) {
         SAGA_OSSTREAM strm;
         strm << "advert::advert_cpi_impl::init: "
                 "couldn't create advert : " << advert_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   ////////////////////////////////////////////////////////////////////////
   //  destructor
   advert_cpi_impl::~advert_cpi_impl (void) {
      transport_->close();
      free(client_);
   }

  ////////////////////////////////////////////////////////////////////////
  //  SAGA CPI functions 

  ////////////////////////////////////////////////////////////////////////
  // attribute functions
   void advert_cpi_impl::sync_attribute_exists (bool        & ret, 
                                               std::string   key) {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path = advert_url.get_path();
      std::string last = get_last(advert_url);
      std::string pidstring = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string(get_node_id(*client_, last, pidstring));

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
                  if(it->first == "metakey:" + key) {
                     ret = true;
                     return;
                  }
               }
            }
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }
      ret = false;
   }

   void advert_cpi_impl::sync_attribute_is_readonly (bool        & ret, 
                                                    std::string   key) {
      instance_data data (this);
      ret = !(data->mode_ & saga::advert::Read);
      ret = false;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
   }

   void advert_cpi_impl::sync_attribute_is_writable (bool        & ret, 
                                                    std::string   key) {
      instance_data data (this);
      ret = (data->mode_ & saga::advert::Write) ? true : false;
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
      instance_data data (this);
      ret = (data->mode_ & saga::advert::Write) ? true : false;
      //ret = true;
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_get_attribute (std::string & ret, 
                                            std::string   key)
  {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);

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
                  if(it->first == "metakey:" + key) {
                     ret = it->second.value;
                     return;
                  }
               }
            }
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }
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
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);
      std::vector<Mutation> mutations;
      mutations.push_back(Mutation());
      mutations.back().column = "metakey:" + key;
      mutations.back().value = val;
      client_->mutateRow("metadata", last + "-" + node_id_string, mutations);
      //client_->put("metadata", last + "-" + node_id_string, "metakey:" + key, val);
      mutations.clear();
      mutations.push_back(Mutation());
      mutations.back().column = "entry:node_id";
      mutations.back().value = node_id_string;
      client_->mutateRow("metadata", last + "-" + node_id_string, mutations);
      //client_->put("metadata", last + "-" + node_id_string, "entry:node_id" , node_id_string);
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
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
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);

      client_->deleteAll("metadata", last + "-" + node_id_string, "metakey:" + key);
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_list_attributes (std::vector <std::string> & ret)
  {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);

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
                     ret.push_back(it->first.substr(strlen("metakey:")));
               }
            }
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void advert_cpi_impl::sync_find_attributes (std::vector<std::string> & ret, 
                                              std::string                pattern)
  {
      std::cerr << "=============!sync_find_vector_attribute!===============" << std::endl;
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  // namespace_entry functions
  void advert_cpi_impl::sync_get_url (saga::url & url)
  {
    instance_data data (this);
    url = data->location_.clone();
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_get_cwd(saga::url& url)
  {
    saga::url adname;
    instance_data data (this);
    adname = data->location_.clone();
    std::string path(adname.get_path());
    std::string::size_type p = path.find_last_of('/');
    if (p != std::string::npos)
        adname.set_path(path.substr(0, p+1));
    
    url = adname.get_url();
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_get_name (saga::url & url)
  {
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
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_read_link (saga::url & url)
  {
    std::cerr << "=============!sync_read_link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_is_dir (bool & ret)
  {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);

      std::vector<std::string> columnNames;
      columnNames.push_back("entry:");
      int scanner = client_->scannerOpen("nodes", "", columnNames);
      try {
         while(true) {
            TRowResult value;
            client_->scannerGet(value, scanner);
            if(value.row == last + "-" + node_id_string) {
               std::map<std::string,TCell>::const_iterator it;
               for(it = value.columns.begin(); it != value.columns.end(); it++) {
                  if(it->first == "entry:is_dir") {
                     if(it->second.value == "false") 
                        ret = false;
                     else
                        ret = true;
                  }
               }
            }
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_is_entry (bool & ret)
  {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);

      std::vector<std::string> columnNames;
      columnNames.push_back("entry:");
      int scanner = client_->scannerOpen("nodes", "", columnNames);
      try {
         while(true) {
            TRowResult value;
            client_->scannerGet(value, scanner);
            if(value.row == last + "-" + node_id_string) {
               std::map<std::string,TCell>::const_iterator it;
               for(it = value.columns.begin(); it != value.columns.end(); it++) {
                  if(it->first == "entry:is_dir") {
                     if(it->second.value == "false") 
                        ret = true;
                     else
                        ret = false;
                  }
               }
            }
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
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
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);
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
      client_->deleteAll("metadata", last + "-" + node_id_string, "entry:store_string");
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
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
   instance_data data (this);
   saga::url advert_url;
   advert_url = data->location_.clone();
   saga::url url_path         = advert_url.get_path();
   std::string last           = get_last(advert_url);
   std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
   std::string node_id_string = get_node_id(*client_, last, pidstring);
   std::vector<Mutation> mutations;
   mutations.push_back(Mutation());
   mutations.back().column = "entry:store_string";
   mutations.back().value = str;
   client_->mutateRow("metadata", last + "-" + node_id_string, mutations);
   //client_->put("metadata", last + "-" + node_id_string, "entry:store_string", str);
   //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void advert_cpi_impl::sync_retrieve_string (std::string & ret)
  {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(advert_url);
      std::string pidstring      = get_parent_id_of_entry(*client_, advert_url);
      std::string node_id_string = get_node_id(*client_, last, pidstring);

      std::string store_string("");
      std::vector<std::string> columnNames;
      columnNames.push_back("entry:");
      int scanner = client_->scannerOpen("metadata", "", columnNames);
      try {
         while(true) {
            TRowResult value;
            client_->scannerGet(value, scanner);
            if(value.row == last + "-" + node_id_string) {
               std::map<std::string,TCell>::const_iterator it;
               for(it = value.columns.begin(); it != value.columns.end(); it++) {
                  if(it->first == "entry:store_string") {
                     store_string = it->second.value;
                     ret = store_string;
                     return;
                  }
               }
            }
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace hbase_advert
//////////////////////////////////////////////////////////////////////////

