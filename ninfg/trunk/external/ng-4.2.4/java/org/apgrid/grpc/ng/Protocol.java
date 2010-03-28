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
 * $RCSfile: Protocol.java,v $ $Revision: 1.47 $ $Date: 2005/10/03 02:48:46 $
 */

package org.apgrid.grpc.ng;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.HashMap;
import java.util.Map;

import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public abstract class Protocol {
	protected int kind;
	protected int type;
	protected String strType;
	protected int sequence;
	protected int contextID;
	protected int executableID;
	protected int sessionID;
	protected int result;
	protected int length;
	protected byte[] headerData;
	protected byte[] paramData;
	private static Map mapBINConstructor = new HashMap();
	private static Map mapXMLConstructor = new HashMap();
	private static Map mapProtXMLName = new HashMap();
	protected NgLog commLog = null;
	
	/* protocol version */
	protected int versionMajor;
	protected int versionMinor;
	protected int versionPatch;
	
	private static final int TYPE_MASK = 0x00ffffff;
	private static final int PROT_OFFSET = 0x00000000;
	private static final int NOTIFY_OFFSET = 0x02000000;
	private static final int PROT_REQUEST = 0;
	private static final int PROT_REPLY = 1;
	private static final int PROT_NOTIFY = 2;
	private static final String STR_REQUEST = "Request";
	private static final String STR_REPLY = "Reply";
	private static final String STR_NOTIFY = "Notify";
	private static final String STR_PROT_REQUEST = "request";
	private static final String STR_PROT_REPLY = "reply";
	private static final String STR_PROT_NOTIFY = "notify";
	
	private static final int PROTOCOL_HEADER_LEN = 32;
	
	/* set name of package */
	private static final String pkgName =
		"org.apgrid.grpc.ng.protocol.";	
	private static final String PROT_FIELD_PROTOCOL_KIND =
		"PROTOCOL_KIND";
	private static final String PROT_FIELD_PROTOCOL_TYPE =
		"PROTOCOL_TYPE";
	private static final String PROT_FIELD_PROTOCOL_TYPE_XML_STR =
		"PROTOCOL_TYPE_XML_STR";
	
	/* set name for Class of indicate Protcol & Notify */
	static {
		try {
			/* QueryFunctionInformation */
			registerConstructor("ProtQueryFunctionInfo");
			
			/* QueryExecutableInformation */
			registerConstructor("ProtQueryExeStatus");
			
			/* ResetExecutable */
			registerConstructor("ProtResetExe");
			
			/* ExitExecutable */
			registerConstructor("ProtExitExe");
			
			/* InvokeSession */
			registerConstructor("ProtInvokeSession");
			
			/* SuspendSession */
			registerConstructor("ProtSuspendSession");
			
			/* ResumeSession */
			registerConstructor("ProtResumeSession");
			
			/* CancelSession */
			registerConstructor("ProtCancelSession");
			
			/* PullbackSession */
			registerConstructor("ProtPullbackSession");
			
			/* TransferArgumentData */
			registerConstructor("ProtTransferArgument");
			
			/* TransferResultData */
			registerConstructor("ProtTransferResult");
			
			/* TransferCallbackArgumentData */
			registerConstructor("ProtTransferCallbackArgument");
			
			/* TransferCallbackResultData */
			registerConstructor("ProtTransferCallbackResult");
			
			/* set name for Class of indicate Notification */
			/* IAmAliveNotify */
			registerNotifyConstructor("ProtIAmAlive");
			
			/* CalculationEndNotify */
			registerNotifyConstructor("ProtFunctionComplete");
			
			/* InvokeCallbackNotify */
			registerNotifyConstructor("ProtInvokeCallback");
		} catch (GrpcException e) {
			/* put Error Message to stderr */
			e.os = System.err;
			e.printStackTrace();
		}
	}
	
	/**
	 * @param typeString
	 * @return
	 */
	static int getProtTypeVal(String typeString) throws GrpcException {
		Integer returnValue = (Integer) mapProtXMLName.get(typeString);
		return returnValue.intValue() & TYPE_MASK;
	}
	
	/**
	 * 
	 */
	public Protocol() {
		/* nothing */
	}
	
	/**
	 * @param kind
	 * @param type
	 * @param sequenceNum
	 * @param contextID
	 * @param executableID
	 * @param sessionID
	 * @param result
	 */
	public Protocol(int kind, int type, int sequenceNum,
		int contextID, int executableID,int sessionID, int result,
		int versionMajor, int versionMinor, int versionPatch) {
		this.kind = kind;
		this.type = type;
		this.sequence = sequenceNum;
		this.contextID = contextID;
		this.executableID = executableID;
		this.sessionID = sessionID;
		this.result = result;
		
		/* set protocol version */
		this.versionMajor = versionMajor;
		this.versionMinor = versionMinor;
		this.versionPatch = versionPatch;
	}
	
	/**
	 * @param ngdi
	 * @throws GrpcException
	 */
	public Protocol(NgDataInputStream ngdi) throws GrpcException {
		this.sequence = ngdi.readInt();
		this.contextID = ngdi.readInt();
		this.executableID = ngdi.readInt();
		this.sessionID = ngdi.readInt();
		int dummy = ngdi.readInt();
		this.result = ngdi.readInt();
		this.length = ngdi.readInt();
		if (this.length >= 0) {
			this.paramData = new byte[this.length];
		}
		ngdi.readBytes(this.paramData, 0, this.length);
	}
	
	/**
	 * @param node
	 */
	public Protocol(Node node) throws GrpcException {
		try {
			String nodeName = node.getNodeName();
			if (nodeName.equals(STR_PROT_REQUEST)) {
				this.kind = PROT_REQUEST;
			} else if (nodeName.equals(STR_PROT_REPLY)) {
				this.kind = PROT_REPLY;
			} else if (nodeName.equals(STR_PROT_NOTIFY)) {
				this.kind = PROT_NOTIFY;
			} else {
				throw new NgExecRemoteMethodException("Protocol: Unknown protocol.");
			}
			this.type = getProtTypeVal(
				XMLUtil.getAttributeValue(node, "type"));
			this.sequence = Integer.parseInt(
				XMLUtil.getAttributeValue(node, "sequenceNo"));
			this.contextID = Integer.parseInt(
				XMLUtil.getAttributeValue(node, "contextID"));
			this.executableID = Integer.parseInt(
				XMLUtil.getAttributeValue(node, "executableID"));
			this.sessionID = Integer.parseInt(
				XMLUtil.getAttributeValue(node, "sessionID"));
			if (this.kind == PROT_REPLY) {
				this.result = Integer.parseInt(
					XMLUtil.getAttributeValue(node, "result"));
			} else {
				this.result = 0;
			}
		} catch (NgXMLReadException e) {
			throw new NgExecRemoteMethodException(e);
		}
	}
	
	/**
	 * @param target
	 * @throws GrpcException
	 */
	private static void registerConstructor(String target)
		throws GrpcException {
		putBINMap(target + STR_REQUEST);
		putBINMap(target + STR_REPLY);
		putXMLMap(target + STR_REQUEST);
		putXMLMap(target + STR_REPLY);
		putXMLProtcolNameMap(target + STR_REQUEST);
	}
	
	/**
	 * @param target
	 */
	private static void registerNotifyConstructor(String target)
		throws GrpcException {
		putBINMap(target + STR_NOTIFY);
		putXMLMap(target + STR_NOTIFY);		
		putXMLNotifyNameMap(target + STR_NOTIFY);
	}

	/**
	 * @param className
	 * @throws GrpcException
	 */
	private static void putBINMap(String className) throws GrpcException {
		String completeClassName = pkgName + className;
		Class[] parameterTypes = {NgDataInputStream.class};

		try {
			Class cl = Class.forName(completeClassName);
			Field kindField = cl.getField(PROT_FIELD_PROTOCOL_KIND);
			Field typeField = cl.getField(PROT_FIELD_PROTOCOL_TYPE);
			int kindAndType =
				(kindField.getInt(cl) << 24) + typeField.getInt(cl);

			mapBINConstructor.put(new Integer(kindAndType),
				cl.getConstructor(parameterTypes));
		} catch (SecurityException e) {
			throw new NgException(e);
		} catch (NoSuchMethodException e) {
			throw new NgException(e);
		} catch (ClassNotFoundException e) {
			throw new NgException(e);
		} catch (NoSuchFieldException e) {
			throw new NgException(e);
		} catch (IllegalArgumentException e) {
			throw new NgException(e);
		} catch (IllegalAccessException e) {
			throw new NgException(e);
		}
	}
	
	/**
	 * @param className
	 * @throws GrpcException
	 */
	private static void putXMLMap(String className) throws GrpcException {
		String completeClassName = pkgName + className;
		Class[] parameterTypes = {Node.class};

		try {
			Class cl = Class.forName(completeClassName);
			Field kindField = cl.getField(PROT_FIELD_PROTOCOL_KIND);
			Field typeField = cl.getField(PROT_FIELD_PROTOCOL_TYPE);
			int kindAndType =
				(kindField.getInt(cl) << 24) + typeField.getInt(cl);

			mapXMLConstructor.put(new Integer(kindAndType),
				cl.getConstructor(parameterTypes));
		} catch (SecurityException e) {
			throw new NgException(e);
		} catch (NoSuchMethodException e) {
			throw new NgException(e);
		} catch (ClassNotFoundException e) {
			throw new NgException(e);
		} catch (NoSuchFieldException e) {
			throw new NgException(e);
		} catch (IllegalArgumentException e) {
			throw new NgException(e);
		} catch (IllegalAccessException e) {
			throw new NgException(e);
		}		
	}
	
	/**
	 * @param className
	 * @throws GrpcException
	 */
	private static void putXMLProtcolNameMap(String className) throws GrpcException {
		putXMLNameMap(className, PROT_OFFSET);
	}

	/**
	 * @param className
	 * @throws GrpcException
	 */
	private static void putXMLNotifyNameMap(String className) throws GrpcException {
		putXMLNameMap(className, NOTIFY_OFFSET);
	}

	/**
	 * @param className
	 * @param offset
	 * @throws GrpcException
	 */
	private static void putXMLNameMap(String className, int offset) throws GrpcException {
		/* put XML Protocol name <-> ID into map */
		String completeClassName = pkgName + className;
		try {
			Class cl = Class.forName(completeClassName);

			Field typeField = cl.getField(PROT_FIELD_PROTOCOL_TYPE);
			Field nameField =
				cl.getField(PROT_FIELD_PROTOCOL_TYPE_XML_STR);
			String protXMLname = (String) nameField.get(cl);
			mapProtXMLName.put(protXMLname, new Integer(typeField.getInt(cl) + offset));
		} catch (ClassNotFoundException e) {
			throw new NgException(e);
		} catch (SecurityException e) {
			throw new NgException(e);
		} catch (NoSuchFieldException e) {
			throw new NgException(e);
		} catch (IllegalArgumentException e) {
			throw new NgException(e);
		} catch (IllegalAccessException e) {
			throw new NgException(e);
		}
		
	}

	/**
	 * @return
	 */
	protected abstract int getType();
	
	/**
	 * @param sb
	 * @return
	 */
	protected abstract StringBuffer appendXMLParameter(StringBuffer sb) throws GrpcException;
	
	/**
	 * @param ngdi
	 * @return
	 * @throws GrpcException
	 */
	protected static Protocol readProtocol(NgDataInputStream ngdi)
			throws GrpcException {
		int typeKind = ngdi.readInt();
		int kind = typeKind >>> 24;
		int type = typeKind & 0x0fff;
		
		/* get name for class of Protocol */
		String kindString = null;
		if (kind == PROT_REQUEST) {
			kindString = STR_REQUEST;
		} else if (kind == PROT_REPLY) {
			kindString = STR_REPLY;
		} else if (kind == PROT_NOTIFY) {
			kindString = STR_NOTIFY;
		}

		/* get Constructor for it */
		Constructor constructor =
			(Constructor)mapBINConstructor.get(new Integer(typeKind));
		/* check constructor */
		if (constructor == null) {
			throw new NgExecRemoteMethodException("constructor for id " +
				kind + "/" + type + " does not exist.");
		}
		/* set arguments for Constructor */
		Object[] arguments = {ngdi};
		
		try {
			/* create Request/Reply Protocol object */
			return (Protocol) constructor.newInstance(arguments);
		} catch (IllegalArgumentException e) {
			throw new NgExecRemoteMethodException(e);
		} catch (InstantiationException e) {
			throw new NgExecRemoteMethodException(e);
		} catch (IllegalAccessException e) {
			throw new NgExecRemoteMethodException(e);
		} catch (InvocationTargetException e) {
			throw new NgExecRemoteMethodException(e);
		}
	}

	/**
	 * @param str
	 * @return
	 */
	public static Protocol readProtocol(String str) throws GrpcException {
		return readProtocol(XMLUtil.getNode(str));
	}

	/**
	 * @param node
	 * @return
	 * @throws GrpcException
	 */
	protected static Protocol readProtocol(Node node) throws GrpcException {
		String tag = node.getNodeName();
		int kind = -1;
		try {
			if (tag.equals(STR_PROT_REQUEST)) {
				kind = PROT_REQUEST;
			} else if (tag.equals(STR_PROT_REPLY)) {
				kind = PROT_REPLY;				
			} else if (tag.equals(STR_PROT_NOTIFY)) {
				kind = PROT_NOTIFY;
			} else {
				throw new NgExecRemoteMethodException(
					"Protocol: unknown protocol.");
			}
			String typeString =
				XMLUtil.getAttributeValue(node, "type");
			int type = getProtTypeVal(typeString);
			
			/* get Constructor for it */
			int typeKind = (kind << 24) + type;
			Constructor constructor =
				(Constructor)mapXMLConstructor.get(new Integer(typeKind));
			/* set arguments for Constructor */
			Object[] arguments = {node};
		
			try {
				/* create Protocol object */
				return (Protocol) constructor.newInstance(arguments);
			} catch (IllegalArgumentException e) {
				throw new NgExecRemoteMethodException(e);
			} catch (InstantiationException e) {
				throw new NgExecRemoteMethodException(e);
			} catch (IllegalAccessException e) {
				throw new NgExecRemoteMethodException(e);
			} catch (InvocationTargetException e) {
				throw new NgExecRemoteMethodException(e);
			}
		} catch (NgXMLReadException e) {
			throw new NgExecRemoteMethodException(e);
		}
	}

	/**
	 * @return
	 * @throws GrpcException
	 */
	protected byte[] toByteArray() throws GrpcException {
		/* length of buffer */
		int bufferLength = PROTOCOL_HEADER_LEN + this.paramData.length;
		byte[] buffer = new byte[bufferLength];
		for (int i = 0; i < bufferLength; i++) {
			if (i < PROTOCOL_HEADER_LEN) {
				/* copy header data */
				buffer[i] = headerData[i];
			} else {
				/* copy param data */
				buffer[i] = paramData[i - PROTOCOL_HEADER_LEN];
			}
		}
		
		/* return created data */
		return buffer;
	}

	/**
	 * @return
	 */
	public String toXMLString() throws GrpcException {
		/* create header */
		StringBuffer sb = new StringBuffer();
		/* kind and type */
		String kindString = null;
		if (kind == PROT_REQUEST) {
			kindString = STR_PROT_REQUEST;
		} else if (kind == PROT_REPLY) {
			kindString = STR_PROT_REPLY;
		} else if (kind == PROT_NOTIFY) {
			kindString = STR_PROT_NOTIFY;
		}
		sb.append("<" + kindString + " ");
		sb.append("type=\"" + strType + "\" ");
		
		/* other information of header */
		sb.append("sequenceNo=\"" + sequence + "\" ");
		sb.append("contextID=\"" + contextID + "\" ");
		sb.append("executableID=\"" + executableID + "\" ");
		sb.append("sessionID=\"" + sessionID + "\"");

		/* append result if it's reply Protocol */
		if (kind == PROT_REPLY) {
			sb.append(" result=\"" + result + "\"");
		}

		/* close tag */
		sb.append(">");

		/* append body of Protocol */
		appendXMLParameter(sb);
		
		/* close tag of protocol */
		sb.append("</" + kindString + ">");

		/* return XML protocol string */
		return sb.toString();
	}
	
	/* (non-Javadoc)
	 * @see java.lang.Object#toString()
	 */
	public String toString() {
		/* check type of Protocol */
		String kindString;
		if (kind == PROT_REQUEST) {
			kindString = STR_REQUEST;
		} else if (kind == PROT_REPLY) {
			kindString = STR_REPLY;
		} else if (kind == PROT_NOTIFY) {
			kindString = STR_NOTIFY;
		} else {
			kindString = "unknown kind";
		}

		/* set ID for class */		
		int classID = type;
		if (kind == PROT_NOTIFY) {
			classID += NOTIFY_OFFSET;
		}
		
		/* return information of Protocol */
		return (String)mapProtXMLName.get(new Integer(classID)) + 
			kindString + ":: sequence: " + sequence + 
			", contextID: " + contextID + ", executableID: " +
			executableID + ", sessionID: " + sessionID +
			", result: " + result + ", length: " + length;
	}
	
	/**
	 * @param sendOrReceive
	 */
	protected void printCommLog(String sendOrReceive)
	{
		if (commLog != null) {
			/* append header information */
			commLog.printCommLog(sendOrReceive, (PROTOCOL_HEADER_LEN + length), getHeaderString());
		}
	}
	
	/**
	 * @param sendOrReceive
	 * @param msg
	 */
	protected void printCommLog(String sendOrReceive, String msg)
	{
		if (commLog != null) {
			/* append header information */
			commLog.printCommLog(sendOrReceive, (PROTOCOL_HEADER_LEN + length), getHeaderString() + msg);
		}
	}
	
	/**
	 * @param sendOrReceive
	 */
	protected void printHeaderCommLog(String sendOrReceive)
	{
		if (commLog != null) {
			/* print header information */
			commLog.printCommLog(sendOrReceive, PROTOCOL_HEADER_LEN, getHeaderString());
		}
	}
	
	/**
	 * @param sendOrReceive
	 * @param data
	 */
	protected void printBodyCommLog(String sendOrReceive, byte[] data)
	{
		if (commLog != null) {
			StringBuffer sb = new StringBuffer();
			int offset = 0;
			while (offset < data.length) {
				sb.append(dumpCommLog(data, offset, 16));
				offset += 16;			
			} 
			
			/* print data information */
			commLog.printCommLog(sendOrReceive, data.length, sb.toString());
		}
	}
	
	/**
	 * @param sendOrReceive
	 * @param data
	 * @param length
	 */
	protected void printBodyCommLog(String sendOrReceive, byte[] data, int length)
	{
		if (commLog != null) {
			StringBuffer sb = new StringBuffer();
			int offset = 0;
			while (offset < length) {
				sb.append(dumpCommLog(data, offset, offset + 16 > length ? length % 16 : 16));
				offset += 16;			
			} 
			
			/* print data information */
			commLog.printCommLog(sendOrReceive, length, sb.toString());
		}
	}
	
	/**
	 * @return
	 */
	protected String getHeaderString() {
		if (headerData == null) {
			return null;
		}

		StringBuffer sb = new StringBuffer();
		
		int offset = 0;
		while (offset < PROTOCOL_HEADER_LEN) {
			sb.append(dumpCommLog(headerData, offset, 16));
			offset += 16;
		}
		
		return sb.toString();
	}
	
	/**
	 * @param target
	 * @param offset
	 * @return
	 */
	protected String dumpCommLog(byte[] target, int offset, int length) {
		StringBuffer sb = new StringBuffer();
		/* indent */
		sb.append("  ");
		
		/* offset */
		for (int i = 3; i > 0; i--) {
			if (((offset >> (i * 8)) & 0xff) > 0) {
				sb.append(
					getHexString((byte)((offset >> (i * 8)) & 0xff)));
			} else {
				sb.append("  ");
			}
		}
		sb.append(getHexString((byte)(offset & 0xff)));
		sb.append("  ");
		
		/* data */
		for (int i = 0; i < 16; i++) {
			if ((i > length) || ((offset + i) >= target.length)) {
				sb.append("  ");
			} else {
				sb.append(getHexString(target[offset + i]));
			}
			if (((i + 1) % 4) == 0) {
				sb.append("  ");
			}
		}
		sb.append("    ");
		
		/* char */
		for (int i = 0; i < 16; i++) {
			if ((i > length) || ((offset + i) >= target.length)) {
				break;
			}
			if (Character.isLetterOrDigit((char)((target[offset + i])))) {
				sb.append(Character.toString((char) target[offset + i]));
			} else {
				sb.append(".");
			}
		}
		sb.append("\n");
		
		return sb.toString();
	}
	
	/**
	 * @param ch
	 * @return
	 */
	protected String getHexString(byte ch) {
		String hexStr =
			Integer.toHexString((((ch >> 4) & 0x0f) * 16) + (ch & 0x0f));
		StringBuffer sb = new StringBuffer();
		if (hexStr.length() < 2) {
			sb.append("0");
		}
		sb.append(hexStr);
		return sb.toString();
	}
	
	/**
	 * @throws GrpcException
	 */
	protected void setupHeader() throws GrpcException {
		ByteArrayOutputStream bo = new ByteArrayOutputStream(NgGlobals.smallBufferSize);
		XDROutputStream xo = new XDROutputStream(bo);

		int typeAndKind = (kind << 24) + type;
		try {
			xo.writeInt(typeAndKind);
			xo.writeInt(sequence);
			xo.writeInt(contextID);
			xo.writeInt(executableID);
			xo.writeInt(sessionID);
			xo.writeInt(0);  /* padding */
			xo.writeInt(result);
			if (paramData == null) {
				xo.writeInt(0);
			} else {
				xo.writeInt(paramData.length);
			}
		} catch (IOException e) {
			throw new NgIOException(e);
		}

		/* set Data to headerData */
		this.headerData = bo.toByteArray();
		xo.close();
	}
	
	/**
	 * @param header
	 */
	protected void setHeader (byte[] header) {
		this.headerData = header;
	}

	/**
	 * @throws GrpcException
	 */
	protected void setupParameter() throws GrpcException {
		/* nothing */
	}
	
	/**
	 * @param param
	 */
	protected void setParameter (byte[] param) {
		this.paramData = param;
		this.length = param.length;
	}
	
	/**
	 * @param wire
	 * @param prot
	 * @throws GrpcException
	 */
	protected abstract void parseParam(Wire wire, Protocol prot) throws GrpcException;
	
	/**
	 * @param versionMajor
	 * @param versionMinor
	 * @param versionPatch
	 */
	protected void setVersion(int versionMajor,
		int versionMinor, int versionPatch) {
			this.versionMajor = versionMajor;
			this.versionMinor = versionMinor;
			this.versionPatch = versionPatch;
		}

	/**
	 * @param wire
	 * @throws GrpcException
	 */
	protected void sendBINDataToWire(Wire wire) throws GrpcException {
		wire.sendBytes(this.headerData);
		if (this.paramData != null) {
			wire.sendBytes(this.paramData);
		}
		/* print commLog */
		printCommLog(NgLog.COMMLOG_SEND);
	}
	
	/**
	 * @param commLog
	 */
	protected void setCommLog(NgLog commLog) {
		this.commLog = commLog;
	}
}
