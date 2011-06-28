#include "sql_fast_advert_database_connection.hpp"


namespace sql_fast_advert
{

  database_connection::database_connection(const saga::url &url, std::map<std::string, std::string> &ini_file_options)
  {
    std::string dbname = "";

    if (url.get_query().find("dbname=") != std::string::npos)
    {
      int start = url.get_query().find("dbname=");
      int end = url.get_query().find(",");

      dbname = url.get_query().substr(start + 7, end - (start + 7));	
    }

    else 
    {
      dbname = ini_file_options["dbname"];
    }

    std::string host = "";

    if (url.get_host() != "")
    {
      host = url.get_host();
    }

    else
    {
      host =  ini_file_options["host"];
    }

    std::string port = "";

    if (url.get_port() != -1)
    {
      port = boost::lexical_cast<std::string>(url.get_port());
    }

    else
    {
      port = ini_file_options["port"];
    }

    std::string user = "";

    if (url.get_username() != "")
    {
      user = url.get_username();
    }

    else
    {
      user = ini_file_options["user"];
    }

    std::string password = "";

    if (url.get_password() != "")
    {
      password = url.get_password();
    }

    else
    {
      password = ini_file_options["password"];
    }

    connectString  = "dbname="    + dbname;
    connectString += " host="     + host;
    connectString += " port="     + port;
    connectString += " user="     + user;
    connectString += " password=" + password;

    CONNECTION_POOL_SIZE 			    = boost::lexical_cast<int>(ini_file_options["connection_pool_size"]);
    BATCH_SIZE 						        = boost::lexical_cast<int>(ini_file_options["batch_size"]);
    CURRENT_CONNECTION_POOL_SIZE 	= 0;


    // Initialize the connection pool
    pool = new soci::connection_pool(CONNECTION_POOL_SIZE);

    //for (int i = 0; i != CONNECTION_POOL_SIZE; i++)
    //{
    //  soci::session &sql = pool->at(i);
    //  sql.open(soci::postgresql, connectString);
    //}
    
    // Try to connect and holt at least one opend soci::session
    //grow_pool();

    // Check if there is allready a database layout
    // if not create a fresh layout 

    if (ini_file_options["check_db"] == "true")
    {
      soci::session sql(*pool);

      boost::optional<std::string> table_name;
      sql << "SELECT tablename FROM pg_tables WHERE tablename='" << DATABASE_VERSION_TABLE << "'", soci::into(table_name);

      if (!table_name.is_initialized())
      {
        sql << "CREATE TABLE "<< DATABASE_VERSION_TABLE << " (layout_version varchar(8))";
        sql << "INSERT INTO  "<< DATABASE_VERSION_TABLE << " VALUES(:versionString)", soci::use(DATABASE_LAYOUT_VERSION);

        sql << "CREATE TABLE "<< DATABASE_NODE_TABLE << " ("
          "id 		  serial 			  PRIMARY KEY	,"
          "name 		varchar(256) 	NOT NULL	,"
          "dir 		  boolean			  NOT NULL	,"
          "lft	  	integer			  NOT NULL	,"
          "rgt		  integer			  NOT NULL	,"
          "hash		  integer			  NOT NULL	)";

        sql << "INSERT INTO " << DATABASE_NODE_TABLE << " (name, dir, lft, rgt, hash)"
          " VALUES (:name, :dir, :lft, :rgt, :hash)", 
          soci::use("root"),
          soci::use("TRUE"),
          soci::use(1),
          soci::use(2),
          soci::use((int) hash["/"]);

        sql << "CREATE TABLE " << DATABASE_ATTRIBUTES_TABLE << " ("
          "node_id		    integer			  NOT NULL	,"
          "key			      varchar(256)	NOT NULL	,"
          "value			    varchar(256)	NOT NULL	,"
          "is_vector 		  boolean			  NOT NULL	)";
          //"UNIQUE (node_id, key)			            )";		

        sql << "CREATE TABLE " << DATABASE_DATA_TABLE << " ("
          "node_id		integer			NOT NULL	,"
          "data			  varchar			NOT NULL	)";
      }

      //Check the Database layout version
      std:: string layoutVersion;
      sql << "SELECT layout_version FROM " << DATABASE_VERSION_TABLE, soci::into(layoutVersion);

      if (layoutVersion != DATABASE_LAYOUT_VERSION)
      {
        soci::soci_error error("Database layout version missmatch !");
        throw error;
      }
    }
  }

  database_connection::~database_connection(void)
  {
    delete pool;
  }

  void database_connection::grow_pool()
  {
    if (CURRENT_CONNECTION_POOL_SIZE < CONNECTION_POOL_SIZE)
    {
      soci::session &sql = pool->at(CURRENT_CONNECTION_POOL_SIZE);
      //sql.open(soci::postgresql, connectString);
      
      try
      {  
        sql << "select * from nodes";
      }
      
      catch(soci::soci_error e)
      {
        std::cout << "Damm" << std::endl;
        std::cout << e.what() << std::endl;
      }
      
      std::cout << "hello" << std::endl;

      CURRENT_CONNECTION_POOL_SIZE++;
    }
  }

