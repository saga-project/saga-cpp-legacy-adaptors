#include "common_helpers.hpp"
#include "Hbase.h"
#include <protocol/TBinaryProtocol.h>
#include <transport/TSocket.h>
#include <transport/TTransportUtils.h>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <saga/saga/adaptors/task.hpp>

using namespace facebook::thrift;
using namespace facebook::thrift::protocol;
using namespace facebook::thrift::transport;
using namespace apache::hadoop::hbase::thrift;

std::string get_node_id(HbaseClient client, std::string entry, std::string parent_id) {
   std::vector<std::string> columnNames;
   std::string node_id("");
   bool flag = false;
   columnNames.push_back("entry:");
   int scanner = client.scannerOpen("nodes", "", columnNames);
   try {
      while(true) {
         TRowResult value;
         client.scannerGet(value, scanner);
         if(value.row.substr(0,entry.length()) == entry) {
            for(std::map<std::string,TCell>::const_iterator it = value.columns.begin();
                it != value.columns.end(); it++) {
               if(it->first == "entry:node_id") {
                  node_id = it->second.value;
               }
               if(it->first == "entry:parent_id" && it->second.value == parent_id) {
                  flag = true;
               }
            }
            // break out found data
            if(flag == true) {
               return node_id;
            }
         }
      }
   }
   catch(NotFound &nf) {
      client.scannerClose(scanner);
   }
   return node_id; // shouldn't get here
}

std::string get_last(saga::url url) {
   std::string path = url.get_string();
   typedef boost::char_separator<char> char_separator_type;
   typedef boost::tokenizer<char_separator_type> tokenizer_type;
   tokenizer_type tok(path, char_separator_type("/"));
   tokenizer_type::iterator end = tok.end();
   std::string element;
   for (tokenizer_type::iterator it = tok.begin(); it != end; ++it) {
      element = *it;
   }
   return element;
}

std::string get_full_url (saga::url url, saga::url entry) {
   std::string element;
   std::vector<std::string> vec;
   std::string path = url.get_path();
   typedef boost::char_separator<char> char_separator_type;
   typedef boost::tokenizer<char_separator_type> tokenizer_type;

   tokenizer_type tok(path, char_separator_type("/"));
   tokenizer_type::iterator end = tok.end();
   for (tokenizer_type::iterator it = tok.begin(); it != end; ++it) {
      element = *it;
      vec.push_back(element);
   }
   path = entry.get_string();
   tokenizer_type tok2(path, char_separator_type("/"));
   for (tokenizer_type::iterator it = tok2.begin(); it != tok2.end(); ++it) {
      element = *it;
      if(element == ".") {
         // do nothing
      }
      else if (element == "") {
         // do nothing
      }
      else if (element == "..") {
         vec.pop_back();
      }
      else {
         vec.push_back(element);
      }
   }
   std::string string_retval;
   string_retval += url.get_scheme() + "://";
   string_retval += url.get_host() + ":"  + boost::lexical_cast<std::string>(url.get_port()) + "//";
   std::vector<std::string>::const_iterator x;
   for(x = vec.begin();x != vec.end(); x++) {
      string_retval += *x;
      string_retval += "/";
   }
   return string_retval;
}

bool entry_exists(HbaseClient client, std::string node_name, std::string parent_id) {
   std::vector<std::string> columnNames;
   columnNames.push_back("entry:");
   int scanner = client.scannerOpen("nodes", "", columnNames);
   std::string node_id = get_node_id(client, node_name, parent_id);
   try {
      while(true) {
         TRowResult value;
         client.scannerGet(value, scanner);
         if(value.row == node_name + "-" + node_id) {
            std::map<std::string,TCell>::const_iterator it;
            for(it = value.columns.begin(); it != value.columns.end(); it++) {
               if(it->first == "entry:parent_id" && it->second.value == parent_id) {
                  client.scannerClose(scanner);
                  return true;
               }
            }
         }
      }
   }
   catch(NotFound &nf) {
      client.scannerClose(scanner);
   }
   return false;
}


