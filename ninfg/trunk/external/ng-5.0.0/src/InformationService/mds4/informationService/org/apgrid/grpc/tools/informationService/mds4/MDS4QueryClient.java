/**
 * $RCSfile: MDS4QueryClient.java,v $
 * $AIST_Release: 5.0.0 $
 * $AIST_Copyright:
 *  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
 *  National Institute of Advanced Industrial Science and Technology
 *  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
 *  
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  
 *      http://www.apache.org/licenses/LICENSE-2.0
 *  
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *  $
 */
package org.apgrid.grpc.tools.informationService.mds4;

import java.rmi.RemoteException;

import javax.xml.rpc.ServiceException;
import javax.xml.rpc.Stub;

import org.apache.axis.message.MessageElement;
import org.globus.wsrf.WSRFConstants;
import org.globus.wsrf.client.BaseClient;
import org.oasis.wsrf.properties.InvalidQueryExpressionFaultType;
import org.oasis.wsrf.properties.InvalidResourcePropertyQNameFaultType;
import org.oasis.wsrf.properties.QueryEvaluationErrorFaultType;
import org.oasis.wsrf.properties.QueryExpressionType;
import org.oasis.wsrf.properties.QueryResourceProperties_Element;
import org.oasis.wsrf.properties.QueryResourceProperties_PortType;
import org.oasis.wsrf.properties.QueryResourcePropertiesResponse;
import org.oasis.wsrf.properties.ResourceUnknownFaultType;
import org.oasis.wsrf.properties.UnknownQueryExpressionDialectFaultType;
import org.oasis.wsrf.properties.WSResourcePropertiesServiceAddressingLocator;
import org.apache.axis.message.addressing.Address;
import org.apache.axis.message.addressing.EndpointReferenceType;
import org.apache.axis.types.URI.MalformedURIException;
import org.globus.wsrf.impl.security.authorization.HostAuthorization;
import org.globus.wsrf.impl.security.authorization.IdentityAuthorization;

public class MDS4QueryClient extends BaseClient {
    private static final int INVALID_TIMEOUT = -1;
    private int timeout = INVALID_TIMEOUT;

    /**
     * @param url
     * @param subject
     * @return
     * @throws GrpcException
     * @throws Exception
     */
    public MDS4QueryClient(String url, String subject)
	throws Query.Exception {
	String authTarget = (subject != null) ? subject : "host";

	// Set Anonymous.
	anonymous = Boolean.TRUE;

	// Set AUTHZ.
	authorization = (subject.equals("host")) ?
		HostAuthorization.getInstance() :
		new IdentityAuthorization(subject);

	// Set EPR.
	endpoint = new EndpointReferenceType();
	try {
		endpoint.setAddress(new Address(url));
	} catch (MalformedURIException e) {
		throw new Query.Exception(e);
	}
    }

    /**
     * @param xPath
     * @return
     * @throws Exception
     */
    MessageElement[] query(final String xPath)
	throws Query.TimeoutException, Query.Exception {
	final WSResourcePropertiesServiceAddressingLocator locator =
		new WSResourcePropertiesServiceAddressingLocator();
	final QueryExpressionType query = new QueryExpressionType();
	try {
		query.setDialect(WSRFConstants.XPATH_1_DIALECT);
	} catch (MalformedURIException e) {
		throw new Query.Exception(e);
	}
	query.setValue(xPath);
	
	QueryResourceProperties_PortType port;
	try {
		port = locator.getQueryResourcePropertiesPort(getEPR());
	} catch (ServiceException e) {
		throw new Query.Exception(e);
	}
	try {
		this.setOptions((Stub)port);
	} catch (Exception e) {
		throw new Query.Exception(e);
	}
	final QueryResourceProperties_Element request =
		new QueryResourceProperties_Element();
	request.setQueryExpression(query);
	
	// set timeout of query (If it's necessary).
	if (this.timeout != INVALID_TIMEOUT) {
		((org.apache.axis.client.Stub)port).setTimeout(
		    this.timeout * 1000);
	}
	
	QueryResourcePropertiesResponse response;
	try {
		response = port.queryResourceProperties(request);
	} catch (InvalidResourcePropertyQNameFaultType e) {
		throw new Query.Exception(e);
	} catch (InvalidQueryExpressionFaultType e) {
		throw new Query.Exception(e);
	} catch (QueryEvaluationErrorFaultType e) {
		throw new Query.Exception(e);
	} catch (ResourceUnknownFaultType e) {
		throw new Query.Exception(e);
	} catch (UnknownQueryExpressionDialectFaultType e) {
		throw new Query.Exception(e);
	} catch (RemoteException e) {
		throw new Query.TimeoutException(e);
	}
	
	return response.get_any();
    }

    /**
     * @param timeout
     */
    protected void setTimeout(int timeout) {
	this.timeout = timeout;
    }
}
