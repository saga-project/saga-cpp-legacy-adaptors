package org.naregi.rns.client.fuse;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.nio.BufferOverflowException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.util.Random;

import javax.xml.transform.TransformerException;

import org.apache.axis.message.MessageElement;
import org.apache.axis.types.URI.MalformedURIException;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.globus.axis.message.addressing.Address;
import org.globus.axis.message.addressing.EndpointReferenceType;
import org.globus.wsrf.encoding.DeserializationException;
import org.globus.wsrf.encoding.SerializationException;
import org.globus.wsrf.utils.AnyHelper;
import org.naregi.rns.ACL;
import org.naregi.rns.RNSUtil;
import org.naregi.rns.client.RNSDirHandle;
import org.naregi.rns.client.RNSACLClient;
import org.naregi.rns.client.RNSClient;
import org.naregi.rns.client.RNSClientHome;
import org.naregi.rns.client.RNSClientUtil;
import org.naregi.rns.client.RNSDirent;
import org.naregi.rns.client.RNSError;
import org.naregi.rns.client.RNSStat;

import sun.misc.Signal;
import sun.misc.SignalHandler;
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

public class RNSFilesystem implements Filesystem3, XattrSupport {
	private static final Log log = LogFactory.getLog(RNSFilesystem.class);

	private RNSClient rnsClient;
	private RNSACLClient aclClient;

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

	private static class SigintHandler implements SignalHandler {
		public void handle(Signal signal) {
			System.out.println("ignore SIGINT");
		}
	}

	public RNSFilesystem() throws Exception {
		RNSClientHome home = new RNSClientHome();
		String[] args2 = {};
		home.initialize(args2, 0, 0);
		rnsClient = home.getRNSClient();
		if (rnsClient == null) {
			return;
		}
		aclClient = new RNSACLClient(rnsClient, home);

		Signal sig = new Signal("INT");
		SignalHandler hand = new SigintHandler();
		Signal.handle(sig, hand);
	}

