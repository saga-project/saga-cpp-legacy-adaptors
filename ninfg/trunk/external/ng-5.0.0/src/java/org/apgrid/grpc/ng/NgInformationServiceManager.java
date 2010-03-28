/*
 * $RCSfile: NgInformationServiceManager.java,v $ $Revision: 1.14 $ $Date: 2008/03/25 05:39:07 $
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
package org.apgrid.grpc.ng;

import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.TimeUnit;

import org.apgrid.grpc.ng.info.RemoteExecutableInfo;
import org.apgrid.grpc.ng.info.RemoteExecutableInfoFactory;

import org.gridforum.gridrpc.GrpcException;

import static org.apgrid.grpc.ng.NgLog.CAT_NG_INTERNAL;

class REIEntry {
	public final int id;
	public final String value;
	public final boolean result; // true success, false failed

	public REIEntry(int qid, boolean result, String data) {
		this.id  = qid;
		this.result = result;
		this.value = data;
	}
	
}

class InformationPriority {
	private List<InformationSource> sources;
	private BlockingQueue<REIEntry> table;
	private NgLog logger;


	public InformationPriority(List<InformationSource> sources, NgLog logger) {
		this.sources = sources;
		this.table = new ArrayBlockingQueue<REIEntry>(32);
		for (InformationSource ent : this.sources) {
			ent.setContainer(this.table);
		}
		this.logger = logger;
	}

	protected void setContainer(InformationSource infoSrc) {
		infoSrc.setContainer(this.table);
	}

	public String getInformation(String classname, String hostname, String tag)
	 throws NgNoSuchInformationException {
		return getInfoBySerial(classname, hostname, tag);
	}

	private String getInfoBySerial(String classname, String hostname,
	 String tag)
	 throws NgNoSuchInformationException {
		logger.logInfo(CAT_NG_INTERNAL, "get Information serially");

		List<InformationSource> list = queryList(tag);
		logger.logDebug(CAT_NG_INTERNAL,
			"List of query Information Service:" + list.toString() );

		for (InformationSource is : list) {
			try {
				logger.logDebug(CAT_NG_INTERNAL, 
					"Query Executable Information about host[" 
					+ hostname + "] class[" + classname + "]" );
				is.query(classname, hostname);

				logger.logDebug(CAT_NG_INTERNAL,
					"Take a Remote Executable Information from Queue");
				long timeout = is.getTimeout();
				REIEntry ent = null;
				if (timeout <= 0) {
					ent = table.take();
				} else {
					ent = table.poll(timeout, TimeUnit.SECONDS);
				}
				if (ent == null) {
					logger.logWarn(CAT_NG_INTERNAL, "Timeout occurred in Information Service " + is.getType());
					continue;
				}
				if (ent.result) {
					return ent.value;
				}
				logger.logInfo(CAT_NG_INTERNAL, "Got unexpected information, ignore");
				logger.logDebug(CAT_NG_INTERNAL,  ent.value);
			} catch (InterruptedException e)  {
				logger.logError(CAT_NG_INTERNAL, 
					"Caught InterruptedException from table.take() method.");
				throw new NgNoSuchInformationException(
					"Caught InterruptedException from table.take() method.");
			} catch (NgException e) {
				logger.logError(CAT_NG_INTERNAL, e.getMessage());
				throw new NgNoSuchInformationException(e.getMessage());
			}
		}
		throw new NgNoSuchInformationException(
				"No Such Remote Executable Information about host[" 
				+ hostname + "] class[" + classname 
				+ "] information_source_tag[" + tag + "].");
	}

	private List<InformationSource> queryList(String tag) {
		if (tag == null) {
			return new ArrayList<InformationSource>(this.sources);
		}

		List<InformationSource> ret = new ArrayList<InformationSource>();
		for (InformationSource is : sources) {
			String t = is.getTag();
			if ((t != null) && tag.equals(t)) {
				ret.add(0, is);
			} else {
				ret.add(is);
			}
		}
		return ret;
	}

	private String getInfoByParallel(String classname, String hostname)
	 throws NgException {
		List<InformationSource> list = query(classname, hostname);
		List<REIEntry> entries = new ArrayList<REIEntry>(); 

		int all_sources = totalSource(list);
		for (;;) {
			REIEntry ent;
			try {
				ent = table.take();
			} catch (InterruptedException e) {
				break;
			}
			entries.add(ent);
			InformationSource high_prio = list.get(0);
			if (high_prio.requested(ent.id))  {
				return ent.value;
			}
			if (all_sources >= entries.size() )
				break;
			update_sources(list);
			all_sources = totalSource(list);
		}
		throw new NgNoSuchInformationException(
			"can't get Remote Executable Information");
	}

	private List<InformationSource> query(String classname, String hostname)
	 throws NgException {
		List<InformationSource> ret = new ArrayList<InformationSource>();
		for (InformationSource is : sources) {
			is.query(classname, hostname);
			ret.add(is);
		}
		return ret;
	}

	void update_sources(List<InformationSource> list) {
		for (Iterator<InformationSource> itr = list.iterator();
			 itr.hasNext(); ) {
			InformationSource is = itr.next();
			if (! is.valid() ) {
				itr.remove();
			}
		}
	}

	int totalSource(List<InformationSource> list) {
		int ret = 0;
		for (InformationSource is : list) {
			ret += is.nSources();
		}
		return ret;
	}

}

/*
 * - decide priority of getting informations.
 * - create InformationService ?
 * - return Remote Executable Information
 */