bool url_exists(HbaseClient client, saga::url url) {
   typedef boost::char_separator<char> char_separator_type;
   typedef boost::tokenizer<char_separator_type> tokenizer_type;
   int parent_id = 0;
   std::string pidstring;
   std::string path = url.get_path();
   std::string last_entry = get_last(url);
   tokenizer_type tok(path, char_separator_type("/"));
   tokenizer_type::iterator end = tok.end();
   for (tokenizer_type::iterator it = tok.begin(); it != end; ++it) {
      std::string entry = *it;
      pidstring = boost::lexical_cast<std::string>(parent_id);
      if(!entry_exists(client, entry, pidstring)) {
         return false;
      }
      else {
         parent_id = boost::lexical_cast<int>(get_node_id(client, entry, pidstring));
      }
   }
   if(entry_exists(client, last_entry, pidstring)) {
      return true;
   }
   else {
      return false;
   }
}

std::string get_new_node_id(HbaseClient client) {
   int retval = 1;
   std::vector<std::string> columnNames;
   columnNames.push_back("entry:");
   int scanner = client.scannerOpen("nodes", "", columnNames);
   try {
      while(true) {
         TRowResult value;
         client.scannerGet(value, scanner);
         retval++;
      }
   }
   catch(NotFound &nf) {
      client.scannerClose(scanner);
   }
   return boost::lexical_cast<std::string>(retval);
}

//Gives parent_id of a url
//Takes url and strips it to path then returns parent of the last entry
std::string get_parent_id_of_entry(HbaseClient client, saga::url url) {
   typedef boost::char_separator<char> char_separator_type;
   typedef boost::tokenizer<char_separator_type> tokenizer_type;
   int parent_id = 0;
   std::string pidstring;
   std::string path = url.get_path();
   tokenizer_type tok(path, char_separator_type("/"));
   tokenizer_type::iterator end = tok.end();
   for (tokenizer_type::iterator it = tok.begin(); it != end; ++it) {
      std::string entry = *it;
      pidstring = boost::lexical_cast<std::string>(parent_id);
      parent_id = boost::lexical_cast<int>(get_node_id(client, entry, pidstring));
   }
   return pidstring;
}

bool create_url(HbaseClient client, saga::url url, bool is_dir) {
   typedef boost::char_separator<char> char_separator_type;
   typedef boost::tokenizer<char_separator_type> tokenizer_type;
   int parent_id = 0;
   std::string last = get_last(url);
   std::string path = url.get_path();
   tokenizer_type tok(path, char_separator_type("/"));
   tokenizer_type::iterator end = tok.end();
   std::string element;
   std::string node_id;
   std::vector<Mutation> mutations;
   for (tokenizer_type::iterator it = tok.begin(); it != end; ++it) {
      std::string entry = *it;
      std::string pidstring = boost::lexical_cast<std::string>(parent_id);
      if(!entry_exists(client, entry, pidstring)) {
         node_id = get_new_node_id(client);
         mutations.clear();
         mutations.push_back(Mutation());
         mutations.back().column = "entry:node_id";
         mutations.back().value = node_id;
         client.mutateRow("nodes", entry + "-" + node_id, mutations);
         //client.put("nodes", entry + "-" + node_id, "entry:node_id"  , node_id);
         mutations.clear();
         mutations.push_back(Mutation());
         mutations.back().column = "entry:node_name";
         mutations.back().value = entry;
         client.mutateRow("nodes", entry + "-" + node_id, mutations);
         //client.put("nodes", entry + "-" + node_id, "entry:node_name", entry);
         mutations.clear();
         mutations.push_back(Mutation());
         mutations.back().column = "entry:parent_id";
         mutations.back().value = pidstring;
         client.mutateRow("nodes", entry + "-" + node_id, mutations);
         //client.put("nodes", entry + "-" + node_id, "entry:parent_id", pidstring);
         mutations.clear();
         mutations.push_back(Mutation());
         mutations.back().column = "entry:is_dir";
         mutations.back().value = "true";
         client.mutateRow("nodes", entry + "-" + node_id, mutations);
         //client.put("nodes", entry + "-" + node_id, "entry:is_dir"   , "true");
         parent_id = boost::lexical_cast<int>(node_id);
      }
      else {
         parent_id = boost::lexical_cast<int>(get_node_id(client, entry, pidstring));
      }
   }
   mutations.clear();
   mutations.push_back(Mutation());
   mutations.back().column = "entry:is_dir";
   mutations.back().value = (is_dir) ? "true" : "false";
   client.mutateRow("nodes", last + "-" + node_id, mutations);
   //client.put("nodes", last + "-" + node_id, "entry:is_dir", (is_dir) ? "true" : "false");
   return true;
}
