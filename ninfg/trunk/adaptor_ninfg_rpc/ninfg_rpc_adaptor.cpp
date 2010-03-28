
// INSERT YOUR LICENSE BOILER PLATE HERE

#include <saga/saga/adaptors/adaptor.hpp>

#include "ninfg_rpc_adaptor.hpp"
#include "ninfg_rpc.hpp"

SAGA_ADAPTOR_REGISTER (ninfg_rpc::rpc_adaptor);

///////////////////////////////////////////////////////////////////////////////
namespace ninfg_rpc
{
  // register function for the SAGA engine
  saga::impl::adaptor_selector::adaptor_info_list_type 
      rpc_adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type list;

    // create preferences
    preference_type prefs; 

    // create rpc cpi infos (each adaptor instance gets its own uuid) 
    // and add cpi_infos to list
    rpc_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);
    //parameter_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);

    // and return list
    return list;
  }

} // namespace ninfg_rpc
///////////////////////////////////////////////////////////////////////////////

