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

#include <saga/saga/adaptors/adaptor.hpp>

#include "rns_replica_adaptor_helper.hpp"

namespace rns_replica_adaptor
{
	void helper::init_jvm(JNIEnv* &jnienv, JavaVM* &javavm)
	{
		JavaVMInitArgs vm_args;

		//set $CLASSPATH
		std::string op_str = "-Djava.class.path=";
		op_str += getenv("CLASSPATH");

		op_str += ":";
		op_str += getenv("SAGA_LOCATION");
		op_str += "/share/sra/classes";

		JavaVMOption options[1];
		options[0].optionString = const_cast<char *>(op_str.c_str());
		vm_args.version = JNI_VERSION_1_6;
		vm_args.options = options;
		vm_args.nOptions = 1;
		vm_args.ignoreUnrecognized = true;

		SAGA_LOG_CRITICAL("Java VM generate\n");

		int result = JNI_CreateJavaVM(&javavm, (void **)&jnienv, &vm_args);
		SAGA_LOG_CRITICAL("helper:: create jvm \n");

		if(result != 0){
			std::string str_buf = "init_jvm:" + result?"true":"false";
			SAGA_LOG_CRITICAL(str_buf.c_str());
			SAGA_LOG_CRITICAL("   jvm is already created. ");
		}
	}

	int helper::get_jvmenv(JNIEnv* &jnienv, JavaVM* &javavm)
	{
		jint size = 1; //?
		jint vmCount;
		int ret = JNI_GetCreatedJavaVMs(&javavm, size, &vmCount);

		if (ret<0){
			std::cout << "Cannot get a created jvm" << std::endl;
			return ret;
		}

		ret = javavm->AttachCurrentThread((void **)&jnienv, NULL);

		if (ret<0){
			std::cout << "Cannot attach to jvm" << std::endl;
			return ret;
		}

		return ret;
	}


	jclass helper::search_class(JNIEnv* &jnienv, std::string class_path, std::string class_name)
	{
		SAGA_LOG_CRITICAL("helper:: searching class \n");
		class_path += class_name;
		jclass cls = jnienv->FindClass(class_path.c_str());
		if(cls == 0){
			printf("failed to find class \n");
			exit(-1);
		}

		return cls;
	}

	void helper::check_java_exception(JNIEnv* &jnienv)
	{
		// check exception
		jthrowable throwResult = jnienv->ExceptionOccurred();
		if (throwResult != NULL) {
			printf("Exception occurred.\n");
			jnienv->ExceptionDescribe();
			jnienv->ExceptionClear();
			exit(-1);
		}
	}

	int helper::detach_jvm(JavaVM* &javavm)
	{
		// detach JVM
		int result = javavm->DetachCurrentThread();
		if(result){
			printf("Failed to detach the thread from jvm \n");
		}
		else {
			SAGA_LOG_CRITICAL("Safely detached the thread from jvm \n");
		}

		return result;
	}

	void helper::destroy_jvm(JavaVM* &javavm)
	{
		// destroy JVM
		int result = javavm->DestroyJavaVM();
		if(result){
			printf("Failed to destroy jvm \n");
			exit(-1);
		}
		else {
			SAGA_LOG_CRITICAL("Safely destroyed jvm \n");
		}
	}

	void helper::print_java_results(JNIEnv* &jnienv, jobject objResult)
	{
	    // print results
	    if (objResult == NULL) {
	        printf("null is returned.\n");
	    } else {
	        jstring strResult = (jstring) objResult;
	        printf("[%s]\n", jnienv->GetStringUTFChars(strResult, NULL));
	    }
	}

//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  std::string helper::create_error_message(const std::string& msg,
//                                   const std::string& func,
//                                   gfarm_error_t e)
//  {
//    std::ostringstream ost;
//    ost << msg << std::endl;
//    ost << create_error_message(func,e);
//    return ost.str();
//  }
//
//  ///////////////////////////////////////////////////////////////////////////////
//  //
//  std::string helper::create_error_message(const std::string& func,
//                                   irods_error_t e)
//  {
//    std::ostringstream ost;
//    ost << func << " failed error=" << e << ":";
////    ost << gfarm_error_string(e);
//    return ost.str();
//  }

  ///////////////////////////////////////////////////////////////////////////////
  // throws : BadParameter
  void helper::check_scheme(saga::url& rm, bool local_ok)
  {
    std::string scheme(rm.get_scheme());

    if (scheme == "rns") {
    	std::cout << "RNS good" << std::endl;
    	return;
    }

    if (scheme.empty() || scheme == "file") {
      if (local_ok) {
        SAGA_LOG_DEBUG("local file.");
        return;
      } else {
        SAGA_OSSTREAM strm;
        strm << "Could not initialize file for [" << rm << "]. "
             << "Only any:// and rns:// "
             << "schemes are supported.";
        SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
      }
    }

    //SRA : rns
    if (!(scheme == "irods" || scheme == "any")) {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize file for [" << rm << "]. "
           << "Only any:// and rns:// "
           << "schemes are supported.";
      SAGA_ADAPTOR_THROW_NO_CONTEXT(SAGA_OSSTREAM_GETSTRING(strm), saga::BadParameter);
    }
  }

}
