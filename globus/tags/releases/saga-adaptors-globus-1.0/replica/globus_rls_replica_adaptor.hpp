//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_GLOBUS_RLS_REPLICA_HPP
#define ADAPTORS_GLOBUS_RLS_REPLICA_HPP

#include <saga/saga/adaptors/adaptor.hpp>
#include "globus_rls_replica_adaptor_connection.hpp"
#include <boost/spirit/core/non_terminal/impl/static.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace globus_rls_replica_adaptor
{
    //////////////////////////////////////////////////////////////////////////
    struct adaptor : public saga::adaptor
    {
        typedef saga::impl::v1_0::op_info         op_info;  
        typedef saga::impl::v1_0::cpi_info        cpi_info;
        typedef saga::impl::v1_0::preference_type preference_type;
        
		//// Connection Pool handling
		//
		typedef std::map<std::string, RLSConnection*>
			RLSConnectionPool_type_;
		RLSConnectionPool_type_ * RLSConnectionPool_;

		RLSConnection* getConnectionHandle (saga::url url);
		void removeConnectionHandle (const saga::url url);
		//
		////
        
        saga::impl::adaptor_selector::adaptor_info_list_type 
                adaptor_register (saga::impl::session *s);

        std::string get_name (void) const
        { 
            return BOOST_PP_STRINGIZE(SAGA_ADAPTOR_NAME);
        }
		
		adaptor();
    
		~adaptor();
    };

	//////////////////////////////////////////////////////////////////////////
} 
///////////////////////////////////////////////////////////////////////////////

#endif

