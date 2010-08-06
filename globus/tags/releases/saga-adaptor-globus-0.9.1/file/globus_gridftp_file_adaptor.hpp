//  Copyright (c) 2006-2007 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_HPP
#define ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_HPP

#include <map>
#include <vector>

#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include "globus_gridftp_file_adaptor_connection.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace globus_gridftp_file_adaptor 
{

struct file_adaptor : public saga::adaptor
{
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    typedef std::map<std::string, GridFTPConnection*>
      connection_pool_type_;
    connection_pool_type_ * ConnectionPool_;
    
    GridFTPConnection* getConnectionHandleForURL (const saga::url url);
    void removeConnectionHandle (const saga::url url);
    
    file_adaptor(); 
    
    ~file_adaptor(); 

    std::string get_name (void) const
    {
        return BOOST_PP_STRINGIZE(SAGA_ADAPTOR_NAME);
    }
    
  /**
    * This functions registers the adaptor with the factory
    *
    * @param factory the factory where the adaptor registers
    *        its maker function and description table
    */
    saga::impl::adaptor_selector::adaptor_info_list_type 
            adaptor_register (saga::impl::session *s);
};

}
#endif // ADAPTORS_GLOBUS_GRIDFTP_FILE_ADAPTOR_HPP

