#include "common_helpers.hpp"
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include <saga/saga/adaptors/task.hpp>

//hypertable helper function
const int RETRY_TIMEOUT = 30;
void handle_mutation_failure(TableMutatorPtr &mutator_ptr) {
   std::vector<std::pair<Cell, int> > failed_mutations;
   mutator_ptr->get_failed(failed_mutations);
   if (!failed_mutations.empty()) {
      for (size_t i=0; i<failed_mutations.size(); i++) {
         std::cerr << "Failed: (" << failed_mutations[i].first.row_key << "," 
              << failed_mutations[i].first.column_family;
         if (failed_mutations[i].first.column_qualifier)
            std::cerr << ":" << failed_mutations[i].first.column_qualifier;
         std::cerr << "," << failed_mutations[i].first.timestamp << ") - "
              << Error::get_text(failed_mutations[i].second) << std::endl;
      }
   }
}


std::string get_node_id(ClientPtr client, std::string entry, std::string parent_id) {
   std::cerr << "Entering get_node_id in common_helpers.cpp" << std::endl;
   TablePtr table_ptr;
   TableScannerPtr scanner_ptr;
   ScanSpecBuilder scan_spec_builder;
   Cell cell;
   std::string node_id("");
   bool flag = false;
   scan_spec_builder.add_column("entry"); //adds column family
   scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
   try {
      while (scanner_ptr->next(cell)) {
         String temp = cell.row_key;
         if(temp.substr(0,entry.length()) == entry) {
            String columnFamily    = cell.column_family;
            String columnQualifier = cell.column_qualifier;
            String cellValue = (const char *)cell.value;
            if(columnFamily == "entry" && columnQualifier == "node_id") {
               node_id = std::string((const char *)cell.value, cell.value_len).c_str();
            }
            if(columnFamily == "entry" && columnQualifier == "parent_id" && cellValue == parent_id) {
               flag = true;
            }
         }
         // break out found data
         if(flag == true) {
            std::cerr << "Leaving get_node_id in common_helpers.cpp" << std::endl;
            std::cerr << "    return " << node_id << " from common_helpers.cpp" << std::endl;
            return node_id;
         }
      }
   }
   catch (Exception &e) {
      std::cerr << "Exception caught: " << Error::get_text(e.code()) << " - " << e.what() << std::endl;
   }
   catch (std::exception &e) {
      std::cerr << "error: " << e.what() << std::endl;
   }
   return node_id; // shouldn't get here
   std::cerr << "Leaving get_node_id in common_helpers.cpp" << std::endl;
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

bool entry_exists(ClientPtr client, std::string node_name, std::string parent_id) {
   std::cerr << "Entering entry_exists in common_helpers.cpp" << std::endl;
   std::cerr << "   looking to see if " << node_name << " with " << parent_id << " as parent" << std::endl;
   TablePtr table_ptr;
   TableScannerPtr scanner_ptr;
   ScanSpecBuilder scan_spec_builder;
   Cell cell;
   try {
      scan_spec_builder.add_column("entry");
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      std::string node_id = get_node_id(client, node_name, parent_id);
      while (scanner_ptr->next(cell)) {
         if(cell.row_key == node_name + "-" + node_id) {
            String columnFamily    = cell.column_family;
            String columnQualifier = cell.column_qualifier;
            String cellValue = (const char *)cell.value;
            if(columnFamily == "entry" && columnQualifier == "parent_id" && cellValue == parent_id) {
               std::cerr << "Leaving entry_exists in common_helpers.cpp" << std::endl;
               std::cerr << "    return true" << std::endl;
               return true;
            }
         }
      }
   }
   catch (std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
   }
   std::cerr << "Leaving entry_exists in common_helpers.cpp" << std::endl;
   std::cerr << "    return false" << std::endl;
   return false;
}


bool url_exists(ClientPtr client, saga::url url) {
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

std::string get_new_node_id(ClientPtr client) {
   std::cerr << "Entering get_new_node_id in common_helpers.cpp" << std::endl;
   int retval = 1;
   TablePtr table_ptr;
   TableScannerPtr scanner_ptr;
   ScanSpecBuilder scan_spec_builder;
   Cell cell;
   try {
      table_ptr = client->open_table("nodes");
      scan_spec_builder.add_column("entry");
      scanner_ptr = table_ptr->create_scanner(scan_spec_builder.get());
      
      while (scanner_ptr->next(cell)) {
         if(cell.column_qualifier == "node_id")
            retval++;
      }
   }
   catch (std::exception &e) {
      std::cerr << "Error: " << e.what() << std::endl;
   }
   std::cerr << "Leaving get_new_node_id in common_helpers.cpp" << std::endl;
   std::cerr << "returning " << retval << std::endl;
   return boost::lexical_cast<std::string>(retval);
}

//Gives parent_id of a url
//Takes url and strips it to path then returns parent of the last entry
std::string get_parent_id_of_entry(ClientPtr client, saga::url url) {
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

bool create_url(ClientPtr client, saga::url url, bool is_dir) {
   std::cerr << "Entering create_url in common_helpers.cpp" << std::endl;
   std::cerr << "   trying to create " << url << std::endl;
   typedef boost::char_separator<char> char_separator_type;
   typedef boost::tokenizer<char_separator_type> tokenizer_type;
   int parent_id = 0;
   std::string last = get_last(url);
   std::string path = url.get_path();
   tokenizer_type tok(path, char_separator_type("/"));
   tokenizer_type::iterator end = tok.end();
   std::string element;
   std::string node_id;

   TablePtr table_ptr;
   TableMutatorPtr mutator_ptr;
   KeySpec key;
   table_ptr = client->open_table("nodes");
   mutator_ptr = table_ptr->create_mutator();

   try {
      memset(&key, 0, sizeof(key));
      for (tokenizer_type::iterator it = tok.begin(); it != end; ++it) {
         std::string entry = *it;
         std::string pidstring = boost::lexical_cast<std::string>(parent_id);
         String rowString = entry + "-" + node_id;
         key.row = rowString.c_str();
         key.row_len = rowString.length();
         if(!entry_exists(client, entry, pidstring)) {
            node_id = get_new_node_id(client);
            key.column_family = "entry";
            key.column_qualifier = "node_id";
            mutator_ptr->set(key, node_id.c_str());
            key.column_family = "entry";
            key.column_qualifier = "node_name";
            mutator_ptr->set(key, entry.c_str());
            key.column_family = "entry";
            key.column_qualifier = "parent_id";
            mutator_ptr->set(key, pidstring.c_str());
            key.column_family = "entry";
            key.column_qualifier = "is_dir";
            mutator_ptr->set(key, "true");
            parent_id = boost::lexical_cast<int>(node_id);
         }
         else {
            parent_id = boost::lexical_cast<int>(get_node_id(client, entry, pidstring));
         }
      }
      key.column_family = "entry";
      key.column_qualifier = "is_dir";
      mutator_ptr->set(key, (is_dir) ? "true" : "false");
      mutator_ptr->flush();
      std::cerr << "Leaving create_url in common_helpers.cpp" << std::endl;
   }
   catch (Exception &e) {
      std::cerr << "Exception caught: " << Error::get_text(e.code()) << " - " << e.what() << std::endl;
      do {
         if (!mutator_ptr->need_retry())
            _exit(1);
         handle_mutation_failure(mutator_ptr);
      } while (!mutator_ptr->retry(RETRY_TIMEOUT));
   }
   return true;
}
