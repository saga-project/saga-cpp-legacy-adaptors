#ifndef ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP
#define ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP

// Saga Includes
#include <saga/url.hpp>

// SOCI Includes
#include <soci.h>
#include <soci-postgresql.h>

namespace sql_fast_advert
{

	class database_connection
	{
	public:
		// Contructor
		database_connection(saga::url url);
		
		// Destructor
		~database_connection(void);
		
		
	private:
		SOCI::Session *sql;
	};

}

#endif // ADAPTORS_SQL_FAST_ADVERT_DATABASE_CONNECTION_HPP