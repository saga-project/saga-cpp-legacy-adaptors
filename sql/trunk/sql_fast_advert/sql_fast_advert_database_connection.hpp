#ifndef ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP
#define ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP

// Saga Includes
#include <saga/url.hpp>

// Enable SOCI Boost Integration
#define SOCI_USE_BOOST

// SOCI Includes
#include <soci.h>
#include <soci-postgresql.h>
#include <connection-pool.h>

// Boost Includes
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <map>

// Jenkins Hash
#include "jenkins_hash_new.hpp"

// Database Connnection Defines
#define DATABASE_LAYOUT_VERSION "1.0"

// Database Table names
#define DATABASE_VERSION_TABLE					"version"
#define DATABASE_NODE_TABLE	 					"nodes"
#define DATABASE_ATTRIBUTES_TABLE 				"attributes"
#define DATABASE_VECTOR_ATTRIBUTES_TABLE 		"vector_attributes"
#define DATABASE_VECTOR_ATTRIBUTES_VALUE_TABLE 	"vector_attribute_values"
#define DATABASE_DATA_TABLE						"data"

namespace sql_fast_advert
{
	struct node
	{
		node() : id(), name(""), dir("FALSE"), lft(0), rgt(0) {}
		
		int 		id;
		std::string name;
		std::string	dir;
		int 		lft;
		int			rgt;
	};

	class database_connection
	{
	private:
		util::jenkins_hash hash;
		soci::connection_pool *pool;
		
		int CONNECTION_POOL_SIZE;
		int BATCH_SIZE;

	public:
		// Contructor
		database_connection(const saga::url &url, std::map<std::string, std::string> &ini_file_options);
		
		// Destructor
		~database_connection(void);
		
		// MPTT Operations
		node find_node(const std::string path);
		
		node insert_node(const node parent, const std::string node_name, const bool is_dir = true);
		
		void remove_node(const node db_node);
		
		std::string get_path(const node db_node);
		
		void get_child_nodes(std::vector<node> &ret, const node parent);
		
		static bool node_is_leaf(const node db_node);
		
		// Attribute Operations
		bool attribute_exists (const node db_node, const std::string key);
		
		bool attribute_is_vector (const node db_node, const std::string key);
		
		std::string get_attribute (const node db_node, const std::string key);
		
		void set_attribute (const node db_node, const std::string key, const std::string value);

		void get_vector_attribute (const node db_node, std::vector<std::string> &ret, const std::string key);
			
		void set_vector_attribute (const node db_node, const std::string key, std::vector<std::string> value);

		void remove_attribute (const node db_node, const std::string key);
		
		void list_attributes (std::vector<std::string> &ret, const node db_node);
		
		void find_attributes (std::vector<std::string> &ret, const node db_node);
		 
	};

}

#endif // ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP