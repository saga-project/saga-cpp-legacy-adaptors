#include "sql_async_server_connection.hpp"

namespace sql_async_advert
{
  server_connection::server_connection (saga::url const &url, boost::asio::io_service &io_service)
    : _resolver(io_service), _socket(io_service), _response_stream(&_response), _request_stream(&_request)
  {
    
    _node_map = new node_map_t();
    _url = url.clone();
   
    //
    // Connect to the server in sync mode
    // and get the node data. After that listen 
    // for updates async. 
    //
   
    // ========================
    // = Resolve and connect  =
    // ========================
    
    try
    {
      boost::asio::ip::tcp::resolver::query query(url.get_host(), boost::lexical_cast<std::string>(url.get_port()));
      boost::asio::ip::tcp::resolver::iterator endpoint_iterator = _resolver.resolve(query);
      boost::asio::connect(_socket, endpoint_iterator);
    }
    
    catch (boost::system::system_error)
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT("Unable to connect to " + url.get_string() + " Server is not running", saga::NoSuccess);
    }

    // ===================
    // = read handshake  =
    // ===================
    boost::asio::read_until(_socket, _response, "\r\n");
   
    std::string handshake;
    _response_stream >> handshake;
    _response.consume(_response.size());
    
    std::cout << handshake << std::endl;

    // ===============================================
    // = Check if the server is a AsyncAdvert server =
    // ===============================================
    if (handshake != "AsyncAdvertServer")
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT("Unable to connect. No AsyncAdvertServer found", saga::NoSuccess);
    }
    
    // ====================
    // = Start async read =
    // ====================
    boost::asio::async_read_until(_socket, _response, "\r\n", boost::bind(&server_connection::read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
  }
  
  server_connection::~server_connection (void)
  {
    
  }
  
  void server_connection::read_handler(const boost::system::error_code &error, std::size_t bytes)
  {
    if(!error)
    {
      JsonBox::Value data = JsonBox::Value(_response_stream);
      _response.consume(_response.size());
      
      JsonBox::Object obj = data.getObject();
      
      if (obj["command"].getString() == "exists")
      {
        _node_exists.set_value(obj["data"].getBoolean());
      }
      
      if (obj["command"].getString() == "updated")
      {
        JsonBox::Object nodeObj = obj["data"].getObject();
        
        _mutex.lock();
        {
          node_map_t::iterator i = _node_map->find(nodeObj["path"].getString());
          
          if (i != _node_map->end())
          {
            i->second->value = nodeObj;
          
            if ( !(i->second->future.is_ready()) )
            {
              i->second->promise.set_value(true);
            }
          }  
        }
        _mutex.unlock();
      }

      if (obj["command"].getString() == "removed")
      {
        node_map_t::iterator i = _node_map->find(obj["data"].getString());
        _node_map->erase(i);
      }

      boost::asio::async_read_until(_socket, _response, "\r\n", boost::bind(&server_connection::read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    
    else
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT(error.message(), saga::NoSuccess);
    }
  }
  
  boost::mutex& server_connection::get_mutex()
  {
    return _mutex;
  }
  
  // =================================================================================
  // = Get a copy of the JsonBox::Value and return if the value is in a opend state  =
  // =================================================================================
  
  const bool server_connection::get_value(const std::string &url, JsonBox::Value &ret)
  {
    bool state = false;
    
    _mutex.lock();
    {
      node_map_t::iterator i = _node_map->find(url);
      
      if ( i != _node_map->end())
      {
        i->second->future.get();
        
        state = true; 
        ret   = JsonBox::Value(i->second->value);
      }
      
    }
    _mutex.unlock();
    
    return state;
  }
  
  const bool server_connection::get_state(const std::string &url)
  {
    bool state = false;
    
    _mutex.lock();
    {
      node_map_t::iterator i = _node_map->find(url);
      
      if ( i != _node_map->end())
      {
        i->second->future.get();
        state = true; 
      }
      
    }
    _mutex.unlock();
    
    return state; 
  }
  
  bool server_connection::exists_directory(const std::string &url)
  {
    _node_exists = boost::promise<bool>();
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("exists");
    obj["path"]     = JsonBox::Value(url);
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
    
    boost::unique_future<bool> future = _node_exists.get_future();
    return future.get();
  }
  
  void server_connection::create_directory(const std::string &url)
  {
    (*_node_map)[url] = new promise_value();
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("create");
    obj["path"]     = JsonBox::Value(url);
    obj["dir"]      = JsonBox::Value(true);
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
  }
   
  void server_connection::create_parents_directory(const std::string &url)
  {
    (*_node_map)[url] = new promise_value();
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("createParents");
    obj["path"]     = JsonBox::Value(url);
    obj["dir"]      = JsonBox::Value(true);
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
  }
   
  void server_connection::open_directory(const std::string &url)
  {
    (*_node_map)[url] = new promise_value();
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("open");
    obj["path"]     = JsonBox::Value(url);
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
  }
  
  void server_connection::set_attribute(const std::string &url, const std::string &key, const std::string &value)
  {
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("setAttribute");
    obj["path"]     = JsonBox::Value(url);
    obj["key"]      = JsonBox::Value(key);
    obj["value"]    = JsonBox::Value(value);
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
  }
  
  void server_connection::set_vector_attribute(const std::string &url, const std::string &key, std::vector<std::string> &value)
  {
    JsonBox::Array array;
    
    for (std::vector<std::string>::iterator i = value.begin(); i != value.end(); ++i)
    {
      array.push_back(*i);
    }
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("setAttribute");
    obj["path"]     = JsonBox::Value(url);
    obj["key"]      = JsonBox::Value(key);
    obj["value"]    = array;
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request); 
  }
  
  void server_connection::remove_attribute(const std::string &url, const std::string &key)
  {
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("setAttribute");
    obj["path"]     = JsonBox::Value(url);
    obj["key"]      = JsonBox::Value(key); 
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
  }
}