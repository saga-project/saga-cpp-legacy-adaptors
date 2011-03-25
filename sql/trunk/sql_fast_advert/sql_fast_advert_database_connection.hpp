#ifndef ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP
#define ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP

// Saga Includes
#include <saga/url.hpp>

// SOCI Includes
#include <soci.h>
#include <soci-postgresql.h>
#include <connection-pool.h>

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

	};

}

#endif // ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP