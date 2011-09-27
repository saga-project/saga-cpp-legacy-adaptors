#include "sql_async_server_connection.hpp"
#include "zhelpers.hpp"

namespace sql_async_advert
{
  server_connection::server_connection (saga::url const &url)
    : _url(url), _context(1), _socket(_context, ZMQ_REQ), _socket_rh(_context, ZMQ_PAIR)   
  {
    _node_map = new node_map_t();
    
    std::string connect_string = "tcp://" + _url.get_host() + ":" + "5557";
    _connect_string = connect_string;
    
    // =============================
    // = Connect to Node.js server =
    // =============================
    //_socket.connect(connect_string.c_str());
    
    // ========================================
    // = Bind socket for thread communication =
    // ========================================
    _socket_rh.bind("inproc://read_handler");
    
    // ================================
    // = create the subscriber thread =
    // ================================
    _thread = boost::thread(boost::bind(&server_connection::read_handler, this, &_context));
  }
  
  server_connection::~server_connection (void)
  {
    delete _node_map;
  }
    
  // ===================================================================================================================================
  // = ZeroMQ async read_handler                                                                                                               =
  // ===================================================================================================================================
  
  void server_connection::read_handler(zmq::context_t *context)
  {
    zmq::socket_t subscriber(*context, ZMQ_SUB);
    
    std::string connect_string = "tcp://" + _url.get_host() + ":" + "5556";
    subscriber.connect(connect_string.c_str());
    
    zmq::socket_t receiver(*context, ZMQ_PAIR);
    receiver.connect("inproc://read_handler");
    
    std::cout << "read_handler running " << std::endl;
    
    zmq::pollitem_t items [] = 
    {
      { subscriber, 0, ZMQ_POLLIN, 0}, 
      { receiver  , 0, ZMQ_POLLIN, 0}
    };
    
    while(1)
    {
      zmq::poll (&items[0], 2, -1);
      
      if (items [0].revents & ZMQ_POLLIN)
      {
        std::string path = s_recv(subscriber);
        std::string type = s_recv(subscriber);
        std::string data = s_recv(subscriber);
        
        //std::cout << "sub received path : " << path << std::endl;
        //std::cout << "sub received type : " << type << std::endl;
        //std::cout << "sub received data : " << data << std::endl;
        
        if (type == "updated")
        {
          _mutex.lock();
          {
            Json::Value node;
            Json::Reader reader;
            
            reader.parse(data, node);

            (*_node_map)[path] = node;
          }
          _mutex.unlock();
        }
        
        if (type == "removed")
        {
          erase_node(path);
          subscriber.setsockopt(ZMQ_UNSUBSCRIBE, path.c_str(), path.size());
          //std::cout << "unsub to channel " << path << std::endl;    
        }
      }
      
      if (items [1].revents & ZMQ_POLLIN)
      {
        if (s_recv(receiver) == "subscribe")
        {
          std::string path = s_recv(receiver);
          subscriber.setsockopt(ZMQ_SUBSCRIBE, path.c_str(), path.size());
          
          std::cout << "sub to channel " << path << std::endl;
        }
        
        if (s_recv(receiver) == "unsubscribe")
        {
          std::string path = s_recv(receiver);
          subscriber.setsockopt(ZMQ_UNSUBSCRIBE, path.c_str(), path.size());
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
    
    bool state = false;

    _mutex.lock();
    {
      node_map_t::iterator i = _node_map->find(url);

      if ( i != _node_map->end())
      {
        state = true;
        ret   = i->second;
      }
    }
    _mutex.unlock();
    
    return state;
  }
  
  const bool server_connection::get_state(const std::string &url)
  {
    //std::cout << "get_state" << std::endl;
    
    bool state = false;
    
    _mutex.lock();
    {
      node_map_t::iterator i = _node_map->find(url);

      if ( i != _node_map->end())
      {
        state = true;
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
    
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "exists");
    s_send(socket, writer.write(value));

    std::string status= s_recv(socket);
    
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
     
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "create");
    s_send(socket, writer.write(value));
    
    std::string status  = s_recv(socket);
    std::string data    = s_recv(socket);

    std::cout << status << data << std::endl;

    if (status == "ok")
    {
      _mutex.lock();
      { 
        Json::Value node;
        Json::Reader reader;

        reader.parse(data, node);
        
        (*_node_map)[url] = node;
      }
      _mutex.unlock();
    
    
      s_sendmore(_socket_rh, "subscribe");
      s_send(_socket_rh, url);
    }
    

  }
   
  void server_connection::create_parents_directory(const std::string &url, const bool dir)
  {
    //std::cout << "create_parents_directory" << std::endl;
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["dir"]      = Json::Value(dir);
    
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "createParents");
    s_send(socket, writer.write(value));
    
    std::string status  = s_recv(socket);
    std::string data    = s_recv(socket);
    
    if (status == "ok")
    {
      _mutex.lock();
      { 
        Json::Value node;
        Json::Reader reader;

        reader.parse(data, node);
        
        (*_node_map)[url] = node;
      }
      _mutex.unlock();
      
      s_sendmore(_socket_rh, "subscribe");
      s_send(_socket_rh, url);
    }
  }
   
  void server_connection::open_directory(const std::string &url)
  {
    //std::cout << "open_directory" << std::endl;
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "open");
    s_send(socket, writer.write(value));
    
    std::string status  = s_recv(socket);
    std::string data    = s_recv(socket);
    
    if (status == "ok")
    {
      _mutex.lock();
      { 
        Json::Value node;
        Json::Reader reader;

        reader.parse(data, node);
        
        (*_node_map)[url] = node;
      }
      _mutex.unlock();
      
      s_sendmore(_socket_rh, "subscribe");
      s_send(_socket_rh, url);
    }
  }
  
  void server_connection::remove_directory(const std::string &url)
  {
    //std::cout << "remove_directory" << std::endl;
    
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "remove");
    s_send(socket, writer.write(value));
    
    std::string status  = s_recv(socket);
    
    s_sendmore(_socket_rh, "unsubscribe");
    s_send(_socket_rh, url);
    
    erase_node(url);
  }
  
