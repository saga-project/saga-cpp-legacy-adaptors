/*
 * Copyright (C) 2008-2011 Osaka University.
 * Copyright (C) 2008-2011 National Institute of Informatics in Japan.
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
package org.naregi.rns.client.fuse;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.xml.transform.TransformerException;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI.MalformedURIException;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.axis.message.addressing.ReferenceParametersType;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.SerializationException;
import org.naregi.rns.ACL;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSDirHandle;
import org.naregi.rns.client.RNSDirent;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSExtensionClient;
import org.naregi.rns.client.RNSKeyValue;
import org.naregi.rns.client.RNSStat;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.util.TimeoutCacheMap;

import fuse.Errno;
import fuse.Filesystem3;
import fuse.FuseDirFiller;
import fuse.FuseException;
import fuse.FuseFS;
import fuse.FuseFtypeConstants;
import fuse.FuseGetattrSetter;
import fuse.FuseMount;
import fuse.FuseOpenSetter;
import fuse.FuseSizeSetter;
import fuse.FuseStatfsSetter;
import fuse.XattrLister;
import fuse.XattrSupport;

public class RNSFS_FUSE implements Filesystem3, XattrSupport {
	private static final Log log = LogFactory.getLog(RNSFS_FUSE.class);

	private RNSClientHome home;
	private RNSClient rnsClient;
	private RNSExtensionClient extensionClient;

	private int rnsErrorToErrno(RNSError e) {
		RNSError.Errno no = e.getError();
		if (no == RNSError.Errno.EACCES) {
			return Errno.EACCES;
		} else if (no == RNSError.Errno.EBUSY) {
			return Errno.EBUSY;
		} else if (no == RNSError.Errno.EEXIST) {
			return Errno.EEXIST;
		} else if (no == RNSError.Errno.EINVAL) {
			return Errno.EINVAL;
		} else if (no == RNSError.Errno.EISDIR) {
			return Errno.EISDIR;
		} else if (no == RNSError.Errno.ELOOP) {
			return Errno.ELOOP;
		} else if (no == RNSError.Errno.ENET) {
			return Errno.ENETRESET;
		} else if (no == RNSError.Errno.ENOENT) {
			return Errno.ENOENT;
		} else if (no == RNSError.Errno.ENOTDIR) {
			return Errno.ENOTDIR;
		} else if (no == RNSError.Errno.ENOTEMPTY) {
			return Errno.ENOTEMPTY;
		} else if (no == RNSError.Errno.ENOTSUPP) {
			return Errno.ENOTSUPP;
		} else if (no == RNSError.Errno.EUNEXPECTED) {
			return Errno.EBADMSG;
		} else {
			return Errno.EBADE; // ?
		}
	}

	private boolean debug = false;

	private void debug(String name, String val) {
		if (!debug) {
			return;
		}
		System.out.println("[" + name + "]" + val);
	}

	public RNSFS_FUSE() throws Exception {
		home = new RNSClientHome();
		String[] args2 = {};
		home.parseArgs(args2, 0, 0);
		rnsClient = home.getRNSClient();
		if (rnsClient == null) {
			return;
		}
		extensionClient = home.getRNSExtensionClient();
	}

	public int chmod(String path, int mode) throws FuseException {
		debug("chmod", path);
		String ownerPerm = ACL.permToString((short) ((mode & 0700) >> 6));
		String ownerGroupPerm = ACL.permToString((short) ((mode & 070) >> 3));
		String otherPerm = ACL.permToString((short) (mode & 07));

		String aclSpec = "ou::" + ownerPerm + ",og::" + ownerGroupPerm + ",o:"
				+ otherPerm;
		try {
			extensionClient.setACL(path, aclSpec);
			return 0;
		} catch (Exception e) {
			System.out.println(e.toString());
			return Errno.EACCES;
		}
	}

	public int chown(String path, int uid, int gid) throws FuseException {
		debug("chown", path);
		return Errno.ENOSYS;
	}

	private static final String MODE_EPR = ".epr";
	private static final String MODE_URL = ".url";
	private static final String MODE_XML = ".xml";

	private String cutRnsSuffix(String path) {
		if (path.endsWith(MODE_EPR) || path.endsWith(MODE_URL)
				|| path.endsWith(MODE_XML)) {
			if (path.length() == 4) {
				path = "/";
			} else {
				path = path.substring(0, path.length() - 4);
				debug("cutRnsSuffix", "path = " + path);
			}
			return path;
		} else {
			return null;
		}
	}

	private String getRnsSuffix(String path) {
		if (path.endsWith(MODE_EPR)) {
			return MODE_EPR;
		} else if (path.endsWith(MODE_URL)) {
			return MODE_URL;
		} else if (path.endsWith(MODE_XML)) {
			return MODE_XML;
		} else {
			return null;
		}
	}

	private int getInoFromEPR(EndpointReferenceType epr) {
		int i = epr.getAddress().toString().hashCode();
		ReferenceParametersType param = epr.getParameters();
		if (param != null) {
			MessageElement[] mes = param.get_any();
			if (mes != null && mes.length >= 1) {
				i = i + mes[0].getValue().hashCode();
			}
		}
		return Math.abs(i);
	}

	private Map<String, String> tmpPaths = Collections.synchronizedMap(new HashMap<String, String>());

	public int getattr(String path, FuseGetattrSetter getattrSetter)
			throws FuseException {
		debug("getattr", "path=" + path);
		if (tmpPaths.containsKey(path)) {
			int time = (int) (System.currentTimeMillis() / 1000L);
			getattrSetter.set(path.hashCode(),
					FuseFtypeConstants.TYPE_FILE | 0600, 1, 0, 0, 0, 0, 10,
					time, time, time);
			return 0;
		}
		String pathOrig = null;
		try {
			boolean haveSuffix = false;
			if (rnsClient.exists(path) == false) {
				pathOrig = path;
				String tmpPath = cutRnsSuffix(path);
				if (tmpPath != null) {
					haveSuffix = true;
					path = tmpPath;
				}
			}
			boolean isdir = false;
			if (haveSuffix == false) {
				isdir = rnsClient.isDirectory(path);
			}

			EndpointReferenceType epr = rnsClient.getEPR(path, true);
			if (isdir) {
				RNSStat st = rnsClient.stat(path);
				int atime = (int) (st.getAccessTime().getTimeInMillis() / 1000L);
				int mtime = (int) (st.getModificationTime().getTimeInMillis() / 1000L);
				int ctime = (int) (st.getCreateTime().getTimeInMillis() / 1000L);
				int nlink = st.getElementCount().intValue() + 2;
				int uid;
				int gid;
				int mode;
				try {
					ACL acl = extensionClient.getACL(path, true);
					mode = ((ACL.PERM_ALL & acl.getOwnerPerm()) << 6)
							+ ((ACL.PERM_ALL & acl.getOwnerGroupPerm()) << 3)
							+ (ACL.PERM_ALL & acl.getOtherPerm());
					uid = acl.getOwner().hashCode(); /* getpwnam? */
					gid = acl.getOwnerGroup().hashCode(); /* getgrnam? */
				} catch (Exception e) {
					mode = 0;
					uid = 0;
					gid = 0;
					e.printStackTrace();
				}
				getattrSetter.set(getInoFromEPR(epr),
						FuseFtypeConstants.TYPE_DIR | mode, nlink, uid, gid, 0,
						4096, 4, atime, mtime, ctime);
			} else {
				int size;
				if (haveSuffix && pathOrig != null && pathOrig.endsWith(".xml")) {
					try {
						String xmlstr = getXMLString(path);
						size = xmlstr.getBytes().length;
					} catch (Exception e) {
						e.printStackTrace();
						size = 65536;
					}
				} else {
					String eprstr;
					try {
						eprstr = RNSUtil.toXMLString(epr);
					} catch (SerializationException e) {
						e.printStackTrace();
						return Errno.EBADMSG;
					}
					size = eprstr.getBytes().length;
				}
				int time = (int) (System.currentTimeMillis() / 1000L);
				getattrSetter.set(getInoFromEPR(epr),
						FuseFtypeConstants.TYPE_FILE | 0600, 1, 0, 0, 0, size,
						10, time, time, time);
			}
			return 0;
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
	}

	public int getdir(String path, FuseDirFiller filler) throws FuseException {
		try {
			EndpointReferenceType dot = rnsClient.getEPR(path, true);
			EndpointReferenceType dotdot = rnsClient.getEPR(
					RNSUtil.getDirname(path), true);
			filler.add(".", getInoFromEPR(dot), FuseFtypeConstants.TYPE_DIR);
			filler.add("..", getInoFromEPR(dotdot), FuseFtypeConstants.TYPE_DIR);

			RNSDirHandle dir = rnsClient.list(path, false);
			if (dir == null) {
				return 0;
			}
			for (RNSDirent ent : dir) {
				if (ent == null) {
					continue;
				}
				EndpointReferenceType epr = ent.getEpr();
				int type = 0;
				boolean isdir = RNSUtil.isDirectory(ent.getMeta());
				if (isdir) {
					type = FuseFtypeConstants.TYPE_DIR;
				} else {
					type = FuseFtypeConstants.TYPE_FILE;
				}
				if (type > 0) {
					filler.add(ent.getName(), getInoFromEPR(epr), type);
				}
			}
			RNSError e2 = dir.getError();
			if (e2 != null) {
				throw e2;
			}
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int link(String from, String to) throws FuseException {
		try {
			rnsClient.copyEntry(from, to);
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int mkdir(String path, int mode) throws FuseException {
		try {
			rnsClient.mkdir(path);
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int rename(String from, String to) throws FuseException {
		try {
			rnsClient.rename(from, to);
			clearXattrCache(from);
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int rmdir(String path) throws FuseException {
		try {
			rnsClient.rmdir(path);
			clearXattrCache(path);
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int statfs(FuseStatfsSetter statfsSetter) throws FuseException {
		return Errno.ENOSYS;
	}

	public int symlink(String from, String to) throws FuseException {
		return Errno.ENOSYS;
	}

	public int truncate(String path, long size) throws FuseException {
		debug("truncate", path);
		return 0;
	}

	public int unlink(String path) throws FuseException {
		try {
			rnsClient.rmJunction(path);
			clearXattrCache(path);
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int utime(String path, int atime, int mtime) throws FuseException {
		debug("utime", path);
		return 0;
	}

	public int readlink(String path, CharBuffer link) throws FuseException {
		return Errno.ENOSYS;
	}

	private static class FH {
		String mode;
		String path;
		byte[] current;
		ByteArrayOutputStream baos;
	}

	public int mknod(String path, int mode, int rdev) throws FuseException {
		debug("mknod", "path=" + path + ", rdev=" + rdev);
		if ((FuseFtypeConstants.TYPE_FILE & mode) > 0) {
			tmpPaths.put(path, "");
			return 0;
		} else {
			return Errno.EPERM;
		}
	}

	private static final int O_ACCMODE = 03;

	public int open(String path, int flags, FuseOpenSetter openSetter)
			throws FuseException {
		try {
			String cutPath = path;
			FH fh = new FH();
			if (rnsClient.exists(path)) {
				fh.mode = MODE_EPR;
			} else {
				fh.mode = getRnsSuffix(path);
				if (fh.mode == null) {
					fh.mode = MODE_EPR;
				} else {
					String tmpPath = cutRnsSuffix(path);
					if (tmpPath == null) {
						/* unexpected */
						debug("open", "EINVAL 1");
						return Errno.EINVAL;
					}
					cutPath = tmpPath;
				}
			}
			if (tmpPaths.containsKey(path)) {
				tmpPaths.remove(path);
				if (fh.mode.equals(MODE_XML)) {
					debug("open", "EINVAL 2");
					return Errno.EINVAL;
				} else if ((flags & O_ACCMODE) == FuseFS.O_RDONLY) {
					debug("open", "EINVAL 3");
					return Errno.EINVAL;
				}
				/* creating new entry */
			} else {
				/* existing entry */
				if ((flags & O_ACCMODE) != FuseFS.O_RDONLY) {
					if (fh.mode.equals(MODE_XML) == false) {
						debug("open", "EINVAL 4");
						return Errno.EINVAL;
					}
				}
				if (fh.mode.equals(MODE_EPR)) {
					EndpointReferenceType epr = rnsClient.getEPR(cutPath, true);
					String str = RNSUtil.toXMLString(epr);
					fh.current = str.getBytes();
				} else if (fh.mode.equals(MODE_URL)) {
					EndpointReferenceType epr = rnsClient.getEPR(cutPath, true);
					String str = epr.getAddress().getValue().toString();
					fh.current = str.getBytes();
				} else if (fh.mode.equals(MODE_XML)) {
					MessageElement[] mes = rnsClient.getMetadata(cutPath);
					StringWriter sw = new StringWriter();
					PrintWriter pw = new PrintWriter(sw);
					if (mes != null) {
						boolean plural = false;
						for (MessageElement me : mes) {
							if (plural) {
								pw.println("----");
							}
							pw.print(RNSUtil.toIndentedXML(me));
							plural = true;
						}
					}
					pw.flush();
					fh.current = sw.toString().getBytes();
				} else {
					debug("open", "EINVAL 5");
					return Errno.EINVAL;
				}
			}
			fh.path = cutPath;
			openSetter.setFh(fh);
			return 0;
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		} catch (SerializationException e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		} catch (TransformerException e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		} catch (Exception e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		}
	}

	public int write(String path, Object fh, boolean isWritepage,
			ByteBuffer buf, long offset) throws FuseException {
		debug("write", "path=" + path + ", offset=" + offset + ", isWritepage="
				+ isWritepage);
		if (fh instanceof FH == false) {
			return Errno.ENOSYS;
		}
		FH f = (FH) fh;
		if (f.baos == null) {
			f.baos = new ByteArrayOutputStream();
		}
		try {
			if (f.baos.size() != offset) {
				/* cannot access random position */
				return Errno.EINVAL;
			}
			byte[] b = new byte[buf.limit()];
			buf.get(b);
			f.baos.write(b);
			return 0;
		} catch (IOException e) {
			e.printStackTrace();
			return Errno.EIO;
		}
	}

	public int read(String path, Object fh, ByteBuffer buf, long offset)
			throws FuseException {
		if (fh instanceof FH == false) {
			return Errno.EBADF;
		}
		byte[] b = ((FH) fh).current;
		if (b == null) {
			return 0;
		}
		int len = b.length - (int) offset;
		if (len <= 0) {
			return 0;
		}
		buf.put(b, (int) offset, Math.min(buf.remaining(), len));
		return 0;
	}

	public int flush(String path, Object fh) throws FuseException {
		if (fh instanceof FH == false) {
			return Errno.EBADF;
		}
		FH f = (FH) fh;
		if (f.baos != null) {
			String str = f.baos.toString();
			debug("flash", str);
			if (f.mode.equals(MODE_EPR)) {
				try {
					EndpointReferenceType epr = RNSUtil.toEPR(str);
					rnsClient.addJunction(f.path, epr);
					return 0;
				} catch (DeserializationException e) {
					e.printStackTrace();
					return Errno.EBADMSG;
				} catch (IOException e) {
					e.printStackTrace();
					return Errno.EIO;
				} catch (RNSError e) {
					return rnsErrorToErrno(e);
				}
			} else if (f.mode.equals(MODE_URL)) {
				try {
					EndpointReferenceType epr = new EndpointReferenceType();
					epr.setAddress(new Address(str));
					rnsClient.addJunction(f.path, epr);
					return 0;
				} catch (MalformedURIException e) {
					e.printStackTrace();
					return Errno.EINVAL;
				} catch (RNSError e) {
					return rnsErrorToErrno(e);
				}
			} else if (f.mode.equals(MODE_XML)) {
				try {
					rnsClient.setMetadata(f.path,
							RNSUtil.toMessageElements(str));
					clearXattrCache(f.path);
					return 0;
				} catch (RNSError e) {
					return rnsErrorToErrno(e);
				} catch (Exception e) {
					e.printStackTrace();
					return Errno.EINVAL;
				}
			} else {
				return Errno.EINVAL;
			}
		}
		return 0;
	}

	public int fsync(String path, Object fh, boolean isDatasync)
			throws FuseException {
		if (fh instanceof FH)
			return 0;
		return Errno.EBADF;
	}

	public int release(String path, Object fh, int flags) throws FuseException {
		if (fh instanceof FH) {
			System.runFinalization();
			return 0;
		}
		return Errno.EBADF;
	}

	private String getEPRString(String path) throws SerializationException,
			RNSError {
		EndpointReferenceType epr = rnsClient.getEPR(path, true);
		return RNSUtil.toXMLString(epr);
	}

	private String getURLString(String path) throws RNSError {
		EndpointReferenceType epr = rnsClient.getEPR(path, true);
		return epr.getAddress().getValue().toString();
	}

	private String getXMLString(String path) throws TransformerException,
			Exception {
		MessageElement[] mes = rnsClient.getMetadata(path);
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		if (mes != null) {
			boolean plural = false;
			for (MessageElement me : mes) {
				if (plural) {
					pw.println("----");
				}
				pw.print(RNSUtil.toIndentedXML(me));
				plural = true;
			}
		}
		pw.flush();
		return sw.toString();
	}

	private static class GetXattrResponse {
		int error;
		byte[] value;

		public GetXattrResponse(int error, byte[] value) {
			this.error = error;
			this.value = value;
		}
	}

	// TEST: ACL
	//	private void int2byte(byte[] b, int pos, int val) {
	//		b[pos] = (byte) (val >>> 0);
	//		b[pos+1] = (byte) (val >>> 8);
	//		b[pos+2] = (byte) (val >>> 16);
	//		b[pos+3] = (byte) (val >>> 24);
	//	}
	//
	//	private void short2byte(byte[] b, int pos, short val) {
	//		b[pos] = (byte) (val >>> 0);
	//		b[pos+1] = (byte) (val >>> 8);
	//	}

	private GetXattrResponse getxattr_common(String path, String name)
			throws FuseException, BufferOverflowException {
		try {
			String str = null;
			if ("rns.epr".equals(name)) {
				str = getEPRString(path);
			} else if ("rns.url".equals(name)) {
				str = getURLString(path);
			} else if ("rns.xml".equals(name)) {
				str = getXMLString(path);
			} else if (name.startsWith("rnskv.")) {
				String key = name.substring(6);
				RNSKeyValue kv = new RNSKeyValue(home, path);
				str = kv.get(key);
				/* ENOATTR if str is null*/

				//			} else if ("system.posix_acl_access".equals(name)) {
				//				// TEST: system.posix_acl_access
				//				byte[] b = new byte[4+ (2+2+4)*4];
				//				int version = 0x0002;
				//				short e_tag = 0x02;
				//				short e_perm = 0x07;
				//				int e_id = 5000;
				//				int2byte(b, 0, version);
				//
				//				short2byte(b, 4, e_tag);
				//				short2byte(b, 6, e_perm);
				//				int2byte(b, 8, e_id);
				//
				//				short2byte(b, 12, (short)0x10);
				//				short2byte(b, 14, (short)0x02);
				//				int2byte(b, 16, 12345);
				//
				//				short2byte(b, 20, (short)0x04);
				//				short2byte(b, 22, (short)0x04);
				//				int2byte(b, 24, 22345);
				//
				//				short2byte(b, 28, (short)0x08);
				//				short2byte(b, 30, (short)0x06);
				//				int2byte(b, 32, 32345);
				//
				//				return new GetXattrResponse(0, b);
				//			} else if ("system.posix_acl_default".equals(name)) {
				//				// TEST: support system.posix_acl_default
				//				byte[] b = new byte[4+2+2+4];
				//				int version = 0x0002;
				//				short e_tag = 0x01;
				//				short e_perm = 0x07;
				//				int e_id = 5000;
				//				int2byte(b, 0, version);
				//				short2byte(b, 4, e_tag);
				//				short2byte(b, 6, e_perm);
				//				int2byte(b, 8, e_id);
				//				return new GetXattrResponse(0, b);
			}
			if (str == null) {
				return new GetXattrResponse(Errno.ENOATTR, null);
			}
			return new GetXattrResponse(0, str.getBytes());
		} catch (SerializationException e) {
			e.printStackTrace();
			return new GetXattrResponse(Errno.EBADMSG, null);
		} catch (RNSError e) {
			return new GetXattrResponse(rnsErrorToErrno(e), null);
		} catch (TransformerException e) {
			e.printStackTrace();
			return new GetXattrResponse(Errno.EBADMSG, null);
		} catch (Exception e) {
			e.printStackTrace();
			return new GetXattrResponse(Errno.EBADMSG, null);
		}
	}

	private Map<String, Map<String, GetXattrResponse>> xattrCache = null;
	private Map<String, List<String>> xattrListCache = null;

	private void clearXattrCache(String path) {
		if (xattrCache != null) {
			xattrCache.remove(path);
		}
		if (xattrListCache != null) {
			xattrListCache.remove(path);
		}
	}

	private GetXattrResponse getxattr_common_cached(String path, String name)
			throws FuseException, BufferOverflowException {
		if (xattrCache == null) {
			xattrCache = new TimeoutCacheMap<String, Map<String, GetXattrResponse>>(
					home.getCacheTimeout());
		}
		GetXattrResponse res;
		Map<String, GetXattrResponse> keyValue = xattrCache.get(path);
		if (keyValue == null) {
			keyValue = new TimeoutCacheMap<String, GetXattrResponse>(
					home.getCacheTimeout());
			xattrCache.put(path, keyValue);
		} else {
			res = keyValue.get(name);
			if (res != null) {
				return res;
			}
		}
		res = getxattr_common(path, name);
		keyValue.put(name, res);
		return res;
	}

	public int getxattr(String path, String name, ByteBuffer dst, int position)
			throws FuseException, BufferOverflowException {
		debug("getxattr", "name=" + name);
		GetXattrResponse res = getxattr_common_cached(path, name);
		if (res.error == 0) {
			dst.put(res.value);
			return 0;
		} else {
			return res.error;
		}
	}

	public int getxattrsize(String path, String name, FuseSizeSetter sizeSetter)
			throws FuseException {
		debug("getxattrsize", "name=" + name);
		GetXattrResponse res = getxattr_common_cached(path, name);
		if (res.error == 0) {
			sizeSetter.setSize(res.value.length);
			return 0;
		} else {
			return res.error;
		}
	}

	private List<String> listxattr_cached(String path) throws RNSError {
		if (xattrListCache == null) {
			xattrListCache = new TimeoutCacheMap<String, List<String>>(
					home.getCacheTimeout());
		}
		List<String> list = xattrListCache.get(path);
		if (list != null) {
			return list;
		}
		list = new ArrayList<String>();
		RNSKeyValue kv = new RNSKeyValue(home, path);
		for (String name : kv.keySet()) {
			list.add(name);
		}
		xattrListCache.put(path, list);
		return list;
	}

	public int listxattr(String path, XattrLister lister) throws FuseException {
		debug("listxattr", "");

		lister.add("rns.epr");
		lister.add("rns.url");
		lister.add("rns.xml");

		// NOTE : support ACL
		// lister.add("system.posix_acl_default");
		// lister.add("system.posix_acl_access");

		try {
			List<String> list = listxattr_cached(path);
			for (String name : list) {
				lister.add("rnskv." + name);
			}
			return 0;
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
	}

	public int setxattr(String path, String name, ByteBuffer value, int flags,
			int position) throws FuseException {
		debug("setxattr", "name=" + name);

		try {
			if ("rns.xml".equals(name)) {
				byte b[] = new byte[value.limit()];
				value.get(b);
				String str = new String(b);
				debug("setxattr", str);
				try {
					rnsClient.setMetadata(path,
							RNSUtil.toMessageElements(str));
					// RNSUtil.setXMLFromString(rnsClient, path, str);
					clearXattrCache(path);
					return 0;
				} catch (RNSError e) {
					return rnsErrorToErrno(e);
				} catch (Exception e) {
					e.printStackTrace();
					return Errno.EBADMSG;
				}
			} else if (name != null && name.startsWith("rnskv.")) {
				byte b[] = new byte[value.limit()];
				value.get(b);
				String val = new String(b);
				String key = name.substring(6);
				RNSKeyValue kv = new RNSKeyValue(home, path);
				try {
					kv.put(key, val);
					clearXattrCache(path);
					return 0;
				} catch (RNSError e) {
					return rnsErrorToErrno(e);
				}
			} else {
				return Errno.ENOSYS;
			}
		} catch (Throwable t) {
			t.printStackTrace();
			return Errno.EBADMSG;
		}
	}

	public int removexattr(String path, String name) throws FuseException {
		debug("removexattr", "name=" + name);

		if (name != null && name.startsWith("rnskv.")) {
			String key = name.substring(6);
			RNSKeyValue kv = new RNSKeyValue(home, path);
			try {
				kv.remove(key);
				clearXattrCache(path);
				return 0;
			} catch (RNSError e) {
				return rnsErrorToErrno(e);
			}
		} else {
			return Errno.ENOSYS;
		}
	}

	public static void main(String[] args) {
		try {
			FuseMount.mount(args, new RNSFS_FUSE(), log);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
