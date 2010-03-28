#ifndef COMMON_HELPERS_HPP
#define COMMON_HELPERS_HPP

#include <saga/saga.hpp>
#include "Hbase.h"

using namespace facebook::thrift;
using namespace facebook::thrift::protocol;
using namespace facebook::thrift::transport;
using namespace apache::hadoop::hbase::thrift;

bool create_url(HbaseClient client, saga::url url, bool is_dir);
bool url_exists(HbaseClient client, saga::url url);
bool entry_exists(HbaseClient client, std::string node_name, std::string parent_id);
std::string get_node_id(HbaseClient client, std::string entry);
std::string get_last(saga::url);
bool create_new_node(HbaseClient client, int parent_id, bool is_dir);
std::string get_node_id(HbaseClient client, std::string entry, std::string parent_id);
std::string get_parent_id_of_entry(HbaseClient client, saga::url url);
std::string get_full_url (saga::url url, saga::url entry);

#endif // COMMON_HELPERS_HPP

