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
 * $RCSfile: GrpcInfoResource.java,v $ $Revision: 1.4 $ $Date: 2006/10/11 08:13:49 $
 */
package org.apgrid.ninf.ng4.grpcinfo.impl;

import java.util.Iterator;

import javax.xml.namespace.QName;

import org.apache.axis.components.uuid.SimpleUUIDGen;
import org.apache.axis.components.uuid.UUIDGen;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.globus.wsrf.ResourceException;
import org.globus.wsrf.Resource;
import org.globus.wsrf.ResourceIdentifier;
import org.globus.wsrf.ResourceLifetime;
import org.globus.wsrf.ResourceProperties;
import org.globus.wsrf.ResourceProperty;
import org.globus.wsrf.ResourcePropertyMetaData;
import org.globus.wsrf.Topic;
import org.globus.wsrf.TopicList;
import org.globus.wsrf.TopicListAccessor;
import org.globus.wsrf.WSNConstants;
import org.globus.wsrf.impl.ReflectionResource;
import org.globus.wsrf.impl.ResourcePropertyTopic;
import org.globus.wsrf.impl.SimpleTopicList;

public class GrpcInfoResource extends ReflectionResource implements Resource, ResourceIdentifier, ResourceLifetime, ResourceProperties, TopicListAccessor {
	private static Log logger = LogFactory.getLog(GrpcInfoResource.class.getName());
	private TopicList topicList = null;

	private static class LazyUUIDGenHolder {
		public static UUIDGen uuidGen = new SimpleUUIDGen();
	}

	public void initialize(final Object resourceBean, final QName resourceElementQName) throws ResourceException {
		logger.trace("GrpcInfoResource#initialize() ent");
		{
			// we need local key here anyway, for persistency
			String uuid = LazyUUIDGenHolder.uuidGen.nextUUID();
			logger.info("key of singleton object is: " + uuid);
			super.initialize(resourceBean, resourceElementQName, uuid);
		}
		this.topicList = new SimpleTopicList(this);
		for (
				Iterator iter = this.getResourcePropertySet().iterator();
				iter.hasNext();
				/* nop */) {
			final ResourceProperty rp = (ResourceProperty)iter.next();
			if (rp instanceof ResourcePropertyTopic) {
				this.topicList.addTopic((Topic)rp);
			}
			//logger.debug("---------------------------");
			//logger.debug("rp == " + rp.toString());
			//logger.debug("rp.getMetaData().getName() == " + rp.getMetaData().getName().toString());
			//logger.debug("---------------------------");
		}
		logger.trace("GrpcInfoResource#initialize() fin");
	}

	protected ResourceProperty createNewResourceProperty(QName rpQName, Object resourceBean) throws Exception {
		logger.fatal("createNewResourceProperty(QName, Object) does not supported");
		throw new Exception();
	}

	/*
	 * this method is callbacked from super, during execution of super.initialize()
	 */
	protected ResourceProperty createNewResourceProperty(final ResourcePropertyMetaData rpMetaData, final Object resourceBean) throws Exception {
		logger.trace("createNewResourceProperty called");
		ResourceProperty rp = super.createNewResourceProperty(rpMetaData, resourceBean);
		if (!rp.getMetaData().getName().getNamespaceURI().equals(WSNConstants.BASEN_NS)) {
			rp = new ResourcePropertyTopic(rp);
		}
		//logger.debug("---------------------------");
		//logger.debug("rpTopic == " + rpTopic.toString());
		//logger.debug("rpTopic.getMetaData().getName() == " + rpTopic.getMetaData().getName().toString());
		//logger.debug("---------------------------");
		return rp;
	}

	public TopicList getTopicList() {
		return this.topicList;
	}
}
