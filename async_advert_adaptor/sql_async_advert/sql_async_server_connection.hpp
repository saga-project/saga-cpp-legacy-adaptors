#ifndef SQL_ASYNC_SERVER_CONNECTION_HPP
#define SQL_ASYNC_SERVER_CONNECTION_HPP

// Boost Includes
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

// STL Includes
#include <iostream>
#include <string>
#include <vector>

// JSON Includes
#include "Value.h"

// Saga Includes
#include <saga/url.hpp>

namespace sql_async_advert
{
  class server_connection
  {
  public:
    
    // ===============
    // = Constructor =
    // ===============
    
    server_connection (saga::url const &url);
    
    // ==============
    // = Destructor =
    // ==============
    
    ~server_connection (void);
    
  private:
    
    // ===================
    // = Private members =
    // ===================
    
    boost::thread                   io_service_thread;
    
    boost::asio::io_service         io_service;
    boost::asio::ip::tcp::resolver  resolver;
    boost::asio::ip::tcp::socket    socket;
    
    boost::array<char, 4096>        buffer;
    
    saga::url url;
    JsonBox::Value node;
    
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
     
     void open_node (const std::string path);
     
     void insert_node (const std::string node_name, const bool is_dir = true);
     
     void child_nodes (std::vector<std::string> &ret);
     
     void set_attribute(const std::string key, const std::string value);
     
     std::string get_attribute(const std::string key);
   };
}

#endif // SQL_ASYNC_SERVER_CONNECTION_HPP