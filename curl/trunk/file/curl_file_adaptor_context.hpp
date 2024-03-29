//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_CURL_FILE_ADAPTOR_CONTEXT_HPP
#define ADAPTORS_CURL_FILE_ADAPTOR_CONTEXT_HPP

#include <saga/util.hpp>
#include <saga/types.hpp>
#include <saga/adaptors/adaptor.hpp>
#include <saga/adaptors/context_cpi_instance_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/context_cpi.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace curl_file_adaptor
{
    ///////////////////////////////////////////////////////////////////////////
    //  This adaptor implements the functionality of the Saga API "context".
    //  It defines the functions declared in its base class, context_cpi.
    class context_cpi_impl 
    : public saga::adaptors::v1_0::context_cpi<context_cpi_impl>
    {
    private:
        typedef saga::adaptors::v1_0::context_cpi<context_cpi_impl> 
            base_cpi;
        
    public:    
        typedef base_cpi::mutex_type mutex_type;
        
        // constructor of the context CPI
        context_cpi_impl (proxy* p, cpi_info const& info, 
                          saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
                          TR1::shared_ptr<saga::adaptor> adaptor);
        
        // destructor of the file adaptor 
        ~context_cpi_impl  (void);
        
        // context functions
        void sync_set_defaults(saga::impl::void_t&);
    };  
    
    ///////////////////////////////////////////////////////////////////////////
}

#endif


