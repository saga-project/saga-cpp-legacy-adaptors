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
				   soci::use((int) hash["/"]);
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
		node db_node;
		soci::session sql(*pool);
		
		sql << "SELECT id, name, dir, lft, rgt FROM " << DATABASE_NODE_TABLE << " WHERE hash = :hash", 
		    soci::into(db_node.id),
		    soci::into(db_node.name),
			soci::into(db_node.dir),
		    soci::into(db_node.lft),
		    soci::into(db_node.rgt),
		    soci::use((int) hash[path]); 
				
		return db_node;
	}

	node database_connection::insert_node(const node parent, const std::string node_name)
	{
		node db_node;
		std::string node_path = get_path(parent) + "/" + node_name;
		
		std::cout << "parent path : " << node_path << std::endl;
		
		int hash_value = (int) hash[node_path];
		soci::session sql(*pool);
		
		sql << "UPDATE nodes SET rgt = rgt + 2 WHERE rgt > :lft", soci::use(parent.lft);
		sql << "UPDATE nodes SET lft = lft + 2 WHERE lft > :lft", soci::use(parent.lft);
		
		sql << "INSERT INTO nodes (name, dir, lft, rgt, hash) VALUES (:name, :dir, :lft, :rgt, :hash)",
			soci::use(node_name),
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
		std::string result = "";
		
		soci::session sql(*pool);
		
		//
		// Select all nodes except the root node 
		//	
		soci::statement statement = ( sql.prepare << 
		"SELECT parent.name FROM nodes AS node, nodes AS parent "
		"WHERE node.lft BETWEEN parent.lft AND parent.rgt and parent.id != 1"
		"AND node.id = :id ORDER BY parent.lft", soci::into(path_vector), soci::use(db_node.id) );
		
		statement.execute();
		
		while(statement.fetch())
		{
			for(std::vector<std::string>::iterator i = path_vector.begin(); i != path_vector.end(); i++)
			{		
				result += ("/" + *i); 
			}
			
			path_vector.resize(BATCH_SIZE);
		}
		
		return result;
	}
	
	void database_connection::get_child_nodes(std::vector<node> &ret, const node parent)
	{
		node tmp_node;
		int depth = 0;
		soci::session sql(*pool);
		
		// Find the parent node depth
		sql << 
		"SELECT (COUNT(node.id) - 1) AS depth "
		"FROM nodes AS node, nodes AS parent "
		"WHERE node.lft BETWEEN parent.lft AND parent.rgt AND node.id = :id "
		"GROUP BY node.id", 		
		soci::into(depth),
		soci::use(parent.id);

		// Find all child nodes with depth = 1 
		soci::statement statement = 
		( sql.prepare << 
			"SELECT node.id, node.name, node.dir, node.lft, node.rgt "
			"FROM nodes AS node, nodes AS parent, nodes AS sub_parent "
			"WHERE node.lft BETWEEN parent.lft AND parent.rgt "
			"AND node.lft BETWEEN sub_parent.lft AND sub_parent.rgt AND sub_parent.id = :id "
			"GROUP BY node.id, node.name, node.dir, node.lft, node.rgt "
			"HAVING (COUNT(node.id) - (:depth + 1)) = 1", 
			soci::into(tmp_node.id),
			soci::into(tmp_node.name),
			soci::into(tmp_node.dir),
			soci::into(tmp_node.lft),
			soci::into(tmp_node.rgt),
			soci::use(parent.id),
 			soci::use(depth)
		);
		
		statement.execute();
		
		while(statement.fetch())
		{
			ret.push_back(tmp_node);
		}
	}
}