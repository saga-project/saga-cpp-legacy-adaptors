//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/util.hpp>
#include <saga/exception.hpp>
#include <saga/task.hpp>
#include <saga/adaptors/attribute.hpp>

#include "curl_file_adaptor_context.hpp"

///////////////////////////////////////////////////////////////////////////////
namespace curl_file_adaptor
{
    
    ///////////////////////////////////////////////////////////////////////////////
    //  constructor
    context_cpi_impl::context_cpi_impl (proxy* p, cpi_info const& info,
                                        saga::ini::ini const& glob_ini, 
                                        saga::ini::ini const& adap_ini,
                                        TR1::shared_ptr<saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
    {
        saga::adaptors::attribute attr(this);
        if ("cURL ?!?" != attr.get_attribute(saga::attributes::context_type))
        {
            SAGA_ADAPTOR_THROW("Can't handle context types others than 'cURL ?!?'", 
                               saga::BadParameter);
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    //  destructor
    context_cpi_impl::~context_cpi_impl (void)
    {
    }
    
    ///////////////////////////////////////////////////////////////////////////////
    void context_cpi_impl::sync_set_defaults(saga::impl::void_t&)
    {
        saga::adaptors::attribute attr(this);
        if ("cURL ?!?" != attr.get_attribute(saga::attributes::context_type))
        {
            SAGA_ADAPTOR_THROW(
                               "Can't handle context types others than 'cURL ?!?'", 
                               saga::BadParameter);
        }
        
        // NOTE: Iterate over context attributes and add missing values...
        
        //attr.set_attribute(saga::attributes::context_userid, "SAGA");
        //attr.set_attribute(saga::attributes::context_userpass, "SAGA_client");
    }
    
    ///////////////////////////////////////////////////////////////////////////////
}
