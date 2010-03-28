//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_CURL_FILE_ADAPTOR_HPP
#define ADAPTORS_CURL_FILE_ADAPTOR_HPP

#include <map>
//#include <vector>

#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include <cURLpp.hpp>
#include <Easy.hpp>

#include "curl_file_adaptor_connection.hpp"

///////////////////////////////////////////////////////////////////////////////

namespace curl_file_adaptor 
{

struct file_adaptor : public saga::adaptor
{
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;    

    /* map to store file ULRs with their cURL handle */
    typedef std::map<std::string, cURLconnection*>
        handle_pool_type_;
    handle_pool_type_ * HandlePool_;

    /* returns an existing or new handle for the given URL */
    cURLconnection* getConnectionHandle(const saga::url url);
    
    /* closes and removes the handle from the pool for the given url */
    void removeConnectionHandle(const saga::url url);
    
    file_adaptor() : HandlePool_(NULL)
    {
        /* initialize the cURL environment */
        cURLpp::initialize(CURL_GLOBAL_NOTHING);
    }
    
    ~file_adaptor()
    {
        /* release the cURL environment */
        // TODO: does terminate close all handles?
        cURLpp::terminate(); 
        delete HandlePool_;
    }    
    
    std::string get_name (void) const
    {
        return BOOST_PP_STRINGIZE(SAGA_ADAPTOR_NAME);
    }
    
    saga::impl::adaptor_selector::adaptor_info_list_type 
            adaptor_register (saga::impl::session *s);
};

}
#endif // ADAPTORS_CURL_FILE_ADAPTOR_HPP

