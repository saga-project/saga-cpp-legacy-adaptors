#include "sql_async_server_connection.hpp"

namespace sql_async_advert
{
  server_connection::server_connection (saga::url const &url) : resolver(io_service), socket(io_service)
  {
   
    //
    // Connect to the server in sync mode
    // and get the node data. After that listen 
    // for updates async. 
    //
   
    boost::asio::ip::tcp::resolver::query query(url.get_host(), boost::lexical_cast<std::string>(url.get_port()));
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
    boost::asio::connect(socket, endpoint_iterator);
    
    boost::asio::streambuf response;
    std::istream response_stream(&response);
     
    boost::asio::read_until(socket, response, "\r\n");
   
    std::string handshake;
    response_stream >> handshake;
    
    response.consume(response.size());

    if (handshake != "hello")
    {
      throw std::runtime_error("This is not a AsyncAdvert server");
    }
    
    boost::asio::streambuf request;
    std::ostream request_stream(&request);

    JsonBox::Object obj;
    obj["command"]  = JsonBox::Value("open");
    obj["path"]     = JsonBox::Value(url.get_path());
    
    JsonBox::Value json_request(obj);
    
    request_stream << json_request;
    boost::asio::write(socket, request);
    
    boost::asio::read_until(socket, response, "\r\n");
    
    JsonBox::Value node_data(response_stream);
    
    std::cout << node_data << std::endl;
    std::cout << response.size() << std::endl;
  }
  
  server_connection::~server_connection (void)
  {
    
  }
  
  void server_connection::resolve_handler(const boost::system::error_code &error, boost::asio::ip::tcp::resolver::iterator i)
  {
    if (!error)
    {
      
    }
  }
}