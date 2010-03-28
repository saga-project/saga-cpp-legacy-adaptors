//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga-defs.hpp>

#if !defined(ADAPTORS_OMII_JOB_ISTREAM_HPP)
#define ADAPTORS_OMII_JOB_ISTREAM_HPP

#include <saga/saga/util.hpp>
#include <saga/saga/base.hpp>
#include <saga/saga/packages/job/istream.hpp>

///////////////////////////////////////////////////////////////////////////////
struct omii_job_istream
  : public saga::job::istream
{
    // initialized istream with eofbit set
    omii_job_istream() : saga::job::istream() {}
};

#endif 

