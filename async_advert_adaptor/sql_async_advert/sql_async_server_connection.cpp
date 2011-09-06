#include "sql_async_server_connection.hpp"

namespace sql_async_advert
{
  server_connection::server_connection (saga::url const &url, boost::asio::io_service &io_service)
    : _resolver(io_service), _socket(io_service), _response_stream(&_response), _request_stream(&_request)
  {
    _node_map = new node_map_t();
    _url = url.clone();
    _node_opened_url = "";
   
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
    
    //std::cout << handshake << std::endl;

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
    delete _node_map; 
  }
  
  
  // ===================================================================================================================================
  // = ASIO read_handler                                                                                                               =
  // ===================================================================================================================================
  
  void server_connection::read_handler(const boost::system::error_code &error, std::size_t bytes)
  {
    if(!error)
    {
      JsonBox::Value data = JsonBox::Value(_response_stream);
      _response.consume(_response.size());
      
      //std::cout << data << std::endl;
      
      JsonBox::Object obj = data.getObject();
      
      // ==================
      // = Message exists =
      // ==================
      
      if (obj["command"].getString() == "exists")
      {
    	  _node_exists.set_value(obj["data"].getBoolean());
      }
      
      // ===================
      // = Message updated =
      // ===================
      
      if (obj["command"].getString() == "updated")
      {
        JsonBox::Object nodeObj = obj["data"].getObject();
        
        write_lock lock(_mutex);
        (*_node_map)[nodeObj["path"].getString()] = nodeObj;
        
        if (_node_opened_url == nodeObj["path"].getString())
        {
          _node_opened.set_value(true);
        }
      }

      // ===================
      // = Message removed =
      // ===================
      
      if (obj["command"].getString() == "removed")
      {
        erase_node(obj["data"].getString());
      }
      
      // =================
      // = Message error =
      // =================
      
      if (obj["command"].getString() == "error")
      {
        erase_node(obj["data"].getString());
        
        if (_node_opened_url == obj["data"].getString())
        {
          std::cout << "ERROR" << std::endl;
          _node_opened.set_value(false);
        }
      }

      // =========================
      // = Read the next Message =
      // =========================
      
      boost::asio::async_read_until(_socket, _response, "\r\n", boost::bind(&server_connection::read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    
    else
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT(error.message(), saga::NoSuccess);
    }
  }
   
  // ===================================================================================================================================
  // = Public methods                                                                                                                  =
  // ===================================================================================================================================
  
  const bool server_connection::get_value(const std::string &url, JsonBox::Value &ret)
  {
    bool state = false;

    read_lock lock(_mutex);
    node_map_t::iterator i = _node_map->find(url);
    
    if ( i != _node_map->end())
    {
      state = true;
      ret   = JsonBox::Value(i->second);
    }
      
    
    return state;
  }
  
  const bool server_connection::get_state(const std::string &url)
  {
    bool state = false;
    
    read_lock lock(_mutex);
    node_map_t::iterator i = _node_map->find(url);
    
    if ( i != _node_map->end())
    {
      state = true;
    }

    return state; 
  }
  
  bool server_connection::exists_directory(const std::string &url)
  {
    // =======================================================
    // = Return true if the node is allready in the node_map =
    // =======================================================
    
    if (get_state(url)) return true;
    
    // ======================================
    // = Else sync query if the node exists =
    // ======================================
    
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
  
  void server_connection::create_directory(const std::string &url, const bool dir)
  {
    if (get_state(url)) return;
    
    set_opened(url);
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("create");
    obj["path"]     = JsonBox::Value(url);
    obj["dir"]      = JsonBox::Value(dir);
    
    send_message(obj);
    reset_opened();
  }
   
  void server_connection::create_parents_directory(const std::string &url, const bool dir)
  {
    if (get_state(url)) return;
    
    set_opened(url);
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("createParents");
    obj["path"]     = JsonBox::Value(url);
    obj["dir"]      = JsonBox::Value(dir);
    
    send_message(obj);
    reset_opened();
  }
   
  void server_connection::open_directory(const std::string &url)
  {
    
    if (get_state(url)) return;
    
    set_opened(url);
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("open");
    obj["path"]     = JsonBox::Value(url);
    
    send_message(obj);
    reset_opened();
  }
  
  void server_connection::async_open_directory(const std::string &url)
  {
    
    if (get_state(url)) return;
    
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("open");
    obj["path"]     = JsonBox::Value(url);
    
    send_message(obj);
  }
  
  void server_connection::remove_directory(const std::string &url)
  {
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("remove");
    obj["path"]     = JsonBox::Value(url);
    
    send_message(obj);
  }
  
  void server_connection::close_directory(const std::string &url)
  {
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("close");
    obj["path"]     = JsonBox::Value(url);
    
    send_message(obj);
    erase_node(url);
  }
  
  void server_connection::set_attribute(const std::string &url, const std::string &key, const std::string &value)
  {
    set_opened(url);
  
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("setAttribute");
    obj["path"]     = JsonBox::Value(url);
    obj["key"]      = JsonBox::Value(key);
    obj["value"]    = JsonBox::Value(value);
    
    JsonBox::Value json_request(obj);
    
    send_message(obj);
    reset_opened();
  }
  
  void server_connection::set_vector_attribute(const std::string &url, const std::string &key, std::vector<std::string> &value)
  {
    set_opened(url);
  
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
    
    send_message(obj);
    reset_opened();
  }
  
  void server_connection::remove_attribute(const std::string &url, const std::string &key)
  {
    set_opened(url);
  
    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("removeAttribute");
    obj["path"]     = JsonBox::Value(url);
    obj["key"]      = JsonBox::Value(key); 
    
    send_message(obj);
    reset_opened();
  }
  
  // ===================================================================================================================================
  // = Private methods                                                                                                                 =
  // ===================================================================================================================================
  
  void server_connection::erase_node(const std::string &url)
  { 
    write_lock lock(_mutex);
    
    node_map_t::iterator i = _node_map->find(url);
    
    if (i != _node_map->end())
    {
      _node_map->erase(i);
    }
  }
  
  void server_connection::send_message(const JsonBox::Object &obj)
  {
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
  }
  
  void server_connection::set_opened(const std::string &url)
  {
    _node_opened_url = url;
    _node_opened = boost::promise<bool>();
  }
  
  void server_connection::reset_opened(void)
  { 
    boost::unique_future<bool> future = _node_opened.get_future();
    //future.get();
    
    boost::posix_time::time_duration td = boost::posix_time::seconds(30);
    future.timed_wait(td);
    
    _node_opened_url = "";
  }
}