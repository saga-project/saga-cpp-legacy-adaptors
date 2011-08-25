#ifndef SQL_ASYNC_SERVER_CONNECTION_HPP
#define SQL_ASYNC_SERVER_CONNECTION_HPP

// Boost Includes
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

// STL Includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>

// JSON Includes
#include <JsonBox.h>

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
    
    server_connection (saga::url const &url, boost::asio::io_service &io_service);
    
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
    
    
    struct promise_value
    {
      boost::promise<bool>        promise;
      boost::unique_future<bool>  future;
      JsonBox::Value              value;
      
      promise_value () : promise(boost::promise<bool>()), future(promise.get_future()), value(JsonBox::Value()) {}
    };
    
    typedef std::map<std::string, promise_value*> node_map_t;
    
    node_map_t*                     _node_map;
    boost::promise<bool>            _node_exists;
      
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
     
     boost::mutex& get_mutex();
     
     const bool get_value(const std::string &url, JsonBox::Value &ret);
     
     const bool get_state(const std::string &url);
     
     bool exists_directory(const std::string &url);
     
     void create_directory(const std::string &url);
     
     void create_parents_directory(const std::string &url);
     
     void open_directory(const std::string &url);
     
     void remove_directory(const std::string &url);
     
     void close_directory(const std::string &url);
     
     void set_attribute(const std::string &url, const std::string &key, const std::string &value);
     
     void set_vector_attribute(const std::string &url, const std::string &key, std::vector<std::string> &value);
     
     void remove_attribute(const std::string &url, const std::string &key);
   };
}

#endif // SQL_ASYNC_SERVER_CONNECTION_HPP