  node database_connection::find_node(const std::string path)
  {
    std::string db_path;
    node db_node;
    
    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }

    sql << "SELECT id, name, dir, lft, rgt FROM " << DATABASE_NODE_TABLE << " WHERE hash = :hash", 
      soci::into(db_node.id),
      soci::into(db_node.name),
      soci::into(db_node.dir),
      soci::into(db_node.lft),
      soci::into(db_node.rgt),
      soci::use((int) hash[path]); 

    return db_node;
  }

  node database_connection::insert_node(const node parent, const std::string node_name, const bool is_dir)
  {
    node db_node;
    std::string node_path = get_path(parent) + "/" + node_name;

    std::string dir = is_dir ? "TRUE":"FALSE";
    int hash_value = (int) hash[node_path];
    
    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }

    /*
    sql << "UPDATE nodes SET rgt = rgt + 2 WHERE rgt > :lft", soci::use(parent.lft);
    sql << "UPDATE nodes SET lft = lft + 2 WHERE lft > :lft", soci::use(parent.lft);

    sql << "INSERT INTO nodes (name, dir, lft, rgt, hash) VALUES (:name, :dir, :lft, :rgt, :hash)",
      soci::use(node_name),
      soci::use(dir),
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

    //std::cout << "insert node" << std::endl;
    */
    
    // ==================================================================
    // = insert node using stored procedure insert_node(n , d, l, r, h) =
    // ==================================================================

    sql << "SELECT id, name, dir, lft, rgt FROM insert_node(:n, :d, :l, :r, :h) AS (id integer, name varchar, dir boolean, lft integer, rgt integer, hash integer);", 
      soci::into(db_node.id),
      soci::into(db_node.name),
      soci::into(db_node.lft),
      soci::into(db_node.rgt),
      soci::use(node_name), 
      soci::use(dir),
      soci::use(parent.lft),
      soci::use(parent.rgt),
      soci::use(hash_value);
    
    return db_node;
  }

  void database_connection::remove_node(const node db_node)
  {		
    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }
    
    //sql.begin();

    //
    // Remove the node and its childs using stored procedure remove_node(l integer, r integer)
    // 

    soci::procedure stored_procedure = (sql.prepare << "remove_node(:l, :r)", soci::use(db_node.lft), soci::use(db_node.rgt));
    stored_procedure.execute(true);

    /*
    std::vector<int> node_id_batch(BATCH_SIZE);

    soci::statement statement = 
    (
      sql.prepare << "DELETE FROM " << DATABASE_NODE_TABLE << " WHERE lft BETWEEN :lft AND :rgt RETURNING id", soci::use(db_node.lft), soci::use(db_node.rgt), soci::into(node_id_batch)	
    );

    statement.execute();

    while(statement.fetch())
    {
      for(std::vector<int>::iterator i = node_id_batch.begin(); i != node_id_batch.end(); i++)
      {
        // =====================
        // = remove attributes =
        // =====================

        sql << "DELETE FROM " << DATABASE_ATTRIBUTES_TABLE << " WHERE node_id = :id", soci::use(*i);


        // ========================
        // = remove data for node =
        // ========================

        sql << "DELETE FROM " << DATABASE_DATA_TABLE << " WHERE node_id = :id", soci::use(*i);


        // ========================
        // = resize the the batch =
        // ========================

        node_id_batch.resize(BATCH_SIZE);
      }
    }


    // ==========================
    // = Update the MPTT values =
    // ==========================
    sql << "UPDATE " << DATABASE_NODE_TABLE << " SET rgt = rgt - :width WHERE rgt > :rgt", soci::use(db_node.rgt - db_node.lft + 1), soci::use(db_node.rgt);
    sql << "UPDATE " << DATABASE_NODE_TABLE << " SET lft = lft - :width WHERE lft > :rgt", soci::use(db_node.rgt - db_node.lft + 1), soci::use(db_node.rgt);
    */
    
    //sql.commit();
  }

  std::string database_connection::get_path(const node db_node)
  {
    std::vector<std::string> path_vector(BATCH_SIZE);		
    std::string result = "";

    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }

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

    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }

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

  bool database_connection::node_is_leaf(const node db_node)
  {
    if ((db_node.rgt - db_node.lft) == 1)
    {
      return true;
    }

    else
    {
      return false;
    }
  }

  bool database_connection::attribute_exists (const node db_node, const std::string key)
  {		
    boost::optional<std::string> attribute_value;

    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }
    
    sql << "SELECT value FROM " << DATABASE_ATTRIBUTES_TABLE << " WHERE node_id = :id AND key = :key", soci::use(db_node.id), soci::use(key), soci::into(attribute_value);

    return (attribute_value.is_initialized());
  }

  bool database_connection::attribute_is_vector (const node db_node, const std::string key)
  {
    std::string is_vector;

    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }
    
    sql << "SELECT is_vector FROM " <<  DATABASE_ATTRIBUTES_TABLE << " WHERE node_id = :id AND key = :key", soci::use(db_node.id), soci::use(key), soci::into(is_vector);

    if (is_vector == "t")
    {
      return true;
    }

    else if (is_vector == "f")
    {
      return false;
    }

    else
    {
      return false;
    }
  }

  std::string database_connection::get_attribute (const node db_node, const std::string key)
  {
    std::string value;

    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }
    
    sql << "SELECT value FROM " << DATABASE_ATTRIBUTES_TABLE " WHERE node_id = :id AND key = :key", soci::use(db_node.id), soci::use(key), soci::into(value);

    return value;
  }

  void database_connection::set_attribute (const node db_node, const std::string key, const std::string value)
  {
    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }
    
    soci::procedure stored_procedure = (sql.prepare << "set_attribute(:node_id, :key, :value, 'f')", soci::use(db_node.id), soci::use(key), soci::use(value));
    stored_procedure.execute(true);
    
    //sql << "INSERT INTO " << DATABASE_ATTRIBUTES_TABLE << " VALUES (:node_id, :key, :value, 'f')", soci::use(db_node.id), soci::use(key), soci::use(value);
  }

  void database_connection::get_vector_attribute ( const node db_node, std::vector<std::string> &ret, const std::string key)
  {	
    std::vector<std::string> values(BATCH_SIZE);

    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }

    soci::statement statement = 
      (
      sql.prepare << "SELECT value FROM " << DATABASE_ATTRIBUTES_TABLE << " WHERE node_id = :id AND key =:key", soci::use(db_node.id), soci::use(key), soci::into(values)
      );

    statement.execute();

    while(statement.fetch())
    {
      for (std::vector<std::string>::iterator i = values.begin(); i != values.end(); i++)
      {
        ret.push_back(*i);
      }

      values.resize(BATCH_SIZE);
    }
  }

  void database_connection::set_vector_attribute (const node db_node, const std::string key, std::vector<std::string> values)
  {
    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }
    
    std::cout << "set_vector_attribute" << std::endl;

    std::string value;
    //soci::statement statement = 
    //  (
    //  sql.prepare << "INSERT INTO " << DATABASE_ATTRIBUTES_TABLE << " VALUES (:node_id, :key, :value, 't')", soci::use(db_node.id), soci::use(key), soci::use(value)
    //  );
      
      
    soci::procedure stored_procedure = (sql.prepare << "set_attribute(:node_id, :key, :value, 't')", soci::use(db_node.id), soci::use(key), soci::use(value));
   

    for (std::vector<std::string>::iterator i = values.begin(); i != values.end(); i++)
    {
      value = *i;
      stored_procedure.execute(true);
    }

  }

  void database_connection::remove_attribute (const node db_node, const std::string key)
  {
    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }
    
    sql << "DELETE FROM " << DATABASE_ATTRIBUTES_TABLE << " WHERE node_id = :id AND key = :key", soci::use(db_node.id), soci::use(key);
  }

  void database_connection::list_attributes (std::vector<std::string> &ret, const node db_node)
  {
    std::vector<std::string> keys(BATCH_SIZE);

    soci::session sql(*pool);
    
    // ======================================
    // = Check if the session is connected  =
    // ======================================
    
    if (!sql.is_connected())
    {
      sql.open(soci::postgresql, connectString);
    }

    soci::statement statement = 
      (
      sql.prepare << "SELECT key FROM " << DATABASE_ATTRIBUTES_TABLE << " WHERE node_id = :id", soci::use(db_node.id), soci::into(keys) 
      );

    statement.execute();

    while(statement.fetch())
    {
      
      for (std::vector<std::string>::iterator i = keys.begin(); i != keys.end(); i++)
      {
        ret.push_back(*i);
      }

      keys.resize(BATCH_SIZE);
    }

  }

  void database_connection::find_attributes (std::vector<std::string> &ret, const node db_node, const std::string key_pattern, const std::string value_pattern)
  {
      soci::session sql(*pool);

      // ======================================
      // = Check if the session is connected  =
      // ======================================

      if (!sql.is_connected())
      {
        sql.open(soci::postgresql, connectString);
      }
      
      // ===================================
      // = Prepare the batch result vector =
      // ===================================
      std::vector<std::string> keys(BATCH_SIZE);
      
      // ==============================
      // = Prepare the soci statement =
      // ==============================
      soci::statement statement = 
        (
          sql.prepare << "SELECT DISTINCT key FROM " << DATABASE_ATTRIBUTES_TABLE << " WHERE node_id = :id AND key SIMILAR TO :key_pattern OR value SIMILAR TO :value_pattern",
          soci::use(db_node.id), 
          soci::use(key_pattern), 
          soci::use(value_pattern),
          soci::into(keys)
        );
      
      statement.execute();
      
      while(statement.fetch())
      {
        for (std::vector<std::string>::iterator i = keys.begin(); i != keys.end(); i++)
        {
          ret.push_back(*i);
        }
        
        keys.resize(BATCH_SIZE);
      }
  }
}