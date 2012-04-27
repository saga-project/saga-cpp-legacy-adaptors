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

#include <sstream>

#include <saga/saga/exception.hpp>

#include "rns_api_logicaldir.hpp"

namespace rns_replica_adaptor { namespace api
{

//	void rns_logicaldir::open(const std::string& rns_url, int flags)
//	{
//		SAGA_LOG_DEBUG("irods_file_adaptor::api::irods_dir::open");
//
//		printf("-------------- create entry (open) -----------------\n");
//
//		JavaVM *javavm;
//		JNIEnv *jnienv;
//		helper::get_jvmenv(jnienv, javavm);
//
//		std::string class_path = "";
//		std::string class_name = "RNS_add";
//		jclass cls = helper::search_class(jnienv, class_path, class_name);
//
//		std::string method_name = "RNS_ent_open";
//		std::string method_type = "([Ljava/lang/String;)V";
//		jmethodID mid = jnienv->GetStaticMethodID(cls,
//				method_name.c_str(), method_type.c_str());
//		if(mid == NULL){
//			std::cout << "failed to find " << method_name << std::endl;
//			exit(-1);
//		}
//
//		printf("calling method.\n");
//		jobjectArray j_args = jnienv->NewObjectArray(1,
//					jnienv->FindClass("java/lang/String"),
//					jnienv->NewStringUTF(""));
//		jnienv->SetObjectArrayElement(
//					j_args,0,jnienv->NewStringUTF(rns_url.c_str()));
//
//		jnienv->CallStaticObjectMethod(cls, mid, j_args);
//
//	}


	std::vector <std::string> rns_logicaldir::list(const std::string& rns_url, std::string pattern, int flags)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::list");

