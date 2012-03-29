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

#include "rns_api_logicalfile.hpp"

namespace rns_replica_adaptor { namespace api
{
	bool rns_logicalfile::attribute_exists(const std::string& rns_url, const std::string& key)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::attribute_exists");

		SAGA_LOG_CRITICAL("-------------- attribute_exists -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
//		std::string class_name = "RNS_setxml";
		std::string class_name = "RNS_lskv";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

//		std::string method_name = "RNS_attr_exists";
//		std::string method_type = "([Ljava/lang/String;)Z";
		std::string method_name = "RNS_lskv_exists";
		std::string method_type = "([Ljava/lang/String;)Z";
		jmethodID mid = jnienv->GetStaticMethodID(cls, method_name.c_str(), method_type.c_str());

		std::string str_buf;
		if(mid == NULL){
			str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
		jobjectArray j_args = jnienv->NewObjectArray(2,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(j_args,0,jnienv->NewStringUTF(rns_url.c_str()));
		jnienv->SetObjectArrayElement(j_args,1,jnienv->NewStringUTF(key.c_str()));

		bool result_exists = (bool) jnienv->CallStaticObjectMethod(cls, mid, j_args);

		return result_exists;

	}

	std::string rns_logicalfile::get_attribute(const std::string& rns_url, const std::string& key)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::get_attribute");

		SAGA_LOG_CRITICAL("-------------- get_attribute -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_getkv";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_getval";
		std::string method_type = "([Ljava/lang/String;)Ljava/lang/String;";
		jmethodID mid = jnienv->GetStaticMethodID(cls,
				method_name.c_str(), method_type.c_str());

		std::string str_buf;
		if(mid == NULL){
			str_buf = "failed to find " + method_name;
			SAGA_LOG_CRITICAL(str_buf.c_str());
			exit(-1);
		}

		SAGA_LOG_CRITICAL("calling method.\n");
		jobjectArray j_args = jnienv->NewObjectArray(2,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(j_args,0,jnienv->NewStringUTF(rns_url.c_str()));
		jnienv->SetObjectArrayElement(j_args,1,jnienv->NewStringUTF(key.c_str()));

		jstring jstr = (jstring)jnienv->CallStaticObjectMethod(cls, mid, j_args);
		std::string result_str = jnienv->GetStringUTFChars(jstr,0);

		return result_str;

	}

	void rns_logicalfile::set_attribute(const std::string& rns_url, const std::string& key, const std::string& val)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::set_attribute");

		SAGA_LOG_CRITICAL("-------------- set_attribute -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
//		std::string class_name = "RNS_setxml";
		std::string class_name = "RNS_setkv";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

//		std::string method_name = "RNS_set_attr";
		std::string method_name = "RNS_setval";
//		std::string method_type = "([Ljava/lang/String;)V";
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
		jobjectArray j_args = jnienv->NewObjectArray(3,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(j_args,0,jnienv->NewStringUTF(rns_url.c_str()));
		jnienv->SetObjectArrayElement(j_args,1,jnienv->NewStringUTF(key.c_str()));
		jnienv->SetObjectArrayElement(j_args,2,jnienv->NewStringUTF(val.c_str()));

		jnienv->CallStaticObjectMethod(cls, mid, j_args);

	}


	void rns_logicalfile::add_location(const std::string& rns_url, const std::string& loc_url)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::add_location");

		printf("-------------- add_location -----------------\n");

		bool result_add_epr = this->add_epr(rns_url, loc_url);

		if (!result_add_epr){
			bool result_add_url = this->add_url(rns_url, loc_url);
			std::string str_buf = "result add_url : "  + result_add_url?"true":"false";;
			SAGA_LOG_CRITICAL(str_buf.c_str());
		}
	}

	bool rns_logicalfile::add_epr(const std::string& rns_url, const std::string& loc_url)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::add_epr");

		SAGA_LOG_CRITICAL("-------------- add_epr -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_add";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_ent_add_epr";
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
		jobjectArray j_args = jnienv->NewObjectArray(2,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(j_args,0,jnienv->NewStringUTF(rns_url.c_str()));
		jnienv->SetObjectArrayElement(j_args,1,jnienv->NewStringUTF(loc_url.c_str()));

		bool result_add_epr = (bool) jnienv->CallStaticObjectMethod(cls, mid, j_args);

		return result_add_epr;

	}

	bool rns_logicalfile::add_url(const std::string& rns_url, const std::string& loc_url)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::add_url");

		SAGA_LOG_CRITICAL("-------------- add_url -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_add";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_ent_add_url";
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
		jobjectArray j_args = jnienv->NewObjectArray(2,
					jnienv->FindClass("java/lang/String"),
					jnienv->NewStringUTF(""));
		jnienv->SetObjectArrayElement(j_args,0,jnienv->NewStringUTF(rns_url.c_str()));
		jnienv->SetObjectArrayElement(j_args,1,jnienv->NewStringUTF(loc_url.c_str()));

		bool result_add_epr = (bool) jnienv->CallStaticObjectMethod(cls, mid, j_args);

		return result_add_epr;

	}

	void rns_logicalfile::remove(const std::string& rns_url, int flags)
	{
		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::remove");

		SAGA_LOG_CRITICAL("-------------- remove logical file -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_rm";
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


	std::vector<std::string> rns_logicalfile::list_attributes(const std::string& rns_url){

		SAGA_LOG_DEBUG("rns_logicalfile::api::rns_logicalfile::list_attribute");

		SAGA_LOG_CRITICAL("-------------- logical file list_attr -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_lskv";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_lskv_list";
		std::string method_type = "([Ljava/lang/String;)[Ljava/lang/String;";
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
		jnienv->SetObjectArrayElement(j_args,0,jnienv->NewStringUTF(rns_url.c_str()));

		std::vector<std::string> keys;
		jobjectArray jkeys = (jobjectArray)jnienv->CallStaticObjectMethod(cls, mid, j_args);
		jint jkeys_size = jnienv->GetArrayLength(jkeys);

		for (int i=0; i<jkeys_size;i++){
			jstring jstr = (jstring)jnienv->GetObjectArrayElement(jkeys, i);
			keys.push_back(jnienv->GetStringUTFChars(jstr,0));
		}

		return keys;
	}

	std::vector<std::string> rns_logicalfile::list_locations(const std::string& rns_url){

		SAGA_LOG_DEBUG("rns_logicalfile::api::rns_logicalfile::list_locations");

		SAGA_LOG_CRITICAL("-------------- logical file list_locations -----------------\n");

		JavaVM *javavm;
		JNIEnv *jnienv;
		helper::get_jvmenv(jnienv, javavm);

		std::string class_path = "";
		std::string class_name = "RNS_getepr";
		jclass cls = helper::search_class(jnienv, class_path, class_name);

		std::string method_name = "RNS_locations";
		std::string method_type = "([Ljava/lang/String;)[Ljava/lang/String;";
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

//	saga::off_t rns_logicalfile::get_size(const std::string& check_url)
//	{
//		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::get_size");
//
//		printf("-------------- get_size -----------------\n");
//
//		saga::off_t data_size = 0;
////
////		boost::filesystem::path check_url_org(check_url);
////		std::string check_name = check_url_org.leaf();
////		std::string check_url_bpath = check_url_org.branch_path().string();
////
////		std::string sql_str_data = "\"SELECT DATA_NAME, DATA_SIZE WHERE COLL_NAME = ";
////		sql_str_data += "'" + check_url_bpath + "'";
////		sql_str_data += " AND DATA_NAME = ";
////		sql_str_data += "'" + check_name + "'";
////		sql_str_data += "\"";
////
////		char* c_sql_data = const_cast<char*>(sql_str_data.c_str());
////		std::cout << "c_sql_data:" << c_sql_data << std::endl;
////
////		int argc = 2;
////		char *argv[2] = {"iquest", c_sql_data};
////		std::vector <irdEnt_t> irds_data;
////		irds_data = icmd.iquest(argc, argv);
////
////		if (irds_data.size() == 1){
////			data_size = irds_data[0].dataSize;
////		}
////		else {
////			data_size = 0;
////		}
//
//		return data_size;
//
//	}

//	void rns_logicalfile::test(void)
//	{
//		SAGA_LOG_DEBUG("rns_replica_adaptor::api::rns_logicalfile::test");
//
//		SAGA_LOG_CRITICAL("-------------- test -----------------\n");
//
//		JNIEnv *jnienv;
//		JavaVM *javavm;
//		JavaVMInitArgs vm_args;
//
//		//set $CLASSPATH
//		std::string cp = "-Djava.class.path=";
//	    cp += getenv("CLASSPATH");
//		cp += ":/RNS/rns/build/classes";
//
//		JavaVMOption options[1];
//		options[0].optionString = const_cast<char *>(cp.c_str());
//		vm_args.version = JNI_VERSION_1_6;
//		vm_args.options = options;
//		vm_args.nOptions = 1;
//		vm_args.ignoreUnrecognized = true;
//
//		SAGA_LOG_CRITICAL("Java VM generate\n");
//
//		int result = JNI_CreateJavaVM(&javavm, (void **)&jnienv, &vm_args);
//		SAGA_LOG_CRITICAL("create jvm");
//
//		if(result != 0){
//			printf("jvm failed to create : %d\n", result);
////			return 1;
//			exit(-1);
//		}
//
//		SAGA_LOG_CRITICAL("searching class");
//		jclass cls = jnienv->FindClass("org/naregi/rns/RNSVersion");
//		if(cls == 0){
//			printf("failed to find class");
////			return 1;
//			exit(-1);
//		}
//
//		SAGA_LOG_CRITICAL("searching method: getVersion");
//		jmethodID mid = jnienv->GetStaticMethodID(cls, "getVersion", "()Ljava/lang/String;");
//		if(mid == NULL){
//			SAGA_LOG_CRITICAL("failed to find getVersion");
////			return 1;
//			exit(-1);
//		}
//
//		printf("calling method.\n");
//	    jobject objResult = jnienv->CallStaticObjectMethod(cls, mid);
//
//	    jthrowable throwResult = jnienv->ExceptionOccurred();
//	    if (throwResult != NULL) {
//	    	printf("Exception occurred.\n");
//	        jnienv->ExceptionDescribe();
//	        jnienv->ExceptionClear();
////	        return 1;
//			exit(-1);
//	    }
//
//	    if (objResult == NULL) {
//	    	printf("null is returned.\n");
//	    } else {
//	        jstring strResult = (jstring) objResult;
//	        printf("[%s]\n", jnienv->GetStringUTFChars(strResult, NULL));
//	    }
//
//		// destroy JVM
//		result = javavm->DestroyJavaVM();
//		if(result){
//			printf("Failed to destroy jvm");
////			return 1;
//			exit(-1);
//		}
//
//		printf("Safely destroyed jvm");
//
//	}

}
}