	public int chmod(String path, int mode) throws FuseException {
		debug("chmod", path);
		String ownerPerm = ACL.permToString((short) ((mode & 0700) >> 6));
		String ownerGroupPerm = ACL.permToString((short) ((mode & 070) >> 3));
		String otherPerm = ACL.permToString((short) (mode & 07));

		String aclSpec = "ou::" + ownerPerm + ",og::" + ownerGroupPerm + ",o:" + otherPerm;
		try {
			aclClient.setACL(path, aclSpec);
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

	private String forCreateTmpPath = null;

	public int getattr(String path, FuseGetattrSetter getattrSetter)
			throws FuseException {
		debug("getattr", "path=" + path);
		if (path.equals(forCreateTmpPath)) {
			int time = (int) (System.currentTimeMillis() / 1000L);
			getattrSetter.set(path.hashCode(),
					FuseFtypeConstants.TYPE_FILE | 0600, 1, 0, 0, 0, 0, 10,
					time, time, time);
			return 0;
		}
		forCreateTmpPath = null;
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
			String eprstr;
			try {
				eprstr = RNSUtil.toXMLStringFromEPR(epr);
			} catch (SerializationException e) {
				e.printStackTrace();
				return Errno.EBADMSG;
			}
			if (isdir) {
				RNSStat st = rnsClient.stat(path);
				int atime = (int) (st.getAccessTime().getTimeInMillis() / 1000L);
				int mtime = (int) (st.getModificationTime().getTimeInMillis() / 1000L);
				int ctime = (int) (st.getCreateTime().getTimeInMillis() / 1000L);
				int nlink = st.getElementCount().intValue() + 2;
				// TODO dir size
				int uid;
				int gid;
				int mode;
				try {
					ACL acl = aclClient.getACL(path, true);
					mode = ((ACL.PERM_ALL & acl.getOwnerPerm()) << 6)
							+ ((ACL.PERM_ALL & acl.getOwnerGroupPerm()) << 3)
							+ (ACL.PERM_ALL & acl.getOtherPerm());
					uid = acl.getOwner().hashCode(); // TODO uid
					gid = acl.getOwnerGroup().hashCode(); // TODO gid
				} catch (Exception e) {
					mode = 0;
					uid = 0;
					gid = 0;
				}
				getattrSetter.set(eprstr.hashCode(),
						FuseFtypeConstants.TYPE_DIR | mode, nlink, uid, gid, 0, 4096,
						4, atime, mtime, ctime);
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
					size = eprstr.getBytes().length;
				}
				int time = (int) (System.currentTimeMillis() / 1000L);
				getattrSetter.set(eprstr.hashCode(),
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
			Random rnd = new Random();
			filler.add(".", rnd.nextLong(), FuseFtypeConstants.TYPE_DIR);
			filler.add("..", rnd.nextLong(), FuseFtypeConstants.TYPE_DIR);

			DirHandle dir = rnsClient.list(path, false);
			if (dir == null) {
				return 0;
			}
			while (dir.hasNext()) {
				RNSDirent ent = dir.next();
				EndpointReferenceType epr = ent.getEpr();
				int type = 0;
				boolean isdir = RNSClientUtil.isDirectory(ent.getMeta());
				if (isdir) {
					type = FuseFtypeConstants.TYPE_DIR;
				} else {
					type = FuseFtypeConstants.TYPE_FILE;
				}
				if (type > 0) {
					filler.add(ent.getName(), epr.hashCode(), type);
				}
			}
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int link(String from, String to) throws FuseException {
		try {
			rnsClient.copyEPR(from, to);
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
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		}
		return 0;
	}

	public int rmdir(String path) throws FuseException {
		try {
			rnsClient.rmdir(path);
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
			rnsClient.rmEPR(path, false);
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
			forCreateTmpPath = path;
			return 0;
		} else {
			return Errno.EPERM;
		}
	}

	public static final int O_ACCMODE = 03;

	public int open(String path, int flags, FuseOpenSetter openSetter)
			throws FuseException {
		try {
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
						// unexpected
						debug("open", "EINVAL 1");
						return Errno.EINVAL;
					}
					path = tmpPath;
				}
			}
			String tmp1 = forCreateTmpPath;
			if (tmp1 != null) {
				String tmp2 = cutRnsSuffix(tmp1);
				if (tmp2 != null) {
					tmp1 = tmp2;
				}
				forCreateTmpPath = null;
			}
			if (path.equals(tmp1)) { // new entry
				if (fh.mode.equals(MODE_XML)) {
					debug("open", "EINVAL 2");
					return Errno.EINVAL;
				} else if ((flags & O_ACCMODE) == FuseFS.O_RDONLY) {
					debug("open", "EINVAL 3");
					return Errno.EINVAL;
				}
				// do nothing
			} else { // existing entry
				if ((flags & O_ACCMODE) != FuseFS.O_RDONLY) {
					if (fh.mode.equals(MODE_XML) == false) {
						debug("open", "EINVAL 4");
						return Errno.EINVAL;
					}
				}
				if (fh.mode.equals(MODE_EPR)) {
					EndpointReferenceType epr = rnsClient.getEPR(path, true);
					String str = RNSUtil.toXMLStringFromEPR(epr);
					fh.current = str.getBytes();
				} else if (fh.mode.equals(MODE_URL)) {
					EndpointReferenceType epr = rnsClient.getEPR(path, true);
					String str = epr.getAddress().getValue().toString();
					fh.current = str.getBytes();
				} else if (fh.mode.equals(MODE_XML)) {
					MessageElement[] mes = rnsClient.getXML(path);
					StringWriter sw = new StringWriter();
					PrintWriter pw = new PrintWriter(sw);
					if (mes != null) {
						boolean plural = false;
						for (MessageElement me : mes) {
							if (plural) {
								pw.println("----");
							}
							pw.println(RNSUtil.filterXMLString(RNSUtil
									.transformXML(AnyHelper.toElement(me))));
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
			fh.path = path;
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
				+ isWritepage); //
		if (fh instanceof FH == false) {
			return Errno.ENOSYS;
		}
		FH f = (FH) fh;
		if (f.baos == null) {
			f.baos = new ByteArrayOutputStream();
		}
		try {
			if (f.baos.size() != offset) {
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
					EndpointReferenceType epr = RNSUtil
							.getEPRFromXMLString(str);
					rnsClient.addEPR(f.path, epr);
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
					rnsClient.addEPR(f.path, epr);
					return 0;
				} catch (MalformedURIException e) {
					e.printStackTrace();
					return Errno.EINVAL;
				} catch (RNSError e) {
					return rnsErrorToErrno(e);
				}
			} else if (f.mode.equals(MODE_XML)) {
				try {
					rnsClient.setXML(f.path, RNSClientUtil.convertStringToMessageElements(str));
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
		return RNSUtil.toXMLStringFromEPR(epr);
	}

	private String getURLString(String path) throws RNSError {
		EndpointReferenceType epr = rnsClient.getEPR(path, true);
		return epr.getAddress().getValue().toString();
	}

	private String getXMLString(String path) throws TransformerException,
			Exception {
		MessageElement[] mes = rnsClient.getXML(path);
		StringWriter sw = new StringWriter();
		PrintWriter pw = new PrintWriter(sw);
		if (mes != null) {
			boolean plural = false;
			for (MessageElement me : mes) {
				if (plural) {
					pw.println("----");
				}
				pw.println(RNSUtil.filterXMLString(RNSUtil
						.transformXML(AnyHelper.toElement(me))));
				plural = true;
			}
		}
		pw.flush();
		return sw.toString();
	}

	// TODO support system.posix_acl_access ?
	// TODO support system.posix_acl_default ?

	public int getxattr(String path, String name, ByteBuffer dst)
			throws FuseException, BufferOverflowException {
		debug("getxattr", "name=" + name);
		try {
			String str;
			if ("epr".equals(name)) {
				str = getEPRString(path);
			} else if ("url".equals(name)) {
				str = getURLString(path);
			} else if ("xml".equals(name)) {
				str = getXMLString(path);
			} else {
				return Errno.ENOATTR;
			}
			dst.put(str.getBytes());
			return 0;
		} catch (SerializationException e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		} catch (TransformerException e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		} catch (Exception e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		}
	}

	public int getxattrsize(String path, String name, FuseSizeSetter sizeSetter)
			throws FuseException {
		debug("getxattrsize", "name=" + name);
		try {
			String str;
			if ("epr".equals(name)) {
				str = getEPRString(path);
			} else if ("url".equals(name)) {
				str = getURLString(path);
			} else if ("xml".equals(name)) {
				str = getXMLString(path);
			} else {
				return Errno.ENOATTR;
			}
			sizeSetter.setSize(str.getBytes().length);
			return 0;
		} catch (SerializationException e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		} catch (RNSError e) {
			return rnsErrorToErrno(e);
		} catch (TransformerException e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		} catch (Exception e) {
			e.printStackTrace();
			return Errno.EBADMSG;
		}
	}

	public int listxattr(String path, XattrLister lister) throws FuseException {
		debug("listxattr", "");

		lister.add("epr");
		lister.add("url");
		lister.add("xml");
		return 0;
	}

	public int setxattr(String path, String name, ByteBuffer value, int flags)
			throws FuseException {
		debug("setxattr", "name=" + name);

		if ("xml".equals(name)) {
			byte b[] = new byte[value.limit()];
			value.get(b);
			String str = new String(b);
			System.out.println(str);
			try {
				rnsClient.setXML(path, RNSClientUtil.convertStringToMessageElements(str));
//				RNSClientUtil.setXMLFromString(rnsClient, path, str);
				return 0;
			} catch (RNSError e) {
				return rnsErrorToErrno(e);
			} catch (Exception e) {
				e.printStackTrace();
				return Errno.EBADMSG;
			}
		} else {
			return Errno.ENOSYS;
		}
	}

	public int removexattr(String path, String name) throws FuseException {
		debug("removexattr", "name=" + name);

		return Errno.ENOSYS;
	}

	public static void main(String[] args) {
		try {
			FuseMount.mount(args, new RNSFilesystem(), log);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
