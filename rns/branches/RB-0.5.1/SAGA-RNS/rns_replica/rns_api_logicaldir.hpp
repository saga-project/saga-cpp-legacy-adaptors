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

#ifndef __RNS_API_LOGICALDIR_HPP__
#define __RNS_API_LOGICALDIR_HPP__

#include "rns_api.hpp"


namespace rns_replica_adaptor { namespace api {

  class rns_logicaldir {

  private:

  public:
    //
	  rns_logicaldir()
    {
		JavaVM* javavm;
		JNIEnv* jnienv;
		helper::init_jvm(jnienv, javavm);

    }

    //
    ~rns_logicaldir()
    {

    	JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		int result = helper::detach_jvm(javavm);

		if(result<0){
		    helper::destroy_jvm(javavm);
		}

    }

    std::vector <std::string> list(const std::string& rns_url, std::string pattern, int flags);

    void make_dir(const std::string& rns_url, int flags);
    void remove(const std::string& rns_url, int flags);

	bool exists(const std::string& check_url);
	bool is_dir(const std::string& check_url);
	bool is_entry(const std::string& check_url);
//	bool is_link(const std::string& check_url);

//	std::size_t get_num_entries(const std::string& rns_url);
//	std::string get_entry(const std::string& rns_url, std::size_t entry);

//	void open(const std::string& rns_url, int flags);
//	void open_dir(const std::string& rns_url, int flags);

    void test(void);

  };
}}

#endif  // __RNS_API_LOGICALDIR_HPP__
