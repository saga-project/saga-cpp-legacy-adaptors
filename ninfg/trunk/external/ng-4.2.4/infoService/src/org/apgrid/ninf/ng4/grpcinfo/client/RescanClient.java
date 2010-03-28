/**
 * $AIST_Release: 4.2.4 $
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
 * $RCSfile: RescanClient.java,v $ $Revision: 1.3 $ $Date: 2006/10/11 08:13:49 $
 */
package org.apgrid.ninf.ng4.grpcinfo.client;

import java.net.MalformedURLException;
import java.net.URL;
import java.rmi.RemoteException;

import javax.xml.rpc.ServiceException;
import javax.xml.rpc.Stub;

import org.apache.commons.cli.ParseException;
import org.globus.wsrf.client.BaseClient;

import org.apgrid.ninf.ng4.grpcinfo.GrpcInfoPortType;
import org.apgrid.ninf.ng4.grpcinfo.types.RescanRequestType;
import org.apgrid.ninf.ng4.grpcinfo.service.GrpcInfoServiceLocator;

public class RescanClient extends BaseClient {
	class RescanClientException extends Exception {
		RescanClientException(final Exception e) {
			super(e);
		}
	}

	RescanClient() {}

	public static void main(final String[] args) throws RescanClientException {
		new RescanClient().rescan(args);
	}

	void rescan(final String[] args) throws RescanClientException {
		try {
			this.parse(args);
		} catch (final ParseException e) {
			throw new RescanClientException(e);
		} catch (final Exception e) {
			throw new RescanClientException(e);
		}
		final GrpcInfoPortType port = getPort(getURL());
		try {
			this.setOptions((Stub)port);
		} catch (final Exception e) {
			throw new RescanClientException(e);
		}
		final RescanRequestType request = new RescanRequestType();
		try {
			port.rescan(new RescanRequestType());
		} catch (final RemoteException e) {
			throw new RescanClientException(e);
		}
	}

	URL getURL() throws RescanClientException {
		final String urlString = this.getEPR().getAddress().toString();
		try {
			return new URL(urlString);
		} catch (final MalformedURLException e) {
			throw new RescanClientException(e);
		}
	}

	GrpcInfoPortType getPort(final URL url) throws RescanClientException {
		final GrpcInfoServiceLocator locator = new GrpcInfoServiceLocator();
		try {
			return locator.getGrpcInfoPortTypePort(url);
		} catch (final ServiceException e) {
			throw new RescanClientException(e);
		}
	}
}
