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
		boost::optional<std::string> table_name;
		sql << "SELECT tablename FROM pg_tables WHERE tablename='" << DATABASE_VERSION_TABLE << "'", soci::into(table_name);
		
		if (!table_name.is_initialized())
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
	}
	
	database_connection::~database_connection(void)
	{
		delete pool;
	}
	
	node database_connection::find_node(const std::string path)
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
		
		sql << "SELECT id, name, dir, lft, rgt FROM " << DATABASE_NODE_TABLE << " WHERE hash = :hash", 
		    soci::into(db_node.id),
		    soci::into(db_node.name),
			soci::into(db_node.dir),
		    soci::into(db_node.lft),
		    soci::into(db_node.rgt),
		    soci::use((int) hash[db_path]); 
		
		
		return db_node;
	}

	node database_connection::insert_node(const node parent, const std::string path)
	{
		node db_node;
		int hash_value = (int) hash[get_path(parent) + path];
		
		soci::session sql(*pool);
		
		sql << "UPDATE nodes SET rgt = rgt + 2 WHERE rgt > :lft", soci::use(parent.lft);
		sql << "UPDATE nodes SET lft = lft + 2 WHERE lft > :lft", soci::use(parent.lft);
		
		sql << "INSERT INTO nodes (name, dir, lft, rgt, hash) VALUES (:name, :dir, :lft, :rgt, :hash)",
			soci::use(path),
			soci::use("TRUE"),
			soci::use(parent.lft + 1),
			soci::use(parent.lft + 2),
			soci::use(hash_value);
			
		sql << "SELECT id, name, dir, lft, rgt FROM " << DATABASE_NODE_TABLE << " WHERE hash = :hash",
			soci::into(db_node.id),
		    soci::into(db_node.name),
			soci::into(db_node.dir),
		    soci::into(db_node.lft),
		    soci::into(db_node.rgt),
		    soci::use(hash_value);
		
		return db_node;
	}
	
	std::string database_connection::get_path(const node db_node)
	{
		const int BATCH_SIZE = 100;
		std::vector<std::string> path_vector(BATCH_SIZE);		
		std::string result = "/";
		
		soci::session sql(*pool);
		
		soci::statement statement = ( sql.prepare << 
		"SELECT parent.name FROM nodes AS node, nodes AS parent "
		"WHERE node.lft BETWEEN parent.lft AND parent.rgt "
		"AND node.id = :id ORDER BY parent.lft", soci::into(path_vector), soci::use(db_node.id) );
		
		statement.execute();
		
		while(statement.fetch())
		{
			for(std::vector<std::string>::iterator i = path_vector.begin(); i != path_vector.end(); i++)
			{
				result += (*i + "/"); 
			}
			
			path_vector.resize(BATCH_SIZE);
		}
		
		return result;
	}
}