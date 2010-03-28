//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <curlpp/cURLpp.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Infos.hpp>

#include "curl_file_adaptor_connection.hpp"

using namespace curl_file_adaptor; 

///////////////////////////////////////////////////////////////////////////
//
cURLconnection::cURLconnection(const std::string url)
{
    try {
        this->url = cURLpp::Options::Url(url);
        handle.setOpt(this->url);

        //handle.perform();
        
        std::string effURL;
        cURLpp::Infos::EffectiveUrl::get(handle, effURL);
        std::cout << "Effective URL: " << effURL << std::endl;
    } 
    catch( cURLpp::RuntimeError const & re) {
        std::cout << "RuntimeError: " << re.what() << std::endl;
    }
    catch( cURLpp::LogicError const & le) {
        std::cout << "LogicError: " << le.what() << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////
//
cURLconnection::~cURLconnection()
{
}