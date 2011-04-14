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

// Jenkins Hash
#include "jenkins_hash_new.hpp"

// Database Connnection Defines
#define CONNECTION_POOL_SIZE 10
#define DATABASE_LAYOUT_VERSION "1.0"

// Database Table names
#define DATABASE_VERSION_TABLE	"version"
#define DATABASE_NODE_TABLE	 	"nodes"

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

	public:
		// Contructor
		database_connection(saga::url url);
		
		// Destructor
		~database_connection(void);
		
		// MPTT Operations
		node find_node(const std::string path);
		
		node insert_node(const node parent, const std::string node_name);
		
		void remove_node(const node db_node);
		
		std::string get_path(const node db_node);
		
		void get_child_nodes(std::vector<node> &ret, const node parent);
		
		static bool node_is_leaf(const node db_node);
	};

}

#endif // ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP