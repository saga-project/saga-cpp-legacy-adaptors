//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_CURL_FILE_ADAPTOR_CONNECTION_HPP
#define ADAPTORS_CURL_FILE_ADAPTOR_CONNECTION_HPP

#include <map>
#include <vector>

#include <saga/saga/url.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

#include <curlpp/cURLpp.hpp>
#include <curlpp/cURLpp.hpp> 
#include <curlpp/Options.hpp> 
#include <curlpp/Easy.hpp>

///////////////////////////////////////////////////////////////////////////////

namespace curl_file_adaptor 
{
    class cURLconnection {
        
    private:
        cURLpp::Easy handle;
        cURLpp::Options::Url url;
        
    public:
        cURLconnection(const std::string url);
        ~cURLconnection();
    };
}

#endif