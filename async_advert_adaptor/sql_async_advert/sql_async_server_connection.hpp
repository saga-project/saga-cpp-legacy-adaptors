#ifndef SQL_ASYNC_SERVER_CONNECTION_HPP
#define SQL_ASYNC_SERVER_CONNECTION_HPP

// Boost Includes
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

// STL Includes
#include <iostream>
#include <string>
#include <vector>

// JSON Includes
#include "Value.h"

// Saga Includes
#include <saga/url.hpp>
#include <saga/saga/exception.hpp>
#include <saga/impl/exception.hpp>

namespace sql_async_advert
{
  class server_connection
  {
  public:
    
    // ===============
    // = Constructor =
    // ===============
    
    server_connection (saga::url const &url, boost::asio::io_service &io_service, boost::thread &thread);
    
    // ==============
    // = Destructor =
    // ==============
    
    ~server_connection (void);
    
  private:
    
    // ===================
    // = Private members =
    // ===================
    
    boost::mutex                    _mutex;
    
    boost::asio::ip::tcp::resolver  _resolver;
    boost::asio::ip::tcp::socket    _socket;
    
    boost::asio::streambuf          _response;
    boost::asio::streambuf          _request;
    
    std::istream                    _response_stream;
    std::ostream                    _request_stream;
    
    saga::url                       _url;
    JsonBox::Value                  _node;
    
    // ===================
    // = Private methods =
    // ===================
    
    void resolve_handler(const boost::system::error_code &error, boost::asio::ip::tcp::resolver::iterator i);
    
    void connect_handler(const boost::system::error_code &error);
    
    void read_handler(const boost::system::error_code &error, std::size_t bytes);
   
   public:
     
     // ==================
     // = Public methods =
     // ==================
     
     const JsonBox::Value& getNode(void) const;
     
     boost::mutex& getMutex(void);
     
     void open_node (const std::string path);
     
     void insert_node (const std::string node_name, const bool is_dir = true);
     
     void list_nodes (std::vector<std::string> &result);
     
     void set_attribute(const std::string key, const std::string value);
     
     std::string get_attribute(const std::string key);
   };
}

#endif // SQL_ASYNC_SERVER_CONNECTION_HPP