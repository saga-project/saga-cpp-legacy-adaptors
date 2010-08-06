//  Copyright (c) 2007 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_OSTREAM_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_OSTREAM_HPP

#include <saga/impl/job.hpp>

#include <boost/iostreams/device/null.hpp>
#include <boost/iostreams/stream.hpp>

#include "globus_gram_job_adaptor_stream.hpp"

namespace io = boost::iostreams;

///////////////////////////////////////////////////////////////////////////

class gram_ostream
:   public saga::impl::ostream_interface
{
public:
    gram_ostream(int fd) : buffer_() {}
    ~gram_ostream() {}
    std::streambuf *get_streambuf() 
{
        return &buffer_;
}

private:
    io::stream_buffer<io::null_sink> buffer_;
};

class globus_gram_job_adaptor_ostream : public saga::job::ostream
{
    
    private:
   
    typedef impl::globus_gram_job_adaptor_stream<gram_ostream> impl_type;
    
    public:
    
    globus_gram_job_adaptor_ostream(saga::impl::v1_0::job_cpi* cpi, int fd)
    : saga::job::ostream(new impl_type(cpi, 0))
    {
        //
    }

    ~globus_gram_job_adaptor_ostream()
    {
        //
    }
    
};

#endif // ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_OSTREAM_HPP

