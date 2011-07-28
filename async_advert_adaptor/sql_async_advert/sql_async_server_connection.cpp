#include "sql_async_server_connection.hpp"

namespace sql_async_advert
{
  server_connection::server_connection (saga::url const &url)
    : _resolver(_io_service), _socket(_io_service), _response_stream(&_response), _request_stream(&_request)
  {
    _url = url.clone();
   
    //
    // Connect to the server in sync mode
    // and get the node data. After that listen 
    // for updates async. 
    //
   
    // ========================
    // = Resolve and connect  =
    // ========================
    boost::asio::ip::tcp::resolver::query query(url.get_host(), boost::lexical_cast<std::string>(url.get_port()));
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = _resolver.resolve(query);
    boost::asio::connect(_socket, endpoint_iterator);
    
     
    // ===================
    // = read handshake  =
    // ===================
    boost::asio::read_until(_socket, _response, "\r\n");
   
    std::string handshake;
    _response_stream >> handshake;
    _response.consume(_response.size());

    // ===============================================
    // = Check if the server is a AsyncAdvert server =
    // ===============================================
    if (handshake != "AsyncAdvertServer")
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT("Unable to connect. No AsyncAdvertServer found", saga::NoSuccess);
    }
    
    // ==========================
    // = request to open a node =
    // ==========================

    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("open");
    obj["path"]     = JsonBox::Value(url.get_path());
    
    JsonBox::Value json_request(obj);
    
    _request_stream << json_request;
    boost::asio::write(_socket, _request);
    
    // ======================
    // = read node response =
    // ======================
    boost::asio::read_until(_socket, _response, "\r\n");
    
    _node = JsonBox::Value(_response_stream);
    _response.consume(_response.size());

    //std::cout << _node << std::endl;

    // ====================
    // = Start async read =
    // ====================
    boost::asio::async_read_until(_socket, _response, "\r\n", boost::bind(&server_connection::read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    
    _thread = boost::thread(boost::bind(&boost::asio::io_service::run, &_io_service));
    //_thread.join();
  }
  
  server_connection::~server_connection (void)
  {
    
  }
  
  void server_connection::read_handler(const boost::system::error_code &error, std::size_t bytes)
  {
    if(!error)
    {
      _mutex.lock();
      _node = JsonBox::Value(_response_stream);
      _response.consume(_response.size());
      _mutex.unlock();
      
      boost::asio::async_read_until(_socket, _response, "\r\n", boost::bind(&server_connection::read_handler, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
    
    else
    {
      SAGA_ADAPTOR_THROW_NO_CONTEXT(error.message(), saga::NoSuccess);
    }
  }
  
  const JsonBox::Value& server_connection::getNode(void) const
  {
    return _node;
  }
  
  boost::mutex& server_connection::getMutex(void)
  {
    return _mutex;
  }
  
  void server_connection::list_nodes(std::vector<std::string> &result)
  {
    JsonBox::Object object  = _node.getObject();
    JsonBox::Array  array   = object["nodeList"].getArray();
    
    for(std::deque<JsonBox::Value>::iterator i = array.begin(); i != array.end(); i++)
    {
      result.push_back(i->getString());
    }
  }
}