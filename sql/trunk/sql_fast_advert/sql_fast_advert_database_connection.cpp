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
				   soci::use((int) hash["/root/"]);
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
		
		for (int i = 0; i != CONNECTION_POOL_SIZE; ++i)
		{
			soci::session &sql = pool->at(i);
			sql.open(soci::postgresql, connectString);
		}
		
		std::cout << "DB Conection Contructor !" << std::endl;
	}
	
	database_connection::~database_connection(void)
	{
		delete pool;
	}
	
	node database_connection::find_node(std::string path)
	{
		std::string db_path;

		if (path.length() == 0 | path == "/")
		{
			db_path = "/root/";
		}
		
		else
		{
			db_path = "/root" + path;
		}
		
		node db_node;
		
		soci::session sql(*pool);
		
		sql << "SELECT id, name, lft, rgt FROM " << DATABASE_NODE_TABLE << " WHERE hash = :hash", 
		    soci::into(db_node.id),
		    soci::into(db_node.name),
		    soci::into(db_node.lft),
		    soci::into(db_node.rgt),
		    soci::use((int) hash[db_path]); 
		
		return db_node;
	}

}