#include "sql_async_server_connection.hpp"
#include "zhelpers.hpp"

namespace sql_async_advert
{
  server_connection::server_connection (saga::url const &url)
    : _url(url), _context(1), _socket(_context, ZMQ_REQ), _socket_rh(_context, ZMQ_PAIR)
  {
    
    _update_map   = new update_map_t();
    _id_map       = new id_map_t();
    _node_map     = new node_map_t();
    
    std::string connect_string = "tcp://" + _url.get_host() + ":" + "5557";
    _connect_string = connect_string;
    
    // =============================
    // = Connect to Node.js server =
    // =============================
    _socket.connect(connect_string.c_str());
    
    // ========================================
    // = Bind socket for thread communication =
    // ========================================
    _socket_rh.bind("inproc://#1");
    
    // ================================
    // = create the subscriber thread =
    // ================================
    _thread = boost::thread(boost::bind(&server_connection::read_handler, this, &_context));
  }
  
  server_connection::~server_connection (void)
  {
    std::cout << "server_connection destructor" << std::endl;
    
    s_sendmore(_socket_rh, "");
    s_send(_socket_rh, "kill");
    
    _thread.join();
    
    delete _update_map;
    delete _node_map;
  }
    
  // ===================================================================================================================================
  // = ZeroMQ async read_handler                                                                                                               =
  // ===================================================================================================================================
  
  void server_connection::read_handler(zmq::context_t *context)
  {
    zmq::socket_t subscriber(*context, ZMQ_SUB);
    
    std::string connect_string = "tcp://" + _url.get_host() + ":" + "5558";
    subscriber.connect(connect_string.c_str());
    
    zmq::socket_t receiver(*context, ZMQ_PAIR);
    receiver.connect("inproc://#1");
    
    zmq::pollitem_t items [] = 
    {
      { subscriber, 0, ZMQ_POLLIN, 0}, 
      { receiver,   0, ZMQ_POLLIN, 0}
    };
    
    while(1)
    {
      zmq::poll (items, 2);
      
      if (items [0].revents & ZMQ_POLLIN)
      {
        std::string id   = s_recv(subscriber);
        std::string type = s_recv(subscriber);
        
        //std::cout << "sub received path : " << id   << std::endl;
        //std::cout << "sub received type : " << type << std::endl;
        
        if (type == "updated")
        {
          _mutex.lock();
          {
            (*_update_map)[id] = true;
          }
          _mutex.unlock();
        }
        
        if (type == "removed")
        {
          subscriber.setsockopt(ZMQ_UNSUBSCRIBE, id.c_str(), id.size());
          
          _mutex.lock();
          {
            update_map_t::iterator i = _update_map->find(id);
            
            if (i != _update_map->end())
            {
              _update_map->erase(i);
            }
          } 
          _mutex.unlock();
        }
      }
      
      if (items [1].revents & ZMQ_POLLIN)
      { 
        std::string id    = s_recv(receiver);
        std::string type  = s_recv(receiver);
        
        if ( type == "sub")
        {  
          subscriber.setsockopt(ZMQ_SUBSCRIBE, id.c_str(), id.size());
        }
        
        if (type == "unsub")
        {
          subscriber.setsockopt(ZMQ_UNSUBSCRIBE, id.c_str(), id.size());
          
          _mutex.lock();
          {
            update_map_t::iterator i = _update_map->find(id);
            
            if (i != _update_map->end())
            {
              _update_map->erase(i);
            }
          } 
          _mutex.unlock();
        }
      
        if (type == "kill")
        {
          break;
        }
      }
    
    }
  }
    
  // ===================================================================================================================================
  // = Public methods                                                                                                                  =
  // ===================================================================================================================================
  
  const bool server_connection::get_value(const std::string &url, Json::Value &ret)
  {
    //std::cout << "get_value" << std::endl;
    
    std::string id      = (*_id_map)[url];
    bool needs_update   = false;
    bool state          = true; 
  
    _mutex.lock();
    {
      update_map_t::iterator i = _update_map->find(id); 
      
      // ================================
      // = check if the node was closed =
      // ================================
      
      if (i == _update_map->end())
      {
        //std::cout << "not found" << std::endl;
        state = false;
      }
      
      // =========================================
      // = check if the node needs to be updated =
      // =========================================
      
      if (i != _update_map->end())
      {
        //std::cout << "found" << std::endl;
        needs_update = i->second;
        i->second = false;
      }
      
    }
    _mutex.unlock();
    
    // ================================
    // = if needs update send request =
    // ================================
    
    //std::cout << needs_update << std::endl;
    
    if (needs_update)
    {
      Json::Value value;
      Json::FastWriter writer;

      value["id"]     = Json::Value(id);
      
      s_sendmore(_socket, "get");
      s_send(_socket, writer.write(value));

      std::string status  = s_recv(_socket);
      std::string data    = s_recv(_socket);
      
      //std::cout << "status : " << status << std::endl;
      
      if (status == "ok")
      {
        Json::Value node;
        Json::Reader reader;

        reader.parse(data, node);

        (*_node_map)[url] = node;
      }
        
      if (status == "error") 
      {
        state = false;
      } 
    }
    
    // =================
    // = deliver value =
    // =================
     
    if (state)
    {
      ret = (*_node_map)[url];
    } 
    
    //std::cout << "state : " << state << std::endl;
    return state;
  }
  
  const bool server_connection::get_state(const std::string &url)
  {
    //std::cout << "get_state" << std::endl;
    
    std::string id  = (*_id_map)[url];
    bool state      = true;
    
    _mutex.lock();
    {
      update_map_t::iterator i = _update_map->find(id); 
      
      // ================================
      // = check if the node was closed =
      // ================================
      
      if (i == _update_map->end())
      {
        //std::cout << "not found" << std::endl;
        state = false;
      }
    }
    _mutex.unlock();

    return state; 
  }
  
