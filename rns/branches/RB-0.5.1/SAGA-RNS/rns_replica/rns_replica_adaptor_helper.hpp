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

#ifndef   __RNS_FILE_ADAPTOR_HELPER_HPP__
#define   __RNS_FILE_ADAPTOR_HELPER_HPP__

#include <string>

#include <saga/saga/url.hpp>
#include <saga/saga/filesystem.hpp>

#include "rns_api.hpp"

namespace rns_replica_adaptor
{
  namespace helper {

	void init_jvm(JNIEnv* &jnienv, JavaVM* &javavm);
    int get_jvmenv(JNIEnv* &javavm, JavaVM* &javavm);
	jclass search_class(JNIEnv* &jnienv, std::string class_path, std::string class_name);
	void check_java_exception(JNIEnv* &jnienv);
	int detach_jvm(JavaVM* &javavm);
	void destroy_jvm(JavaVM* &javavm);
	void print_java_results(JNIEnv* &jnienv, jobject objResult);

//    std::string create_error_message(const std::string& msg,
//                                     const std::string& func,
//                                     rns_error_t e);
//
//    std::string create_error_message(const std::string& func,
//                                     rns_error_t e);

    void check_scheme(saga::url& rm, bool local_ok=true);

  }
}

#endif // __RNS_FILE_ADAPTOR_HELPER_HPP__
