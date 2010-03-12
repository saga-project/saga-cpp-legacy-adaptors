//  Copyright (c) 2005-2009 Hartmut Kaiser
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 
#include <saga/saga/util.hpp>
#include <saga/saga/exception.hpp>
#include <saga/saga/task.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

#include "omii_gridsam_context.hpp"
#include "common_helpers.hpp"

///////////////////////////////////////////////////////////////////////////////
//  constructor
omii_gridsam_context::omii_gridsam_context (proxy* p, cpi_info const& info,
        saga::ini::ini const& glob_ini, saga::ini::ini const& adap_ini,
        TR1::shared_ptr<saga::adaptor> adaptor)
  : base_cpi (p, info, adaptor, cpi::Noflags), ini_(adap_ini)
{
    saga::adaptors::attribute attr(this);
    if (attr.attribute_exists(saga::attributes::context_type) &&
        "omii_gridsam" != attr.get_attribute(saga::attributes::context_type))
    {
        SAGA_OSSTREAM strm;
        strm << "Can't handle context types others than 'omii_gridsam' "
             << "(got: " << attr.get_attribute(saga::attributes::context_type) 
             << ")";
        SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined);
    }
}

///////////////////////////////////////////////////////////////////////////////
//  destructor
omii_gridsam_context::~omii_gridsam_context (void)
{
}

///////////////////////////////////////////////////////////////////////////////
void omii_gridsam_context::sync_set_defaults(saga::impl::void_t&)
{
    saga::adaptors::attribute attr(this);
    if (attr.attribute_exists(saga::attributes::context_type))
    {
        if ("omii_gridsam" != attr.get_attribute(saga::attributes::context_type))
        {
            SAGA_OSSTREAM strm;
            strm << "Can't handle context types others than 'omii_gridsam' "
                 << "(got: " << attr.get_attribute(saga::attributes::context_type) 
                 << ")";
            SAGA_ADAPTOR_THROW(SAGA_OSSTREAM_GETSTRING(strm), saga::adaptors::AdaptorDeclined);
        }

        // use the OMII installation directory as the default cert store
        std::string omii_root(get_omii_path(ini_));
        std::string omii_certs(omii_root + "/pem_format_certs/ca_certs");
        std::string omii_usercert(omii_root + "/pem_format_certs/usercert.pem");
        std::string omii_userkey(omii_root + "/pem_format_certs/userunencryptedkey.pem");

        if (!attr.attribute_exists(saga::attributes::context_usercert))
            attr.set_attribute(saga::attributes::context_usercert, omii_usercert);
        if (!attr.attribute_exists(saga::attributes::context_userkey))
            attr.set_attribute(saga::attributes::context_userkey, omii_userkey);
        if (!attr.attribute_exists(saga::attributes::context_certrepository))
            attr.set_attribute(saga::attributes::context_certrepository, omii_certs);
    }
}

