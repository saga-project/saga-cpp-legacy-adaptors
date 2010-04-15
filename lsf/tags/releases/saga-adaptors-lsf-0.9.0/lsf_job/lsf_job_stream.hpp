//  Copyright (c) 2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(ADAPTORS_LSF_JOB_STREAM_HPP)
#define ADAPTORS_LSF_JOB_STREAM_HPP

// stl includes
#include <iosfwd>

// saga engine includes
#include <saga/impl/engine/cpi.hpp>


///////////////////////////////////////////////////////////////////////////////
namespace saga { namespace adaptors { namespace lsf
{
    template <typename Base>
    class stream
      :   public Base
    {
      private:
        typedef Base base_type;

        // a saga stream has to keep alive the proxy and the cpi instance
        TR1::shared_ptr <saga::impl::v1_0::cpi> cpi_;
        TR1::shared_ptr <saga::impl::proxy>     proxy_;


      public:
        stream (saga::impl::v1_0::job_cpi * cpi, 
                std::streambuf            * buf)
          : base_type (buf),
            cpi_      (cpi->shared_from_this ()),
            proxy_    (cpi->get_proxy ()->shared_from_this ())
          {
          }
    };

}}} // namespace impl
////////////////////////////////////////////////////////////////////////

#endif // !defined(ADAPTORS_LSF_JOB_STREAM_HPP)

