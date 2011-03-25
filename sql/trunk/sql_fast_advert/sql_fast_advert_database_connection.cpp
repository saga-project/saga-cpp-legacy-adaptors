#include "sql_fast_advert_database_connection.hpp"


namespace sql_fast_advert
{
	
	database_connection::database_connection(saga::url url)
	{
		std::string connectString = "dbname=fast_advert";
		connectString += " host=" + url.get_host();
		connectString += " port=" + url.get_port();
		
		// Try to connect 
		soci::session sql(soci::postgresql, connectString);

		// Check if there is allready a database layout
		// if not create a fresh layout 		
		int tableCount = 0;
		sql << "SELECT COUNT(*) FROM pg_tables WHERE tablename='" << DATABASE_VERSION_TABLE << "'", soci::into(tableCount);
		
		if (tableCount == 0)
		{
			sql << "CREATE TABLE "<< DATABASE_VERSION_TABLE << " (layout_version varchar(8))";
			sql << "INSERT INTO  "<< DATABASE_VERSION_TABLE << " VALUES(:versionString)", soci::use(DATABASE_LAYOUT_VERSION);
			
			sql << "CREATE TABLE "<< DATABASE_NODE_TABLE << " ("
			       "id 		serial 			PRIMARY KEY	,"
				   "name 	varchar(256) 	NOT NULL	,"
				   "dir 	boolean			NOT NULL	,"
				   "lft		integer			NOT NULL	,"
				   "rgt		integer			NOT NULL	,"
				   "hash	integer			NOT NULL	)";
				
			sql << "INSERT INTO " << DATABASE_NODE_TABLE << " (name, dir, lft, rgt, hash)"
			       " VALUES (:name, :dir, :lft, :rgt, :hash)", 
				   soci::use("root"),
				   soci::use("TRUE"),
				   soci::use(1),
				   soci::use(2),
				   soci::use((int) hash["/root"]);
		}
		
		// Check the Database layout version
		std:: string layoutVersion;
		sql << "SELECT layout_version FROM " << DATABASE_VERSION_TABLE, soci::into(layoutVersion);
		
		if (layoutVersion != DATABASE_LAYOUT_VERSION)
		{
			std::runtime_error error("Database layout version missmatch !");
			throw error;
		}
		
		// Initialize the connection pool
		pool = new soci::connection_pool(CONNECTION_POOL_SIZE);
		
	}
	
	database_connection::~database_connection(void)
	{
		delete pool;
	}

}