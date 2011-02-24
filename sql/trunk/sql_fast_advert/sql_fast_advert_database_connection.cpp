#include "sql_fast_advert_database_connection.hpp"

namespace sql_fast_advert
{
	database_connection::database_connection(saga::url url)
	{
		std::string connectString = "dbname=fast_advert";
		connectString += " host=" + url.get_host();
		connectString += " port=" + url.get_port();
		
		try
		{
			sql = new soci::session("postgresql", connectString);
		}
		catch(std::runtime_error e)
		{
			throw(e);
		}
	}

}