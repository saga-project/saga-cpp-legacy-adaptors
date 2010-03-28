/**
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
 * $RCSfile: GrpcInfoHome.java,v $ $Revision: 1.6 $ $Date: 2008/02/06 10:38:30 $
 */
package org.apgrid.ninf.ng5.grpcinfo.impl;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FilenameFilter;
import java.io.InputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Iterator;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import javax.xml.namespace.QName;
import javax.xml.parsers.ParserConfigurationException;
import javax.xml.soap.SOAPException;

import org.apache.axis.message.MessageElement;
import org.apache.axis.utils.XMLUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apgrid.ninf.ng5.grpcinfo.types.GrpcInfoRPType;
import org.apgrid.ninf.ng5.grpcinfo.types.GrpcInfoSetType;
import org.globus.mds.servicegroup.client.ServiceGroupRegistrationParameters;
import org.globus.wsrf.Resource;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.ResourceHome;
import org.globus.wsrf.ResourceProperty;
import org.globus.wsrf.config.ContainerConfig;
import org.globus.wsrf.impl.SingletonResourceHome;
import org.globus.wsrf.impl.servicegroup.client.ServiceGroupRegistrationClient;
import org.globus.wsrf.jndi.Initializable;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.SAXException;

public class GrpcInfoHome extends SingletonResourceHome implements ResourceHome, Initializable {

	private static Log logger = LogFactory.getLog(GrpcInfoHome.class.getName());
	private static final String NG5EITYPES_NS = "http://ninf.apgrid.org/ng5/grpcinfo/types";

	/* config resource */
	private String infoDirPath = null;
	private String hostName = null;
	private Integer mpiCpus = null;
	private boolean firstScan = true;

	/* config resource setter */
	public void setInfoDirPath(String infoDirPath) {
		this.infoDirPath = infoDirPath;
	}

	public void setHostName(String hostName) {
		this.hostName = hostName;
	}

	public void setMpiCpus(String mpiCpus) {
		this.mpiCpus = Integer.valueOf(mpiCpus);
	}

	/* singleton resource */
	private static class LazyGrpcInfoResourceHolder {
		public static GrpcInfoResource grpcInfoResource = new GrpcInfoResource();
	}

	public GrpcInfoHome() {
		logger.trace("GrpcInfoHome() called");
	}

	public void initialize() throws ResourceException {
		logger.trace("GrpcInfoHome#initialize() ent");
		final GrpcInfoRPType grpcInfoRP = new GrpcInfoRPType();
		grpcInfoRP.setGrpcInfoSet(new GrpcInfoSetType(new MessageElement[] {new MessageElement()}));
		synchronized (this) {
			LazyGrpcInfoResourceHolder.grpcInfoResource.initialize(
				grpcInfoRP,
				new QName(NG5EITYPES_NS, "grpcInfoRP"));
			this.rescan();
		}
		logger.trace("registering to ServiceGroupRegistrationClient.containerClient");
		String regPath = ContainerConfig.getGlobusLocation() + "/etc/ng5grpcinfo/regist.xml";
		ServiceGroupRegistrationParameters params = null;
		try {
			params = ServiceGroupRegistrationClient.readParams(regPath);
		} catch (Exception e) {
			logger.error("failed to read regist.xml");
		}
		if (params == null) {
			return;
		}
		final ServiceGroupRegistrationClient containerClient = ServiceGroupRegistrationClient.getContainerClient();
		containerClient.register(params);
		new Timer(true).scheduleAtFixedRate(new TimerTask() {
			public void run() {
				GrpcInfoHome.this.rescan();
				if (GrpcInfoHome.this.firstScan) {
					GrpcInfoHome.this.firstScan = false;
					GrpcInfoHome.logger.info("done rescan to regist");
				}
			}
		} , 30000, 59000);
		logger.trace("registered to ServiceGroupRegistrationClient.containerClient");
		logger.trace("GrpcInfoHome#initialize() fin");
	}

	protected Resource findSingleton() {
		return LazyGrpcInfoResourceHolder.grpcInfoResource;
	}

	synchronized void rescan() {
		if (this.infoDirPath == null) {
			logger.error("infoDirPath parameter isn't set. rescan aborted");
			return;
		}
		final File dir = new File(this.infoDirPath);
		if (!dir.exists()) {
			logger.error("path in infoDirPath doesn't exist. rescan aborted");
			return;
		}
		if (!dir.isDirectory()) {
			logger.error("path in infoDirPath doesn't directory. rescan aborted");
			return;
		}
		final List infoList = new ArrayList();
		collectExecInfo(dir, infoList);
		final GrpcInfoSetType newSet = new GrpcInfoSetType((MessageElement[])infoList.toArray(new MessageElement[] {}));
		final ResourceProperty rp = LazyGrpcInfoResourceHolder.grpcInfoResource.getResourcePropertySet().get(new QName(NG5EITYPES_NS, "grpcInfoSet"));
		rp.set(0, newSet);
	}

	private void collectExecInfo(final File dir, final List infoList) {
		for (
				final Iterator iter = Arrays.asList(
					dir.listFiles(
						new FilenameFilter() {
							public boolean accept(File dir, String name) {
								// "name" does glob-match?("*.nrf")
								return name.endsWith(".nrf");
							}
						} )).iterator();
				iter.hasNext();
				/* nop */) {
			final File file = (File)iter.next();
			BufferedReader bufferedReader = null;
			try {
				bufferedReader = new BufferedReader(new FileReader(file));
			} catch (FileNotFoundException e) {
				// ignore, this may happen when file is deleted between scan and open
				continue;
			}
			if (bufferedReader == null) {
				logger.error("failed to build BufferedReader, this shouldn't be happen. please contact");
				continue;
			}
			MessageElement execInfo = null;
			try {
				execInfo = buildExecInfo(bufferedReader);
			} catch (IOException e) {
				logger.error("IOException in reading NRF file");
			} finally {
				try {
					bufferedReader.close();
				} catch (IOException e) {
					logger.error("IOException in closing NRF file");
				}
			}
			if (execInfo != null) {
				infoList.add(execInfo);
			}
		}
	}

	private MessageElement buildExecInfo(BufferedReader reader) throws IOException {
		final StringBuffer buf = new StringBuffer();
		String line;
		while ((line = reader.readLine()) != null) {
			if (line.length() != 0) {
				buf.append(line);
				buf.append("\n");
			}
		}
		final String classInfoString = buf.toString();
		if (classInfoString == null) {
			return null;
		}
		InputStream is = null;
		Document document = null;
		try {
			is = new ByteArrayInputStream(classInfoString.getBytes());
			document = XMLUtils.newDocument(is);
		} catch (ParserConfigurationException e) {
			logger.error("ParserConfigurationException in building execinfo");
			return null;
		} catch (SAXException e) {
			logger.error("SAXException in building execinfo");
			return null;
		} finally {
			if (is != null) {
				is.close();
			}
		}
		is = null;
		if (document == null) {
			return null;
		}
		final Element execInfo = document.createElementNS("http://ninf.apgrid.org/ng5", "ng5:execInfo");
		execInfo.appendChild(document.getDocumentElement());
		return new MessageElement(execInfo);
	}
}