  bool server_connection::exists_directory(const std::string &url)
  { 
    //std::cout << "exists_directory" << std::endl;
        
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    s_sendmore(_socket, "exists");
    s_send(_socket, writer.write(value));

    std::string status= s_recv(_socket);
    
    if (status == "true")
    {
      return true;
    }
    
    else 
    {
      return false;
    }
  }
  
  void server_connection::create_directory(const std::string &url, const bool dir)
  {
    //std::cout << "create_directory" << std::endl;
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["dir"]      = Json::Value(dir);
    
    s_sendmore(_socket, "create");
    s_send(_socket, writer.write(value));
    
    std::string status  = s_recv(_socket);
    std::string id      = s_recv(_socket);

    if (status == "ok")
    {
      (*_id_map)[url] = id;
      
      _mutex.lock();
      {
        (*_update_map)[id] = true;
      }
      _mutex.unlock();
    
      s_sendmore(_socket_rh, id);
      s_send(_socket_rh, "sub");
    }
  }
   
  void server_connection::create_parents_directory(const std::string &url, const bool dir)
  {
    //std::cout << "create_parents_directory" << std::endl;
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["dir"]      = Json::Value(dir);
    
    s_sendmore(_socket, "createParents");
    s_send(_socket, writer.write(value));
    
    std::string status  = s_recv(_socket);
    std::string id      = s_recv(_socket);
    
    if (status == "ok")
    {
      (*_id_map)[url] = id;
      
      _mutex.lock();
      {
        (*_update_map)[id] = true;
      }
      _mutex.unlock();
      
      s_sendmore(_socket_rh, id);
      s_send(_socket_rh, "sub");
    }
  }
   
  void server_connection::open_directory(const std::string &url)
  {
    //std::cout << "open_directory" << std::endl;
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    s_sendmore(_socket, "open");
    s_send(_socket, writer.write(value));
    
    std::string status  = s_recv(_socket);
    std::string id      = s_recv(_socket);
    
    if (status == "ok")
    {
      (*_id_map)[url] = id;
      
      _mutex.lock();
      {
        (*_update_map)[id] = true;
      }
      _mutex.unlock();
      
      s_sendmore(_socket_rh, id);
      s_send(_socket_rh, "sub");
    }
  }
  
  void server_connection::remove_directory(const std::string &url)
  {
    //std::cout << "remove_directory" << std::endl;
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    s_sendmore(_socket, "remove");
    s_send(_socket, writer.write(value));
    
    std::string status  = s_recv(_socket);
    
    if (status == "ok")
    {
      std::string id = (*_id_map)[url];
      
      s_sendmore(_socket_rh, id);
      s_send(_socket_rh, "unsub");
    }
  }
  
  void server_connection::close_directory(const std::string &url)
  { 
    std::string id = (*_id_map)[url];
    
    s_sendmore(_socket_rh, id);
    s_send(_socket_rh, "unsub");
  }
  
  void server_connection::set_attribute(const std::string &url, const std::string &key, const std::string &_value)
  {
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["key"]      = Json::Value(key);
    value["value"]    = Json::Value(_value);
    
    s_sendmore(_socket, "setAttribute");
    s_send(_socket, writer.write(value));
    
    std::string status  = s_recv(_socket);
    std::string id      = s_recv(_socket);
    
    if (status == "ok")
    {
      (*_id_map)[url] = id;
      
      _mutex.lock();
      {
        (*_update_map)[id] = true;
      }
      _mutex.unlock();
    }
  }
  
  void server_connection::set_vector_attribute(const std::string &url, const std::string &key, std::vector<std::string> &_value)
  {
    Json::Value array;
    
    for (std::vector<std::string>::iterator i = _value.begin(); i != _value.end(); ++i)
    {
      array.append(*i);
    }
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["key"]      = Json::Value(key);
    value["value"]    = array;
    
    s_sendmore(_socket, "setAttribute");
    s_send(_socket, writer.write(value));
    
    std::string status  = s_recv(_socket);
    std::string id      = s_recv(_socket);
    
    if (status == "ok")
    {
      (*_id_map)[url] = id;
      
      _mutex.lock();
      {
        (*_update_map)[id] = true;
      }
      _mutex.unlock();
    }
  }
  
  void server_connection::remove_attribute(const std::string &url, const std::string &key)
  {
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["key"]      = Json::Value(key); 
    
    s_sendmore(_socket, "removeAttribute");
    s_send(_socket, writer.write(value));
    
    std::string status  = s_recv(_socket);
    std::string id      = s_recv(_socket);
    
    if (status == "ok")
    {
      (*_id_map)[url] = id;
      
      _mutex.lock();
      {
        (*_update_map)[id] = true;
      }
      _mutex.unlock();
    }
  }
  
  // ===================================================================================================================================
  // = Private methods                                                                                                                 =
  // ===================================================================================================================================
  
  void server_connection::erase_node(const std::string &url)
  { 
    _mutex.lock();
    
    node_map_t::iterator i = _node_map->find(url);
    
    if (i != _node_map->end())
    {
      _node_map->erase(i);
    }
    
    _mutex.unlock();
  }
  
  void server_connection::send_message(const std::string &command, const Json::Value &value)
  {
    Json::FastWriter writer;
    
    std::cout << s_sendmore(_socket, command) << std::endl;
    std::cout << s_send(_socket, writer.write(value)) << std::endl;
  }

}