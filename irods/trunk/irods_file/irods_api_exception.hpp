/*
 * Copyright (C) 2008-2011 High Energy Accelerator Research Organization (KEK)
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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
#ifndef __iRODS_API_EXCEPTION_HPP__
#define __iRODS_API_EXCEPTION_HPP__
#include <iostream>
#include <sstream>
#include <vector>

#include "irods_api.hpp"

namespace irods_file_adaptor { namespace api
{
	class exception {
	protected:
		std::string _msg;

	public:
		exception(std::string msg) : _msg(msg) {}
		virtual ~exception() {}
		virtual std::string what() const;

		std::ostream& put(std::ostream& s) const
		{
			s << what();
			return s;
		}
	};

//	class func_exception : public exception {
//		irods_error_t e;
//
//	public:
//		func_exception(irods_error_t e, std::string func): exception(func), e(e){}
//
//		std::string what() const;
//		irods_error_t get_error() { return e; }
//	};

	std::ostream& operator<<(std::ostream& s, api::exception& e);
}}

#endif  // __iRODS_API_EXCEPTION_HPP__
