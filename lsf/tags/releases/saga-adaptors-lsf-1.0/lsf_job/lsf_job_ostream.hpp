//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifdef  ADAPTORS_LSF_JOB_OSTREAM_HPP
#define ADAPTORS_LSF_JOB_OSTREAM_HPP


// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/job.hpp>

// adaptor includes
#include "lsf_job_stream.hpp"


////////////////////////////////////////////////////////////////////////
namespace saga { namespace adaptors { namespace lsf
{
  class ostream : public saga::job::ostream
  {
    private:
      typedef impl::lsf_job_stream <saga::adaptors::ostream_ptr> 
              impl_type;

    public:
      template <typename Stream>
      ostream (saga::impl::v1_0::job_cpi * cpi, 
               Stream                    & child_ostream)
        : saga::job::ostream (new impl_type (cpi, child_ostream.rdbuf ()))
      {
      }
  };

}}} // namespace lsf_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_LSF_JOB_OSTREAM_HPP

