#include "sql_async_server_connection.hpp"
#include "zhelpers.hpp"

namespace sql_async_advert
{
  server_connection::server_connection (saga::url const &url)
    : _url(url), _context(1), _socket_rh(_context, ZMQ_PAIR)
  {
    
    _update_map   = new update_map_t();
    _id_map       = new id_map_t();
    _node_map     = new node_map_t();
    
    std::string connect_string = "tcp://" + _url.get_host() + ":" + "5557";
    _connect_string = connect_string;
    
    // =============================
    // = Connect to Node.js server =
    // =============================
    
    // =================================================================================
    // = Cause a zmq socket is not thread safe we don't use a gloabal socket anymore ! =
    // =================================================================================
    
    //_socket.connect(connect_string.c_str());
    
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
        state = false;
      }
      
      // =========================================
      // = check if the node needs to be updated =
      // =========================================
      
      if (i != _update_map->end())
      {
        needs_update = i->second;
        i->second = false;
      }
      
    }
    _mutex.unlock();
    
    // ================================
    // = if needs update send request =
    // ================================
    
    if (needs_update)
    { 
      Json::Value value;
      Json::FastWriter writer;

      value["id"]     = Json::Value(id);
      
      zmq::socket_t req_socket(_context, ZMQ_REQ);
      req_socket.connect(_connect_string.c_str());
      
      s_sendmore(req_socket, "get");
      s_send(req_socket, writer.write(value));

      std::string status  = s_recv(req_socket);
      std::string data    = s_recv(req_socket);
      
      if (status == "ok")
      {
        Json::Value   node;
        Json::Reader  reader;

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
    
    return state;
  }
  
  const bool server_connection::get_state(const std::string &url)
  {
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
        state = false;
      }
    }
    _mutex.unlock();

    return state; 
  }
  
  bool server_connection::exists_directory(const std::string &url)
  {     
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "exists");
    s_send(req_socket, writer.write(value));

    std::string status= s_recv(req_socket);
    
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
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["dir"]      = Json::Value(dir);
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "create");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);

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
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["dir"]      = Json::Value(dir);
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "createParents");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);
    
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
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "open");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);
    
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
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "remove");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    
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
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "setAttribute");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);
    
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
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "setAttribute");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);
    
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
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "removeAttribute");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);
    
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
  
  void server_connection::set_string(const std::string &url, const std::string &data)
  {
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["data"]     = Json::Value(data);
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "setString");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);
    
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
  
  void server_connection::remove_string(const std::string &url)
  {
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    zmq::socket_t req_socket(_context, ZMQ_REQ);
    req_socket.connect(_connect_string.c_str());
    
    s_sendmore(req_socket, "removeString");
    s_send(req_socket, writer.write(value));
    
    std::string status  = s_recv(req_socket);
    std::string id      = s_recv(req_socket);
    
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
  
//  void server_connection::send_message(const std::string &command, const Json::Value &value)
//  {
//    Json::FastWriter writer;
//    
//    std::cout << s_sendmore(_socket, command) << std::endl;
//    std::cout << s_send(_socket, writer.write(value)) << std::endl;
//  }

}