  void server_connection::close_directory(const std::string &url)
  { 
    s_sendmore(_socket_rh, "unsubscribe");
    s_send(_socket_rh, url);
    
    erase_node(url);
  }
  
  void server_connection::set_attribute(const std::string &url, const std::string &key, const std::string &_value)
  {
    Json::Value value;
    Json::FastWriter writer;
    
    value["path"]     = Json::Value(url);
    value["key"]      = Json::Value(key);
    value["value"]    = Json::Value(_value);
    
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "setAttribute");
    s_send(socket, writer.write(value));
    
    std::string status  = s_recv(socket);
    std::string data    = s_recv(socket);
    
    if (status == "ok")
    {
      _mutex.lock();
      { 
        Json::Value node;
        Json::Reader reader;

        reader.parse(data, node);
        
        (*_node_map)[url] = node;
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
    
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "setAttribute");
    s_send(socket, writer.write(value));
    
    std::string status  = s_recv(socket);
    std::string data    = s_recv(socket);
    
    if (status == "ok")
    {
      _mutex.lock();
      { 
        Json::Value node;
        Json::Reader reader;

        reader.parse(data, node);
        
        (*_node_map)[url] = node;
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
    
    zmq::socket_t socket(_context, ZMQ_REQ);
    socket.connect(_connect_string.c_str());
    
    s_sendmore(socket, "removeAttribute");
    s_send(socket, writer.write(value));
    
    std::string status  = s_recv(socket);
    std::string data    = s_recv(socket);
    
    if (status == "ok")
    {
      _mutex.lock();
      { 
        Json::Value node;
        Json::Reader reader;

        reader.parse(data, node);
        
        (*_node_map)[url] = node;
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