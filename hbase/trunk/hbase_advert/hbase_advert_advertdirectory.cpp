//  Copyright (c) 2005-2008 Hartmut Kaiser 
//  Copyright (c) 2005-2008 Michael Miceli   (mmicel2@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// saga includes
#include <saga/saga.hpp>

// adaptor includes
#include "hbase_advert_advertdirectory.hpp"
#include <protocol/TBinaryProtocol.h>
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>
#include "common_helpers.hpp"
#include "Hbase.h"

using namespace facebook::thrift;
using namespace facebook::thrift::protocol;
using namespace facebook::thrift::transport;
using namespace apache::hadoop::hbase::thrift;


///////////////////////////////////////////////////////////////////////////////
namespace hbase_advert {
  ////////////////////////////////////////////////////////////////////////
  //  constructor
  advertdirectory_cpi_impl::advertdirectory_cpi_impl (proxy                           * p, 
                                                      cpi_info const                  & info,
                                                      saga::ini::ini const            & glob_ini, 
                                                      saga::ini::ini const            & adap_ini,
                                                      TR1::shared_ptr <saga::adaptor>   adaptor)
    : saga::adaptors::v1_0::advert_directory_cpi <advertdirectory_cpi_impl> (p, info, adaptor, cpi::Noflags) 
  {
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
      if(path == "/") {
      }
      else if (((mode & saga::advert::Create) || (mode & saga::advert::CreateParents)) && 
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
             if(!create_url(*client_, advert_url.get_url(), true)) {
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
      /*if(!url_exists(*client_, advert_url.get_url())) {
         //here is where I am
         SAGA_OSSTREAM strm;
         strm << "advert::advert_cpi_impl::init: "
                 "advert does not exist: " << advert_url.get_url();
         SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
      }*/
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  ////////////////////////////////////////////////////////////////////////
  //  destructor
  advertdirectory_cpi_impl::~advertdirectory_cpi_impl (void) {
      transport_->close();
      free(client_);
  }

  ////////////////////////////////////////////////////////////////////////
  //  SAGA CPI functions 

  ////////////////////////////////////////////////////////////////////////
  // attribute functions
  void 
    advertdirectory_cpi_impl::sync_attribute_exists (bool        & ret, 
                                                     std::string   key)
  {
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
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_attribute_is_readonly (bool        & ret, 
                                                          std::string   key)
  {
    instance_data data (this);
    ret = !(data->mode_ & saga::advert::Read);
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_attribute_is_writable (bool        & ret, 
                                                          std::string   key)
  {
    instance_data data (this);
    ret = (data->mode_ & saga::advert::Write) ? true : false;
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_attribute_is_vector (bool        & ret, 
                                                        std::string   key)
  {
    std::cerr << "===========!sync_attribute_is_vector!=========" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_attribute_is_extended (bool        & ret, 
                                                          std::string   key)
  {
    instance_data data (this);
    ret = (data->mode_ & saga::advert::Write) ? true : false;
    ret = false;
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_get_attribute (std::string & ret, 
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
  void 
    advertdirectory_cpi_impl::sync_get_vector_attribute (std::vector <std::string> & ret, 
                                                         std::string                 key)
  {
    std::cerr << "==========!sync_get_vector_attribute!========" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_set_attribute (saga::impl::void_t & ret, 
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
  void 
    advertdirectory_cpi_impl::sync_set_vector_attribute (saga::impl::void_t              & ret, 
                                                         std::string                 key, 
                                                         std::vector <std::string>   val)
  {
      SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_remove_attribute (saga::impl::void_t & ret, 
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
  void 
    advertdirectory_cpi_impl::sync_list_attributes (std::vector <std::string> & ret)
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
  void 
    advertdirectory_cpi_impl::sync_find_attributes (std::vector <std::string> & ret, 
                                                    std::string                 pattern)
  {
    std::cerr << "==========!sync_find_attributes!=======" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  ////////////////////////////////////////////////////////////////////////
  // namespace_entry functions
  void 
    advertdirectory_cpi_impl::sync_get_url (saga::url & url)
  {
    instance_data data (this);
    url = data->location_.clone();
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_get_cwd (saga::url & url)
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

  void 
    advertdirectory_cpi_impl::sync_get_name (saga::url & url)
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
    //Throwing because Idk what it should return
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_is_dir (bool & ret)
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

  void 
    advertdirectory_cpi_impl::sync_is_entry (bool & ret)
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

  void 
    advertdirectory_cpi_impl::sync_is_link (bool & ret)
  {
    std::cerr << "=============!is_link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_read_link (saga::url & url)
  {
    std::cerr << "=============!read_link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                         saga::url      target, 
                                         int            flags)
  {
    std::cerr << "=============!sync_copy!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                         saga::url      target, 
                                         int            flags)
  {
    std::cerr << "=============!link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                         saga::url      target, 
                                         int            flags)
  {
    std::cerr << "=============!move!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                           int            flags)
  {
    std::cerr << "=============!remove!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_close (saga::impl::void_t & ret, 
                                          double         timeout)
  {
    std::cerr << "=============!close!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  ////////////////////////////////////////////////////////////////////////
  //  namespace_dir functions
  ////////////////////////////////////////////////////////////////////////
  void 
    advertdirectory_cpi_impl::sync_list (std::vector <saga::url> & ret, 
                                         std::string               pattern, 
                                         int                       flags)
  {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string last           = get_last(url_path);
      std::string pidstring      = get_parent_id_of_entry(*client_, url_path);
      std::string node_id        = get_node_id(*client_, last, pidstring);
      std::string::size_type pos = url_path.get_string().find_last_of("/");
      std::string url_minus_end  = url_path.get_string().substr(pos);
      std::string node_name;

      bool flag = false;
      std::vector<std::string> columnNames;
      columnNames.push_back("entry:");
      int scanner = client_->scannerOpen("nodes", "", columnNames);
      try {
         while(true) {
            TRowResult value;
            client_->scannerGet(value, scanner);
               std::map<std::string,TCell>::const_iterator it;
               for(it = value.columns.begin(); it != value.columns.end(); it++) {
                  if(it->first == "entry:parent_id" && it->second.value == node_id) {
                     flag = true;
                  }
                  else if(it->first == "entry:node_name") {
                     node_name = url_minus_end + it->second.value;
                     //ret.push_back();
                  }
               }
               if(flag) {
                  saga::url temp(url_path.get_string() + node_name);
                  temp.set_host(advert_url.get_host());
                  temp.set_port(advert_url.get_port());
                  temp.set_scheme(advert_url.get_scheme());
                  ret.push_back(temp);
               }
               flag = false;
         }
      }
      catch(NotFound &nf) {
         client_->scannerClose(scanner);
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_find (std::vector <saga::url> & ret, 
                                         std::string               pattern, 
                                         int                       flags)
  {
    std::cerr << "=============!find!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_exists (bool      & ret, 
                                           saga::url   entry)
  {
      if(url_exists(*client_, entry)) {
         ret = true;
      }
      else {
         ret = false;
      }
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_is_dir (bool      & ret, 
                                           saga::url   entry)
  {
      instance_data data (this);
      saga::url advert_url;
      advert_url = data->location_.clone();
      saga::url url_path         = advert_url.get_path();
      std::string string_arg     = get_full_url(advert_url, entry);
      saga::url arg(string_arg);
      std::string last           = get_last(arg);
      std::string pidstring      = get_parent_id_of_entry(*client_, arg);
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

  void 
    advertdirectory_cpi_impl::sync_is_entry (bool      & ret, 
                                             saga::url   entry)
  {
    std::cerr << "=============!is_entry!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_is_link (bool      & ret, 
                                            saga::url   target)
  {
    std::cerr << "=============!is_link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_read_link (saga::url & url, 
                                              saga::url   target)
  {
    std::cerr << "=============!read_link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_get_num_entries (std::size_t  & num_entries)
  {
    std::cerr << "=============!get_num_entries!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_get_entry (saga::url    & entry, 
                                              std::size_t    idx)
  {
    std::cerr << "=============!get_entry!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_copy (saga::impl::void_t & ret, 
                                         saga::url      source,
                                         saga::url      target, 
                                         int            flags)
  {
    std::cerr << "=============!copy!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_link (saga::impl::void_t & ret, 
                                         saga::url      source,
                                         saga::url      target, 
                                         int            flags)
  {
    std::cerr << "=============!link!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_move (saga::impl::void_t & ret, 
                                         saga::url      source,
                                         saga::url      target, 
                                         int            flags)
  {
    std::cerr << "=============!move!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_remove (saga::impl::void_t & ret, 
                                           saga::url      entry, 
                                           int            flags)
  {
    std::cerr << "=============!remove!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_open (saga::name_space::entry & ret, 
                                         saga::url                 entry, 
                                         int                       flags)
  {
    std::cerr << "=============open_(name_space)===============" << std::endl;
    sync_open((saga::name_space::directory&)ret, entry, flags);
    std::cerr << "=============!open_(name_space)===============" << std::endl;
    //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_open_dir (saga::name_space::directory & ret, 
                                             saga::url                     entry,
                                             int                           flags)
  {
      std::cerr << "=============open_dir(name_space)===============" << std::endl;
      sync_open_dir((saga::name_space::directory&)ret, entry, flags);
      std::cerr << "=============!open_dir(name_space)===============" << std::endl;
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_change_dir (saga::impl::void_t & ret, 
                                               saga::url      dir)
  {
    std::cerr << "=============!change_dir!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_make_dir (saga::impl::void_t & ret, 
                                             saga::url      dir, 
                                             int            flags)
  {
    std::cerr << "=============!make_dir!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  ////////////////////////////////////////////////////////////////////////
  //  directory functions
  void 
    advertdirectory_cpi_impl::sync_open (saga::advert::entry & ret, 
                                         saga::url             entry, 
                                         int                   flags)
  {
      saga::url advert_directory_url;
      instance_data data (this);
      advert_directory_url = data->location_.clone();
      saga::url entry_to_create = get_full_url(advert_directory_url, entry);
      ret = saga::advert::entry(this->get_proxy()->get_session(), 
         entry_to_create.get_url(), flags);
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_open_dir (saga::advert::directory & ret, 
                                             saga::url                 entry, 
                                             int                       flags)
  {
      saga::url advert_directory_url;
      instance_data data (this);
      advert_directory_url = data->location_.clone();
      saga::url entry_to_create = get_full_url(advert_directory_url, entry);
      ret = saga::advert::directory(this->get_proxy()->get_session(), 
         entry_to_create.get_url(), flags);
      //SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void 
    advertdirectory_cpi_impl::sync_find (std::vector<saga::url>  & ret, 
                                         std::string               pattern, 
                                         std::vector <std::string> patterns, 
                                         int                       flags)
  {
    std::cerr << "=============!sync_find!===============" << std::endl;
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace hbase_advert
////////////////////////////////////////////////////////////////////////

