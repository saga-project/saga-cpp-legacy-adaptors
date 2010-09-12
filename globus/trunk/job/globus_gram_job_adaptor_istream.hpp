//  Copyright (c) 2007 Ole Christian Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_ISTREAM_HPP
#define ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_ISTREAM_HPP

#include <saga/impl/job.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include "globus_gram_job_adaptor_stream.hpp"

namespace io = boost::iostreams;

///////////////////////////////////////////////////////////////////////////

class gram_istream
:   public saga::impl::istream_interface
{
public:
	#if BOOST_VERSION < 104400
    	gram_istream(int fd) : pipe_handle(fd), buffer_(fd) {}
    #else
        gram_istream(int fd) : pipe_handle(fd), buffer_(fd, io::never_close_handle) {}
    #endif
    
    ~gram_istream() 
    {
        if(-1 == close(pipe_handle))
            printf("close error(fd: %d): %s\n", pipe_handle, strerror( errno ));
    }
    std::streambuf *get_streambuf() 
    {
        return &buffer_;
    }

private:

    int pipe_handle;

    io::stream_buffer<io::file_descriptor_source> buffer_;
};

class globus_gram_job_adaptor_istream : public saga::job::istream
{

    private:
    
        int pipe_handle;
    
    typedef impl::globus_gram_job_adaptor_stream<gram_istream> impl_type;
    
    public:
    
    globus_gram_job_adaptor_istream(saga::impl::v1_0::job_cpi* cpi, int fd)
    : saga::job::istream(new impl_type(cpi, fd))
    {
        //
    }

    ~globus_gram_job_adaptor_istream()
    {
        //
    }
};

#endif // ADAPTORS_GLOBUS_GRAM_JOB_ADAPTOR_ISTREAM_HPP

