//  Copyright (c) 2007-2008 Ole Weidner (oweidner@cct.lsu.edu)
// 
//  Distributed under the Boost Software License, Version 1.0. (See accompanying 
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <saga/saga/url.hpp>
#include <boost/lexical_cast.hpp>
#include "globus_rls_replica_adaptor_connection.hpp"

namespace globus_rls_replica_adaptor
{

///////////////////////////////////////////////////////////////////////////////
//
    RLSConnection::RLSConnection(std::string host_url) : lfnListSize(0)
{
	//adaptorConfig = ini;
	// FIXME: retry for non rls or rlsn!!!
	saga::url host_url_s(host_url);
	if(host_url_s.get_scheme() != GLOBUS_RLS_URL_SCHEME &&
	   host_url_s.get_scheme() != GLOBUS_RLS_URL_SCHEME_NOAUTH)
	{
		host_url_s.set_scheme(GLOBUS_RLS_URL_SCHEME);
		host_url = host_url_s.get_url();
	}
	
	globus_result_t result = GLOBUS_FALSE;
	globus_rls_client_set_timeout(getPreferencesTimeout());
	result = globus_rls_client_connect((char*)host_url.c_str(), &RLSHandle);
	if(result != GLOBUS_RLS_SUCCESS)
	{
		throw globus_rls_replica_adaptor::exception(result);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
RLSConnection::~RLSConnection()
{
	globus_rls_client_close(RLSHandle);
}

///////////////////////////////////////////////////////////////////////////////
//
bool RLSConnection::LFNExistsThrow(std::string LFNPath)
{
	globus_result_t result = GLOBUS_FALSE;
	result = globus_rls_client_lrc_exists(RLSHandle, (char*)LFNPath.c_str(), 
										  globus_rls_obj_lrc_lfn);
	if(result != GLOBUS_RLS_SUCCESS) {
		throw globus_rls_replica_adaptor::exception(result);
	}
	else return true;
}

///////////////////////////////////////////////////////////////////////////////
//
bool RLSConnection::LFNExists(std::string LFNPath)
{
	globus_result_t result = GLOBUS_FALSE;
	result = globus_rls_client_lrc_exists(RLSHandle, (char*)LFNPath.c_str(), 
										  globus_rls_obj_lrc_lfn);
	if(result != GLOBUS_RLS_SUCCESS) {
		if(helper::globus_result_t_to_rls_ec(result) == GLOBUS_RLS_LFN_NEXIST)
			return false;
		else
			throw globus_rls_replica_adaptor::exception(result);
	}
	else return true;
}

///////////////////////////////////////////////////////////////////////////////
//
std::vector<saga::url> RLSConnection::LFNGetPFNList(std::string LFNPath)
{
	std::vector<saga::url> PFNList;
	
	globus_result_t result = GLOBUS_FALSE; 
	int offset=0; globus_list_t * result_list;
	
	result = globus_rls_client_lrc_get_pfn(RLSHandle, (char*)LFNPath.c_str(),
										   &offset, 0, &result_list);
	if (result != GLOBUS_RLS_SUCCESS) {
		throw globus_rls_replica_adaptor::exception(result);
	}
	else
	{
		globus_list_t *p;
		globus_rls_string2_t *str2;

		for (p = result_list; p; p = globus_list_rest(p)) {
			str2 = (globus_rls_string2_t *) globus_list_first(p);
			PFNList.push_back(saga::url(str2->s2));
		}
		
		globus_rls_client_free_list(p);
		globus_rls_client_free_list(result_list);
	}
	return PFNList;
}

    
///////////////////////////////////////////////////////////////////////////////
//
	std::vector<saga::url> RLSConnection::LFNList(std::string POSIXPattern)
	{
    std::vector<saga::url> LFNList;
    LFNList.reserve(lfnListSize);
	
	time_t t1; time(&t1);
				
	globus_result_t result = GLOBUS_FALSE; 
	int offset=0; globus_list_t * result_list;
    
    result = globus_rls_client_lrc_get_lfn_wc(RLSHandle, 
                                              (char*)POSIXPattern.c_str(), 
                                              rls_pattern_unix, &offset, 
                                              0, // reslimit 
                                              &result_list);
		if (result != GLOBUS_RLS_SUCCESS) {
			throw globus_rls_replica_adaptor::exception(result);
		}
		else
		{	
			globus_list_t *p;
			globus_rls_string2_t *str2;
			
			std::string previous("");
			
			time_t t1; time(&t1);
						
			for (p = result_list; p; p = globus_list_rest(p)) {
				str2 = (globus_rls_string2_t *) globus_list_first(p);
				
				// globus_rls_client_lrc_get_lfn_wc returns redundant LFN entries
				// each for every PFN. we don't want that in our list:
				std::string current(str2->s1);
				if(current != previous) LFNList.push_back(saga::url(current));
				
				previous = current;
			}
						
			globus_rls_client_free_list(p);
			globus_rls_client_free_list(result_list);
		}
    
		lfnListSize = LFNList.size();
		return LFNList;
	}
	
///////////////////////////////////////////////////////////////////////////////
//
bool RLSConnection::LFNAttributeExists(std::string LFNName, 
									   std::string AttrName)
{
	// to check wether an aatribute exists or not, we have to use 
	// the globus_rls_client_lrc_attr_value_get and check the error code
	globus_result_t result = GLOBUS_FALSE;
	globus_list_t * result_list;
	
	result = globus_rls_client_lrc_attr_value_get(RLSHandle, 
												  (char*)LFNName.c_str(),
												  (char*)AttrName.c_str(),
												  globus_rls_obj_lrc_lfn,
												  &result_list);
	if (result != GLOBUS_RLS_SUCCESS) {
		if(helper::globus_result_t_to_rls_ec(result) == GLOBUS_RLS_ATTR_NEXIST)
			return false;
		else
			throw globus_rls_replica_adaptor::exception(result);
	}
	else {
		globus_rls_client_free_list(result_list);
		return true;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void RLSConnection::LFNAttributeRemove(std::string LFNName, 
									   std::string AttrName)
{	
	globus_result_t result = GLOBUS_FALSE; 
	
	globus_rls_attribute_t attr;
	attr.objtype = globus_rls_obj_lrc_lfn;
	attr.name = (char*)AttrName.c_str();
	
	result = globus_rls_client_lrc_attr_remove(RLSHandle, 
											   (char*)LFNName.c_str(),
											   &attr);

	if (result != GLOBUS_RLS_SUCCESS) {
		throw globus_rls_replica_adaptor::exception(result);
	}
	
	// let's try to get rid of RLI attributes if they're not in use 
	// by any LFN anymore.
	// FIXME: do we need a .ini flag for that? it may mess things up if
	// the RLI server is not used through SAGA exclusively...
	globus_result_t result2 = GLOBUS_FALSE; 
	
	result2 = globus_rls_client_lrc_attr_delete(RLSHandle,
												(char*)AttrName.c_str(),
												globus_rls_obj_lrc_lfn,
												GLOBUS_FALSE); // IMPORTANT!
	if (result != GLOBUS_RLS_SUCCESS) {
		// doesn't work for some reason? we don't care! if any LFN is still
		// using this attribute, a GLOBUS_RLS_ATTR_EXIST arrives here...
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void RLSConnection::LFNAttributeCreate(std::string LFNName, std::string AttrName,
									   std::string AttrValue)
{
	// to create a new attribute we need to define the attribute globally 
	// on the RLI server first:
	globus_result_t result = GLOBUS_FALSE;
	result = globus_rls_client_lrc_attr_create(RLSHandle,
											   (char*)AttrName.c_str(),
											   globus_rls_obj_lrc_lfn,
											   globus_rls_attr_type_str);
	if (result != GLOBUS_RLS_SUCCESS) {
		if(helper::globus_result_t_to_rls_ec(result) == GLOBUS_RLS_ATTR_EXIST)
		{
			// attribute is already defined for the RLI server - this
			// is not an error.
			;
		}
		else {
			throw globus_rls_replica_adaptor::exception(result);
		}
	}
	
	// define and set the new attribute for a specific LFN
	globus_rls_attribute_t attr;
	attr.objtype = globus_rls_obj_lrc_lfn;
	attr.type    = globus_rls_attr_type_str;
	attr.name    = (char*)AttrName.c_str();
	attr.val.s   = (char*)AttrValue.c_str();
	
	globus_result_t result2 = GLOBUS_FALSE;
	result2 = globus_rls_client_lrc_attr_add(RLSHandle, 
											   (char*)LFNName.c_str(),
											   &attr);
	if (result2 != GLOBUS_RLS_SUCCESS) {
		throw globus_rls_replica_adaptor::exception(result2);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void RLSConnection::LFNAttributeModify(std::string LFNName, std::string AttrName,
									   std::string AttrValue)
{				
	// RLS bug: if the attribute is set to a value equal to the current
	// value, the call produces an error: GLOBUS_RLS_ATTR_NEXIST. We don't
	// want that.

	globus_result_t result = GLOBUS_FALSE; 
	
	globus_rls_attribute_t attr;
	attr.objtype = globus_rls_obj_lrc_lfn;
	attr.type    = globus_rls_attr_type_str;
	attr.name    = (char*)AttrName.c_str();
	attr.val.s   = (char*)AttrValue.c_str();
	
	result = globus_rls_client_lrc_attr_modify(RLSHandle, 
											   (char*)LFNName.c_str(),
											   &attr);

	if (result != GLOBUS_RLS_SUCCESS) {
		if(helper::globus_result_t_to_rls_ec(result) == GLOBUS_RLS_ATTR_NEXIST) 
		{
			// RLS bugfix... do nothing if GLOBUS_RLS_ATTR_NEXIST ;-)
			;
		}
		else
		{
			throw globus_rls_replica_adaptor::exception(result);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//
std::string RLSConnection::LFNAttributeGet(std::string LFNName, 
										   std::string AttrName)
{
	// there's a minor bug (?) in the RLS client library: if we query the attr.
	// list for a non-existing LFN, the error code is 23 (ATTR_NEXIST). We
	// have to call LFNExists() first to distinguish errors. 
	bool exists = false;
	exists = LFNExists((char*)LFNName.c_str());
	if(!exists) {
		// FIXME: throw error with correct TEXT!
	}
	
	std::string AttrValue;
	
	globus_result_t result = GLOBUS_FALSE; 
	globus_list_t * result_list;
	
	result = globus_rls_client_lrc_attr_value_get(RLSHandle, 
												  (char*)LFNName.c_str(),
												  (char*)AttrName.c_str(),
												  globus_rls_obj_lrc_lfn,
												  &result_list);
	if (result != GLOBUS_RLS_SUCCESS) {
		throw globus_rls_replica_adaptor::exception(result);
	}
	else
	{
		char buf[MAX_ATTR_LENGTH];
		globus_rls_attribute_t * ap;

		// the list should contain only ONE entry - if not ?!?
		//for (p = result_list; p; p = globus_list_rest(p)) {
		ap = (globus_rls_attribute_t *) globus_list_first(result_list);
		AttrValue = globus_rls_client_attr2s(ap, buf, MAX_ATTR_LENGTH);
		//}
		
		//globus_rls_client_free_list(p);
		globus_rls_client_free_list(result_list);
	}
	return AttrValue;
}							

///////////////////////////////////////////////////////////////////////////////
//
std::vector<std::string> RLSConnection::LFNAttributeList(std::string LFNName)
{
	// there's a minor bug (?) in the RLS client library: if we query the attr.
	// list for a non-existing LFN, the error code is 23 (ATTR_NEXIST). We
	// have to call LFNExists() first to distinguish errors. 
	bool exists = false;
	exists = LFNExists((char*)LFNName.c_str());
	if(!exists) {
		// FIXME: throw error with correct TEXT!
	}
	
	std::vector<std::string> AttrList;
	
	globus_result_t result = GLOBUS_FALSE; 
	globus_list_t * result_list;
	
	result = globus_rls_client_lrc_attr_value_get(RLSHandle, 
												  (char*)LFNName.c_str(),
												  NULL, // get all attributes
												  globus_rls_obj_lrc_lfn,
												  &result_list);
	if (result != GLOBUS_RLS_SUCCESS) {
		if(helper::globus_result_t_to_rls_ec(result) == GLOBUS_RLS_ATTR_NEXIST)
			return AttrList;
		else
			throw globus_rls_replica_adaptor::exception(result);
	}
	else
	{
		globus_list_t *p;
		globus_rls_attribute_t * ap;

		for (p = result_list; p; p = globus_list_rest(p)) {
			ap = (globus_rls_attribute_t *) globus_list_first(p);
			AttrList.push_back(ap->name);
		}
		
		globus_rls_client_free_list(p);
		globus_rls_client_free_list(result_list);
	}
	return AttrList;
}

///////////////////////////////////////////////////////////////////////////////
//
void RLSConnection::LFNAddPFN (std::string LFNName, std::string PFNUrl)
{
	globus_result_t result = GLOBUS_FALSE;
	result = globus_rls_client_lrc_add(RLSHandle, 
									   (char*)LFNName.c_str(), 
									   (char*)PFNUrl.c_str());
	if(result != GLOBUS_RLS_SUCCESS) {
		if(helper::globus_result_t_to_rls_ec(result) == GLOBUS_RLS_MAPPING_EXIST)
		{
			// Spec. p271: if the replica is already in the set, this method 
			// does nothing, and in particular MUST NOT raise an 'AlreadyExists' 
			// exception. 
			;
		}
		else throw globus_rls_replica_adaptor::exception(result);
	}
}

///////////////////////////////////////////////////////////////////////////////
//
void RLSConnection::LFNRemovePFN (std::string LFNName, std::string PFNUrl)
{
	globus_result_t result = GLOBUS_FALSE;
	result = globus_rls_client_lrc_delete(RLSHandle, 
									     (char*)LFNName.c_str(), 
									     (char*)PFNUrl.c_str());
	if(result != GLOBUS_RLS_SUCCESS) {
		throw globus_rls_replica_adaptor::exception(result);
	}
}

bool RLSConnection::LFNtoPFNMappingExists (std::string LFNName, std::string PFNName)
{
	globus_result_t result = GLOBUS_FALSE;
	result = globus_rls_client_lrc_mapping_exists(RLSHandle, 
												 (char*)LFNName.c_str(), 
												 (char*)PFNName.c_str());
	if(result != GLOBUS_RLS_SUCCESS) {
		if(helper::globus_result_t_to_rls_ec(result) == GLOBUS_RLS_MAPPING_NEXIST) {
			return false;
		}
		else throw globus_rls_replica_adaptor::exception(result);
	}
	else return true;
}

///////////////////////////////////////////////////////////////////////////////
//
int RLSConnection::getPreferencesTimeout()
{
	// 30 secs. is the RLS default
	int timeout = 30;
	/*
	if (adaptorConfig.has_section("preferences")) {
        saga::ini::ini prefs = adaptorConfig.get_section ("preferences");
        std::string timeout_s(prefs.has_entry("timeout") ? prefs.get_entry("timeout") : "");
		
		try {
			timeout = boost::lexical_cast<int>(timeout_s);
		}
		catch(boost::bad_lexical_cast &)
		{
			// fallback to default timeout
			timeout = 30; 
		}
	}
	*/
	//std::cerr << "DEBUG: Timeout: " << timeout << std::endl; 
	
	return timeout;
}





} //namespace globus_rls_replica_adaptor