		SAGA_LOG_CRITICAL("-------------- list entries (logical dir) -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_list";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_ent_list";
		std::string method_type = "([Ljava/lang/String;)[Ljava/lang/String;";
		jmethodID mid = jnienv->GetStaticMethodID(cls,
				method_name.c_str(), method_type.c_str());
		if(mid == NULL){
			std::string str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
		jobjectArray j_args = jnienv->NewObjectArray(1,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(
					j_args,0,jnienv->NewStringUTF(rns_url.c_str()));

		jobjectArray jarray = (jobjectArray) jnienv->CallStaticObjectMethod(cls, mid, j_args);

		std::vector<std::string> result_str;
		jstring jstr;
		for(int i=0; i< (int)jnienv->GetArrayLength(jarray); i++){
			jstr = (jstring)jnienv->GetObjectArrayElement(jarray, i);
			const char *ca = jnienv->GetStringUTFChars(jstr,0);
			result_str.push_back(ca);
			jnienv->ReleaseStringUTFChars(jstr,0);
		}

		return result_str;

	}

	bool rns_logicaldir::exists(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::exists");

		SAGA_LOG_CRITICAL("-------------- check exists (logical dir) -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_exists";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_ent_exists";
		std::string method_type = "([Ljava/lang/String;)Z";
		jmethodID mid = jnienv->GetStaticMethodID(cls,
				method_name.c_str(), method_type.c_str());

		std::string str_buf;
		if(mid == NULL){
			str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
		jobjectArray j_args = jnienv->NewObjectArray(1,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(
					j_args,0,jnienv->NewStringUTF(check_url.c_str()));

		bool result_exists = (bool) jnienv->CallStaticObjectMethod(cls, mid, j_args);

		str_buf = "reslut_exists=" + result_exists?"true":"false";
		SAGA_LOG_CRITICAL(str_buf.c_str());

		return result_exists;
	}

	bool rns_logicaldir::is_dir(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::exists");

		SAGA_LOG_CRITICAL("-------------- is_dir (logical dir) -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_isdir";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_ent_isdir";
		std::string method_type = "([Ljava/lang/String;)Z";
		jmethodID mid = jnienv->GetStaticMethodID(cls,
				method_name.c_str(), method_type.c_str());

		std::string str_buf;
		if(mid == NULL){
			str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
		jobjectArray j_args = jnienv->NewObjectArray(1,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(
					j_args,0,jnienv->NewStringUTF(check_url.c_str()));

		bool result_isdir = (bool) jnienv->CallStaticObjectMethod(cls, mid, j_args);

		return result_isdir;
	}

	bool rns_logicaldir::is_entry(const std::string& check_url)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::exists");

		SAGA_LOG_CRITICAL("-------------- is_entry (logical dir) -----------------\n");

		if(this->is_dir(check_url)){
			return false;
		}
		else{
			return this->exists(check_url);
		}

	}


	void rns_logicaldir::make_dir(const std::string& rns_url, int flags)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::make_dir");

		SAGA_LOG_CRITICAL("-------------- make logical dir -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_mkdir";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "main";
		std::string method_type = "([Ljava/lang/String;)V";
		jmethodID mid = jnienv->GetStaticMethodID(cls,
				method_name.c_str(), method_type.c_str());

		std::string str_buf;
		if(mid == NULL){
			str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
		jobjectArray j_args = jnienv->NewObjectArray(1,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(
					j_args,0,jnienv->NewStringUTF(rns_url.c_str()));

		jnienv->CallStaticObjectMethod(cls, mid, j_args);

	    helper::check_java_exception(jnienv);
	}



	void rns_logicaldir::remove(const std::string& rns_url, int flags)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::remove");

		SAGA_LOG_CRITICAL("-------------- remove logical dir -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_rmdir";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "main";
		std::string method_type = "([Ljava/lang/String;)V";
		jmethodID mid = jnienv->GetStaticMethodID(cls,
				method_name.c_str(), method_type.c_str());

		std::string str_buf;
		if(mid == NULL){
			str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
		jobjectArray j_args = jnienv->NewObjectArray(1,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(
					j_args,0,jnienv->NewStringUTF(rns_url.c_str()));

		jnienv->CallStaticObjectMethod(cls, mid, j_args);

	    helper::check_java_exception(jnienv);
	}

	void rns_logicaldir::test(void)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::test");

		SAGA_LOG_CRITICAL("-------------- test -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "org/naregi/rns/";
		std::string class_name = "RNSVersion";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "getVersion";
		std::string method_type = "()Ljava/lang/String;";
		jmethodID mid = jnienv->GetStaticMethodID(cls, method_name.c_str(), method_type.c_str());

		std::string str_buf;
		if(mid == NULL){
			str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
//		jstring str = jnienv->NewStringUTF("testXML");
	    jobject objResult = jnienv->CallStaticObjectMethod(cls, mid);

	    helper::check_java_exception(jnienv);
	    helper::print_java_results(jnienv, objResult);

	}


//	std::size_t rns_logicaldir::get_num_entries(const std::string& rns_url)
//	{
//		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicaldir::get_num_entries");
//
//		printf("-------------- get_num_entries (logical dir) -----------------\n");
//
//		JavaVM *javavm;
//		JNIEnv *jnienv;
//		helper::get_jvmenv(jnienv, javavm);
//
//		std::string class_path = "";
//		std::string class_name = "RNS_list";
//		jclass cls = helper::search_class(jnienv, class_path, class_name);
//
//		std::string method_name = "RNS_ent_getnum";
//		std::string method_type = "([Ljava/lang/String;)I";
//		jmethodID mid = jnienv->GetStaticMethodID(cls,
//				method_name.c_str(), method_type.c_str());
//		if(mid == NULL){
//			std::cout << "failed to find " << method_name << std::endl;
//			exit(-1);
//		}
//
//		printf("calling method.\n");
//		jobjectArray j_args = jnienv->NewObjectArray(1,
//					jnienv->FindClass("java/lang/String"),
//					jnienv->NewStringUTF(""));
//		jnienv->SetObjectArrayElement(
//					j_args,0,jnienv->NewStringUTF(rns_url.c_str()));
//
//		std::size_t num = (std::size_t)jnienv->CallStaticObjectMethod(cls, mid, j_args);
//
////		std::cout << "num=" << num << std::endl;
//
//		return num;
//
//	}

}
}