public class NgInformationServiceManager {

	// Information Service configuration 
	private NgInformationManager infoMng;
	private List<NgConfigSection> isconfig;
	private List<InformationSource> sources;
	private InformationPriority prio;
	private NgLog logger;

	/*
	 * Constructor
	 * 
	 * @param config configuration of Information Service.
	 * @param logger Logging class
	 */
	public NgInformationServiceManager(
		NgInformationManager infoMng, NgConfig config, NgLog logger)
	throws GrpcException {
		this.infoMng = infoMng;
		// copy Information Source configuration from NgConfig 
		this.isconfig =
			new ArrayList<NgConfigSection>(config.getInformationSource());
		this.logger = logger;

		logger.logDebug(CAT_NG_INTERNAL,
			"created NgInformationServiceManager");
	}

	/*
	 * Initialize Information Source.
	 */
	protected void initializeInformationSource() {
		this.sources = setupInformationServices();
		this.prio = new InformationPriority(this.sources, this.logger);
	}

	/*
	 * 
	 */
	protected void updateConfigSection(List<NgConfigSection> list)
	throws GrpcException {
		// Finds a new INFORMATION_SOURCE section.
		for (NgConfigSection cs : list) {
			String newType = null;
			String newTag = null;
			for (Iterator<NgConfigEntry> itr = cs.iterator(); itr.hasNext();) {
				NgConfigEntry ce = itr.next();
				if (ce.getKey().equals("type")) {
					newType = ce.getValue();
				} else if (ce.getKey().equals("tag")) {
					newTag = ce.getValue();
				}
			}
			if (newType == null) {
				throw new GrpcException("Information Source has no type");
			} else if (newTag == null) {
				throw new GrpcException("Information Source has no tag");
			}
			updateConfigSection(newType, newTag, cs);
	    	}
	}

	/*
	 *
	 */
	private void updateConfigSection(String newType, String newTag, NgConfigSection newCs)
	throws GrpcException {
		// Finds same INFORMATION_SOURCE from current.
		for (NgConfigSection cs : this.isconfig) {
			String type = null;
			String tag = null;
			for (Iterator<NgConfigEntry> itr = cs.iterator(); itr.hasNext();) {
				NgConfigEntry ce = itr.next();
				if (ce.getKey().equals("type")) {
					type = ce.getValue();
				} else if (ce.getKey().equals("tag")) {
					tag = ce.getValue();
				}
			}
			if (type == null) {
				throw new GrpcException("Information Source has no type");
			} else if (tag == null) {
				throw new GrpcException("Information Source has no tag");
			}

			if (newType.equals(type) && newTag.equals(tag)) {
				cs.update(newCs);
				return;
			}
		}

		isconfig.add(newCs);
	}

