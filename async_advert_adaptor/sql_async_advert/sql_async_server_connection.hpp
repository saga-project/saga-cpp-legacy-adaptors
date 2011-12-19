#ifndef SQL_ASYNC_SERVER_CONNECTION_HPP
#define SQL_ASYNC_SERVER_CONNECTION_HPP

// Boost Includes
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

// JSON Includes
#include <json/json.h>

// ZMQ
#include <zmq.hpp>

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
    
    server_connection (saga::url const &url);
    
    // ==============
    // = Destructor =
    // ==============
    
    ~server_connection (void);
    
  private:
    
    // ===================
    // = Private members =
    // ===================
    
    saga::url         _url;
    std::string       _connect_string;
    
    zmq::context_t    _context;
    zmq::socket_t     _socket;
    zmq::socket_t     _socket_rh;
    
    boost::mutex      _mutex;
    boost::thread     _thread;
    
    typedef std::map<std::string, bool> update_map_t;
    update_map_t*     _update_map;
    
    typedef std::map<std::string, std::string> id_map_t;
    id_map_t*         _id_map;
    
    typedef std::map<std::string, Json::Value> node_map_t;  
    node_map_t*       _node_map;
      
    // ===================
    // = Private methods =
    // ===================
    
    void read_handler(zmq::context_t *context);
    
    void erase_node(const std::string &url);
    
    void send_message(const std::string &command, const Json::Value &value);
    
   public:
     
     // ==================
     // = Public methods =
     // ==================
     
     const bool get_value(const std::string &url, Json::Value &ret);
     
     const bool get_state(const std::string &url);
     
     bool exists_directory(const std::string &url);
     
     void create_directory(const std::string &url, const bool dir);
     
     void create_parents_directory(const std::string &url, const bool dir);
     
     void open_directory(const std::string &url);
     
     void remove_directory(const std::string &url);
     
     void close_directory(const std::string &url);
     
     void set_attribute(const std::string &url, const std::string &key, const std::string &value);
     
     void set_vector_attribute(const std::string &url, const std::string &key, std::vector<std::string> &value);
     
     void remove_attribute(const std::string &url, const std::string &key);
     
     void set_string(const std::string &url, const std::string &data);
     
     void remove_string(const std::string &url);
   };
   
}

#endif // SQL_ASYNC_SERVER_CONNECTION_HPP
