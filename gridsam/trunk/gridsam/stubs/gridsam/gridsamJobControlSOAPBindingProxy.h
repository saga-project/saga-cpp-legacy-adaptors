/* gridsamJobControlSOAPBindingProxy.h
   Generated by gSOAP 2.7.11 from ./wsdl/wsdl/gridsam.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef gridsamJobControlSOAPBindingProxy_H
#define gridsamJobControlSOAPBindingProxy_H
#include "gridsamH.h"

class SOAP_CMAC JobControlSOAPBindingProxy : public soap
{ public:
	/// Endpoint URL of service 'JobControlSOAPBindingProxy' (change as needed)
	const char *soap_endpoint;
	/// Constructor
	JobControlSOAPBindingProxy();
	/// Constructor with copy of another engine state
	JobControlSOAPBindingProxy(const struct soap&);
	/// Constructor with engine input+output mode control
	JobControlSOAPBindingProxy(soap_mode iomode);
	/// Constructor with engine input and output mode control
	JobControlSOAPBindingProxy(soap_mode imode, soap_mode omode);
	/// Destructor frees deserialized data
	virtual	~JobControlSOAPBindingProxy();
	/// Initializer used by constructor
	virtual	void JobControlSOAPBindingProxy_init(soap_mode imode, soap_mode omode);
	/// Disables and removes SOAP Header from message
	virtual	void soap_noheader();
	/// Get SOAP Fault structure (NULL when absent)
	virtual	const SOAP_ENV__Fault *soap_fault();
	/// Get SOAP Fault string (NULL when absent)
	virtual	const char *soap_fault_string();
	/// Get SOAP Fault detail as string (NULL when absent)
	virtual	const char *soap_fault_detail();
	/// Force close connection (normally automatic, except for send_X ops)
	virtual	int soap_close_socket();
	/// Print fault
	virtual	void soap_print_fault(FILE*);
#ifndef WITH_LEAN
	/// Print fault to stream
	virtual	void soap_stream_fault(std::ostream&);
	/// Put fault into buffer
	virtual	char *soap_sprint_fault(char *buf, size_t len);
#endif

	/// Web service operation 'terminateJob' (returns error code or SOAP_OK)
	virtual	int terminateJob(_gridsam__terminateJob *gridsam__terminateJob, _gridsam__terminateJobResponse *gridsam__terminateJobResponse);

	/// Web service operation 'startJob' (returns error code or SOAP_OK)
	virtual	int startJob(_gridsam__startJob *gridsam__startJob, _gridsam__startJobResponse *gridsam__startJobResponse);
};
#endif
