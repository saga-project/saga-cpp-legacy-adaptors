/*
 * Copyright (C) 2008-2012 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef __ADAPTORS_RNS_REPLICA_HPP
#define __ADAPTORS_RNS_REPLICA_HPP

#include <saga/saga/adaptors/adaptor.hpp>
#include <boost/spirit/core/non_terminal/impl/static.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace rns_replica_adaptor
{
    //////////////////////////////////////////////////////////////////////////
    struct adaptor : public saga::adaptor
    {
        typedef saga::impl::v1_0::op_info         op_info;
        typedef saga::impl::v1_0::cpi_info        cpi_info;
        typedef saga::impl::v1_0::preference_type preference_type;

//		//// Connection Pool handling
//		//
//		typedef std::map<std::string, RLSConnection*>
//			RLSConnectionPool_type_;
//		RLSConnectionPool_type_ * RLSConnectionPool_;
//
//		RLSConnection* getConnectionHandle (saga::url url);
//		void removeConnectionHandle (const saga::url url);
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

#endif	//__ADAPTORS_RNS_REPLICA_HPP

