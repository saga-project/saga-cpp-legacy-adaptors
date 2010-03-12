//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_OMII_GRIDSAM_CONTEXT_HPP
#define ADAPTORS_OMII_GRIDSAM_CONTEXT_HPP

#include <saga/saga/util.hpp>
#include <saga/saga/types.hpp>
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/context_cpi_instance_data.hpp>

#include <saga/impl/engine/proxy.hpp>
#include <saga/impl/context_cpi.hpp>

///////////////////////////////////////////////////////////////////////////////
//  This adaptor implements the functionality of the Saga API "context".
//  It defines the functions declared in its base class, context_cpi.
class omii_gridsam_context 
    : public saga::adaptors::v1_0::context_cpi<omii_gridsam_context>
{
private:
    typedef saga::adaptors::v1_0::context_cpi<omii_gridsam_context> 
        base_cpi;
    
public:    
    typedef base_cpi::mutex_type mutex_type;

    // constructor of the context CPI
    omii_gridsam_context (proxy* p, cpi_info const& info, 
        saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
        TR1::shared_ptr<saga::adaptor> adaptor);

    // destructor of the file adaptor 
    ~omii_gridsam_context  (void);

    // context functions
    void sync_set_defaults(saga::impl::void_t&);
    
private:
    saga::ini::ini ini_;
};  

#endif