	/*
	 *
	 */
	protected void updateInformationSource(List<NgConfigSection> list)
	throws GrpcException {
		for (NgConfigSection cs : list) {
			String newType = null;
			String newTag = null;
			for (Iterator<NgConfigEntry> itr = cs.iterator(); itr.hasNext();) {
				NgConfigEntry ce = itr.next();
				if (ce.getKey().equals("type")) {
					newType = ce.getValue();
				} else if (ce.getKey().equals("tag")) {
					newTag = ce.getValue();
				}
			}
			if (newType == null) {
				throw new GrpcException("Information Source has no type");
			} else if (newTag == null) {
				throw new GrpcException("Information Source has no tag");
			}
			updateInformationSource(newType, newTag, cs);
	    	}
	}

	/*
	 *
	 */
	private void updateInformationSource(String newType, String newTag, NgConfigSection newCs)
	throws GrpcException {
		// Finds same INFORMATION_SOURCE from current.
		for (InformationSource infoSrc : this.sources) {
			String type = infoSrc.getType();
			String tag = infoSrc.getTag();
			if (type.equals(newType) && tag.equals(newTag)) {
				infoSrc.update(newCs);
				return;
			}
		}
		InformationSource infoSrc = new InformationSource(this.infoMng, newCs, logger);
		this.sources.add(infoSrc);
		this.prio.setContainer(infoSrc);
	}

	/*
	 *  
	 */
	private List<InformationSource> setupInformationServices() {
		if (this.isconfig == null)
			throw new IllegalStateException("isconfig field is null");

		List<InformationSource> ret = new ArrayList<InformationSource>();
		for (NgConfigSection sect : isconfig) {
			InformationSource isrc;
			try {
				isrc = new InformationSource(this.infoMng, sect, logger);
			} catch (NgException e) {
				logger.logWarn(CAT_NG_INTERNAL, e.getMessage());
				continue; // ignore that error was occurred
			}
			ret.add(isrc);
		}
		return ret;
	}

	/*
	 * 
	 */
	public RemoteExecutableInfo getRemoteExecutableInfo(String classname, 
	 String hostname, String priority)
	 throws GrpcException {
		if ((classname == null) || (hostname == null))
			throw new IllegalArgumentException("classname " + classname 
				+ " hostname " + hostname);

		// query & get info
		String xmlData;
		logger.logInfo(CAT_NG_INTERNAL,
			"get Remote Executable Information");

		xmlData = prio.getInformation(classname, hostname, priority);

		logger.logDebug(CAT_NG_INTERNAL,
			"Remote Executable Info is \n" + xmlData);

		// parser xml
		logger.logInfo(CAT_NG_INTERNAL,
			"Create Remote Class Information");

		RemoteExecutableInfo ret =
			RemoteExecutableInfoFactory.newRemoteExecutableInfo(xmlData);

		logger.logDebug(CAT_NG_INTERNAL,
			"Remote Class Info is \n" + ret.toString());

		return ret;
	}

	/*
	 *
	 */
	public String toString() {
		StringBuffer sb = new StringBuffer();

		sb.append("+ InformationSource\n");
		for (NgConfigSection cs: isconfig) {
			sb.append(cs.toString());
		}

		return sb.toString();
	}

	public void dispose() {
		logger.logInfo(CAT_NG_INTERNAL, "dispose Information Service.");
		for (InformationSource target : sources) {
			target.exit();
		}
		sources.clear();
		isconfig.clear();
	}

}
