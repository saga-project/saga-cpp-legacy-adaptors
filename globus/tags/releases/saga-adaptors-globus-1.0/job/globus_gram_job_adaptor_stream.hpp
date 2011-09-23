//  Copyright (c) 2007 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOBUS_GRAM_JOB_ADAPTOR_STREAM_HPP
#define GLOBUS_GRAM_JOB_ADAPTOR_STREAM_HPP

#include <iosfwd>

#include <saga/impl/engine/cpi.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace impl
{
    template <typename Base> class globus_gram_job_adaptor_stream : public Base
    {
    private:
        typedef Base base_type;
    
    public:
        globus_gram_job_adaptor_stream(saga::impl::v1_0::job_cpi* cpi, int fd)
        : base_type(fd), cpi_(cpi->shared_from_this()),
        proxy_(cpi->get_proxy()->shared_from_this())
        {
        }
        
        ~globus_gram_job_adaptor_stream()
        {
            //
        }
        

private:
// a saga stream has to keep alive the proxy and the cpi instance
TR1::shared_ptr<saga::impl::v1_0::cpi> cpi_;
TR1::shared_ptr<saga::impl::proxy> proxy_;
};

///////////////////////////////////////////////////////////////////////////////
}   // namespace impl

#endif // GLOBUS_GRAM_JOB_ADAPTOR_STREAM_HPP

