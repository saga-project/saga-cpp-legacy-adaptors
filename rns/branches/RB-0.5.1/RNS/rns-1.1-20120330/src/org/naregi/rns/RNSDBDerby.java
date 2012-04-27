/*
 * Copyright (C) 2008-2012 Osaka University.
 * Copyright (C) 2008-2012 National Institute of Informatics in Japan.
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
package org.naregi.rns;

import java.io.IOException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;

import javax.sql.rowset.serial.SerialClob;

import org.apache.axis.message.MessageElement;
import org.apache.commons.collections.map.ReferenceMap;
import org.apache.commons.logging.Log;
import org.naregi.rns.stubs.ACLEntryType;
import org.naregi.rns.util.RNSUtil;
import org.naregi.rns.util.TempOneFileStringList;

/**
 * An implementation to access Apache Derby database.
 */
public class RNSDBDerby implements RNSDB {
	private static Connection conn = null; /* shared by all resources */
	private static final String dbname = "rns";
	private static final String dburl = "jdbc:derby:" + dbname;
	private static Log logger;

	private static Object globalLock = new Object();

	private long id = -1;

	private static final long ROOTID = 0;

	/* --- common static methods --- */
	private synchronized static void dbConnect() throws Exception {
		if (conn != null) {
			return;
		}
		logger = RNSLog.getLog();

		/* NOTE: Cannot use derby.system.home, because GT4 derby setting is preferred. */

		// String systemDirs = RNSConfig.getStorageDir() + FILE_SEP + "derby";
		// System.setProperty("derby.system.home", systemDirs);
		// File systemDir = new File(systemDirs);
		// if (systemDir.exists() == false && systemDir.mkdir() == false) {
		// logger.warn("RNSDBDerby:cannot create:"
		// + systemDir.getAbsolutePath());
		// return;
		// }

		/* store the database into GLOBUS_LOCATION/var/rns */
		String derbyhome = System.getProperty("derby.system.home");
		logger.info("derby.system.home=" + derbyhome);

		try {
			Class.forName("org.apache.derby.jdbc.EmbeddedDriver");
		} catch (ClassNotFoundException e) {
			logger.warn("RNSDBDerby:" + e.getMessage());
			throw e;
		}

		try {
			conn = DriverManager.getConnection(dburl + ";create=false");
			conn.setAutoCommit(false);
			if (dbExistID(ROOTID)) {
				return;
			}
			/* else -> through */
		} catch (SQLException e) {
			//e.printStackTrace();
			logger.info("RNSDBDerby: " + e.getMessage());
			/* through */
		}

		Statement st = null;
		try {
			conn = DriverManager.getConnection(dburl + ";create=true");
			conn.setAutoCommit(false);
			st = conn.createStatement();
			st.execute("CREATE TABLE DirProp ("
					+ "dirid BIGINT NOT NULL PRIMARY KEY GENERATED ALWAYS AS IDENTITY (START WITH 0, INCREMENT BY 1),"
					+ "accessTime TIMESTAMP NOT NULL,"
					+ "createTime TIMESTAMP NOT NULL,"
					+ "modificationTime TIMESTAMP NOT NULL" + ")");
			st.execute("CREATE TABLE DirList ("
					+ "entid BIGINT NOT NULL PRIMARY KEY GENERATED ALWAYS AS IDENTITY (START WITH 0, INCREMENT BY 1),"
					+ "dirid BIGINT NOT NULL REFERENCES DirProp(dirid) ON DELETE CASCADE,"
					+ "name  VARCHAR(1024) NOT NULL,"
					+ "isdir SMALLINT NOT NULL," + "localid VARCHAR(64),"
					+ "epr XML," + "UNIQUE (dirid, name)" + ")");
			st.execute("CREATE INDEX indexDirList ON DirList(dirid)");
			st.execute("CREATE INDEX indexDirList2 ON DirList(dirid, name)");
			st.execute("CREATE TABLE Metadata ("
					+ "dirid BIGINT NOT NULL REFERENCES DirProp(dirid) ON DELETE CASCADE,"
					+ "entid BIGINT NOT NULL REFERENCES DirList(entid) ON DELETE CASCADE,"
					+ "data XML NOT NULL" + ")");
			st.execute("CREATE INDEX indexMetadata ON Metadata(dirid, entid)");
			st.execute("CREATE TABLE Acl ("
					+ "dirid BIGINT NOT NULL REFERENCES DirProp(dirid) ON DELETE CASCADE,"
					+ "type SMALLINT NOT NULL," + "nameid INT NOT NULL,"
					+ "perm SMALLINT NOT NULL,"
					+ "UNIQUE (dirid, type, nameid)" + ")");
			st.execute("CREATE TABLE Names ("
					+ "nameid INT NOT NULL PRIMARY KEY GENERATED ALWAYS AS IDENTITY (START WITH 0, INCREMENT BY 1),"
					+ "name VARCHAR(128) NOT NULL," + "UNIQUE (nameid, name)"
					+ ")");
			st.execute("CREATE INDEX indexNames ON Names(name)");

			dbCreateNewDir(); /* create ROOTID */
			ACL acl = new ACL();
			acl.setOwner("admin", ACL.PERM_ALL);
			acl.setOwnerGroup("admin", ACL.PERM_READ);
			acl.setOther(ACL.PERM_READ);
			dbSetACL(ROOTID, acl.toACLEntries());
			if (dbExistID(ROOTID) == false) {
				logger.fatal("cannot create root directory.");
				dbDisconnect();
				conn = null;
				throw new Exception("cannot create root directory.");
			}
			logger.info("create new table (" + derbyhome + "/rns)");
			conn.commit();
		} catch (SQLException e) {
			conn.rollback();
			e.printStackTrace();
			logger.warn("RNSDBDerby:" + e.getMessage());
			dbDisconnect();
			conn = null;
			throw e;
		} finally {
			try {
				if (st != null) {
					st.close();
				}
			} catch (SQLException e) {
			}
		}
	}

	private synchronized static void dbDisconnect() throws SQLException {
		if (conn == null) {
			return;
		}
		try {
			conn = DriverManager.getConnection(dburl
					+ ";create=false;shutdown=true");
		} catch (SQLException e) {
			conn = null;
			throw e;
		}
		conn = null;
	}

	private static PreparedStatement psExistID = null;

	private synchronized static boolean dbExistID(long id) throws SQLException {
		ResultSet rs = null;
		try {
			if (psExistID == null) {
				String sql = "SELECT dirid FROM DirProp WHERE dirid = ?";
				psExistID = conn.prepareStatement(sql);
			}
			psExistID.setLong(1, id);
			rs = psExistID.executeQuery();
			if (rs.next()) {
				return true;
			}
		} catch (SQLException e) {
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
		return false;
	}

	private static PreparedStatement psGetDirProps = null;

	private synchronized static RNSDirectoryProperties dbGetDirProps(long id)
			throws SQLException {
		ResultSet rs = null;
		try {
			if (psGetDirProps == null) {
				String sql = "SELECT accessTime,createTime,modificationTime FROM DirProp WHERE dirid = ?";
				psGetDirProps = conn.prepareStatement(sql);
			}
			psGetDirProps.setLong(1, id);
			rs = psGetDirProps.executeQuery();
			RNSDirectoryProperties newprops = new RNSDirectoryProperties();
			if (rs.next()) {
				Calendar at = Calendar.getInstance();
				at.setTimeInMillis(rs.getTimestamp(1).getTime());
				Calendar ct = Calendar.getInstance();
				ct.setTimeInMillis(rs.getTimestamp(2).getTime());
				Calendar mt = Calendar.getInstance();
				mt.setTimeInMillis(rs.getTimestamp(3).getTime());
				newprops.setAccessTime(at);
				newprops.setCreateTime(ct);
				newprops.setModificationTime(mt);
				return newprops;
			}
			return null;
		} catch (SQLException e) {
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psCreateNewDir = null;
	private static PreparedStatement psCreateNewDir_id = null;

	private synchronized static long dbCreateNewDir() throws SQLException {
		ResultSet rs = null;
		try {
			if (psCreateNewDir == null) {
				String sql = "INSERT INTO DirProp (accessTime,createTime,modificationTime) VALUES (?,?,?)";
				psCreateNewDir = conn.prepareStatement(sql);
			}
			psCreateNewDir.setTimestamp(1,
					new Timestamp(System.currentTimeMillis()));
			psCreateNewDir.setTimestamp(2,
					new Timestamp(System.currentTimeMillis()));
			psCreateNewDir.setTimestamp(3,
					new Timestamp(System.currentTimeMillis()));
			psCreateNewDir.execute();

			if (psCreateNewDir_id == null) {
				String sql = "VALUES IDENTITY_VAL_LOCAL()";
				psCreateNewDir_id = conn.prepareStatement(sql);
			}
			rs = psCreateNewDir_id.executeQuery();
			if (rs.next()) {
				return rs.getLong(1);
			}
			return -1;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbCreateNewDir:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psUpdateDirProps = null;

	private synchronized static void dbUpdateDirProps(long dirid,
			RNSDirectoryProperties props) throws SQLException {
		try {
			if (psUpdateDirProps == null) {
				String sql = "UPDATE DirProp SET accessTime = ?, createTime = ?, modificationTime = ? WHERE dirid = ?";
				psUpdateDirProps = conn.prepareStatement(sql);
			}
			psUpdateDirProps.setTimestamp(1, new Timestamp(
					props.getAccessTime().getTimeInMillis()));
			psUpdateDirProps.setTimestamp(2, new Timestamp(
					props.getCreateTime().getTimeInMillis()));
			psUpdateDirProps.setTimestamp(3, new Timestamp(
					props.getModificationTime().getTimeInMillis()));
			psUpdateDirProps.setLong(4, dirid);
			psUpdateDirProps.execute();
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbUpdateDirProps:" + e.getMessage());
			throw e;
		}
	}

	private synchronized static void dbSetTime(long dirid, Calendar t,
			String propName) throws SQLException {
		PreparedStatement ps = null;
		try {
			String sql = "UPDATE DirProp SET " + propName
					+ " = ? WHERE dirid = ?";
			ps = conn.prepareStatement(sql);
			ps.setTimestamp(1, new Timestamp(t.getTimeInMillis()));
			ps.setLong(2, dirid);
			ps.execute();
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbSetTime:" + e.getMessage());
			throw e;
		} finally {
			if (ps != null) {
				try {
					ps.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psGetListSize = null;

	private synchronized static long dbGetListSize(long id) throws SQLException {
		ResultSet rs = null;
		try {
			if (psGetListSize == null) {
				String sql = "SELECT COUNT(name) FROM DirList WHERE dirid = ?";
				psGetListSize = conn.prepareStatement(sql);
			}
			psGetListSize.setLong(1, id);
			rs = psGetListSize.executeQuery();
			if (rs.next()) {
				return rs.getLong(1);
			}
			return 0;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbGetListSize:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psGetList = null;

	private synchronized static List<String> dbGetList(long id)
			throws SQLException, IOException {
		ResultSet rs = null;
		try {
			if (psGetList == null) {
				String sql = "SELECT name FROM DirList WHERE dirid = ?";
				psGetList = conn.prepareStatement(sql);
			}
			psGetList.setLong(1, id);
			rs = psGetList.executeQuery();

			List<String> l = new TempOneFileStringList(RNSConfig.tmpdir(),
					"RNSLST");
			//			List<String> l = new ArrayList<String>();
			while (rs.next()) {
				l.add(rs.getString(1));
			}
			return l;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbGetList:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement ps1GetEntryData = null;
	private static PreparedStatement ps2GetEntryData = null;

	private synchronized static RNSEntryData dbGetEntryData(long dirid,
			String name) throws Exception {
		ResultSet rs = null;
		try {
			if (ps1GetEntryData == null) {
				String sql = "SELECT isdir,localid,XMLSERIALIZE (epr AS CLOB),entid FROM DirList WHERE dirid = ? AND name = ?";
				ps1GetEntryData = conn.prepareStatement(sql);
			}
			ps1GetEntryData.setLong(1, dirid);
			ps1GetEntryData.setString(2, name);
			rs = ps1GetEntryData.executeQuery();
			RNSEntryData ed = new RNSEntryData();
			long entid;
			if (rs.next()) {
				ed.setDirectory(rs.getShort(1) != 0);
				ed.setLocalID(rs.getString(2));
				String eprStr = rs.getString(3);
				if (eprStr != null) {
					ed.setEpr(RNSUtil.toEPR(eprStr));
				}
				entid = rs.getLong(4);
			} else {
				return null;
			}
			rs.close();
			rs = null;

			ArrayList<MessageElement> al = new ArrayList<MessageElement>();
			if (ps2GetEntryData == null) {
				String sql2 = "SELECT XMLSERIALIZE (data AS CLOB) FROM Metadata WHERE dirid = ? AND entid = ?";
				ps2GetEntryData = conn.prepareStatement(sql2);
			}
			ps2GetEntryData.setLong(1, dirid);
			ps2GetEntryData.setLong(2, entid);
			rs = ps2GetEntryData.executeQuery();
			while (rs.next()) { /* more than one */
				al.add(RNSUtil.toMessageElement(rs.getString(1)));
			}
			ed.setAny(al.toArray(new MessageElement[0]));
			return ed;
		} catch (Exception e) {
			logger.error("RNSDBDerby:dbGetEntryData:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psDeleteDir = null;

	private synchronized static void dbDeleteDir(long dirid)
			throws SQLException {
		try {
			if (psDeleteDir == null) {
				String sql = "DELETE FROM DirProp WHERE dirid = ?";
				psDeleteDir = conn.prepareStatement(sql);
			}
			psDeleteDir.setLong(1, dirid);
			psDeleteDir.execute();
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbDeleteDir:" + e.getMessage());
			throw e;
		}
	}

	private static PreparedStatement psDeleteEntry = null;

	private synchronized static void dbDeleteEntry(long dirid, String name)
			throws SQLException {
		try {
			if (psDeleteEntry == null) {
				String sql = "DELETE FROM DirList WHERE dirid = ? AND name = ?";
				psDeleteEntry = conn.prepareStatement(sql);
			}
			psDeleteEntry.setLong(1, dirid);
			psDeleteEntry.setString(2, name);
			psDeleteEntry.execute();
			return;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbDeleteEntry:" + e.getMessage());
			throw e;
		}
	}

	private static PreparedStatement entpsInsertEntry = null;
	private static PreparedStatement getidpsInsertEntry = null;
	private static PreparedStatement metapsInsertEntry = null;

	private synchronized static void dbInsertEntry(long dirid, String name,
			RNSEntryData ed) throws Exception {
		ResultSet rs = null;
		try {
			if (entpsInsertEntry == null) {
				String entsql = "INSERT INTO DirList (dirid,name,isdir,localid,epr) VALUES (?,?,?,?,XMLPARSE (DOCUMENT CAST (? AS CLOB) PRESERVE WHITESPACE))";
				entpsInsertEntry = conn.prepareStatement(entsql);
			}
			entpsInsertEntry.setLong(1, dirid);
			entpsInsertEntry.setString(2, name);
			entpsInsertEntry.setShort(3, (short) (ed.isDirectory() ? 1 : 0));
			entpsInsertEntry.setString(4, ed.getLocalID());
			if (ed.getEpr() != null) {
				String eprXML = RNSUtil.toXMLString(ed.getEpr());
				if (eprXML.length() > RNSConfig.limitMetadataSize()) {
					throw new Exception("exceed the limit of EPR size: "
							+ eprXML.length());
				}
				SerialClob clob = new SerialClob(eprXML.toCharArray()); /* copy */
				eprXML = null;
				entpsInsertEntry.setClob(5, clob);
			} else {
				entpsInsertEntry.setNull(5, java.sql.Types.CLOB);
			}
			entpsInsertEntry.execute();

			if (ed.getRNSMetadata() != null
					&& ed.getRNSMetadata().get_any() != null) {
				if (getidpsInsertEntry == null) {
					String getidsql = "VALUES IDENTITY_VAL_LOCAL()";
					getidpsInsertEntry = conn.prepareStatement(getidsql);
				}
				long entid;
				rs = getidpsInsertEntry.executeQuery();
				if (rs.next()) {
					entid = rs.getLong(1);
				} else {
					logger.fatal("RNSDBDerby:dbInsertEntry:IDENTITY_VAL_LOCAL failed");
					throw new Exception("IDENTITY_VAL_LOCAL failed");
				}
				rs.close();
				rs = null;

				if (metapsInsertEntry == null) {
					String metasql = "INSERT INTO Metadata (dirid,entid,data) VALUES (?,?,XMLPARSE (DOCUMENT CAST (? AS CLOB) PRESERVE WHITESPACE))";
					metapsInsertEntry = conn.prepareStatement(metasql);
				}
				for (MessageElement me : ed.getRNSMetadata().get_any()) {
					metapsInsertEntry.setLong(1, dirid);
					metapsInsertEntry.setLong(2, entid);

					String xml = me.getAsString();
					if (xml.length() > RNSConfig.limitMetadataSize()) {
						throw new Exception(
								"exceed the limit of metadata size: "
										+ xml.length());
					}
					SerialClob clob = new SerialClob(xml.toCharArray()); /* copy */
					xml = null;
					metapsInsertEntry.setClob(3, clob);

					metapsInsertEntry.execute();
				}
			}
		} catch (Exception e) {
			logger.error("RNSDBDerby:setList:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psRename = null;

	private synchronized static void dbRename(long dirid, String from, String to)
			throws Exception {
		try {
			if (psRename == null) {
				String sql = "UPDATE DirList SET name = ? WHERE dirid = ? AND name = ?";
				psRename = conn.prepareStatement(sql);
			}
			psRename.setString(1, to);
			psRename.setLong(2, dirid);
			psRename.setString(3, from);
			psRename.execute();
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbRename:" + e.getMessage());
			throw e;
		}
	}

	private static PreparedStatement entidpsReplaceMetadata = null;
	private static PreparedStatement delmetapsReplaceMetadata = null;
	private static PreparedStatement metapsReplaceMetadata = null;

	private synchronized static void dbReplaceMetadata(long dirid, String name,
			MessageElement[] mes) throws Exception {
		ResultSet rs = null;
		try {
			if (entidpsReplaceMetadata == null) {
				String entidsql = "SELECT entid FROM DirList WHERE dirid = ? AND name = ?";
				entidpsReplaceMetadata = conn.prepareStatement(entidsql);
			}
			entidpsReplaceMetadata.setLong(1, dirid);
			entidpsReplaceMetadata.setString(2, name);
			rs = entidpsReplaceMetadata.executeQuery();
			long entid;
			if (rs.next()) {
				entid = rs.getLong(1);
			} else {
				logger.fatal("RNSDBDerby:dbReplaceMetadata:");
				throw new Exception("");
			}
			rs.close();
			rs = null;

			if (delmetapsReplaceMetadata == null) {
				String delmetasql = "DELETE FROM Metadata WHERE dirid = ? AND entid = ?";
				delmetapsReplaceMetadata = conn.prepareStatement(delmetasql);
			}
			delmetapsReplaceMetadata.setLong(1, dirid);
			delmetapsReplaceMetadata.setLong(2, entid);
			delmetapsReplaceMetadata.execute();

			if (mes == null || mes.length == 0) {
				return; /* end */
			}
			if (metapsReplaceMetadata == null) {
				String metasql = "INSERT INTO Metadata (dirid,entid,data) VALUES (?,?,XMLPARSE (DOCUMENT CAST (? AS CLOB) PRESERVE WHITESPACE))";
				metapsReplaceMetadata = conn.prepareStatement(metasql);
			}
			for (MessageElement me : mes) {
				metapsReplaceMetadata.setLong(1, dirid);
				metapsReplaceMetadata.setLong(2, entid);

				String xml = me.getAsString();
				if (xml.length() > RNSConfig.limitMetadataSize()) {
					throw new Exception("exceed the limit of metadata size: "
							+ xml.length());
				}
				SerialClob clob = new SerialClob(xml.toCharArray()); /* copy */
				xml = null;
				metapsReplaceMetadata.setClob(3, clob);

				metapsReplaceMetadata.execute();
			}
		} catch (Exception e) {
			logger.error("RNSDBDerby:setList:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static final String DUMMY = "__dummy__";

	/* synchronized by nameIdToName */
	private static ReferenceMap namesCache = new ReferenceMap(
			ReferenceMap.SOFT, ReferenceMap.SOFT);

	private static PreparedStatement psIdToName = null;

	/* synchronized by caller methods */
	private static String nameIdToName(int nameId) throws SQLException {
		String s = (String) namesCache.get(nameId);
		if (s != null) {
			return s;
		}
		ResultSet rs = null;
		try {
			if (psIdToName == null) {
				String sql = "SELECT name FROM Names WHERE nameid = ?";
				psIdToName = conn.prepareStatement(sql);
			}
			psIdToName.setInt(1, nameId);
			rs = psIdToName.executeQuery();
			if (rs.next()) {
				String s2 = rs.getString(1);
				namesCache.put(nameId, s2);
				return s2;
			}
			return null;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:nameIdToName:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psGetACL = null;

	private synchronized static ACLEntryType[] dbGetACL(long dirid)
			throws SQLException {
		ResultSet rs = null;
		try {
			if (psGetACL == null) {
				String sql = "SELECT type,nameid,perm FROM Acl WHERE dirid = ?";
				psGetACL = conn.prepareStatement(sql);
			}
			psGetACL.setLong(1, dirid);
			rs = psGetACL.executeQuery();
			ArrayList<ACLEntryType> al = new ArrayList<ACLEntryType>();
			while (rs.next()) {
				ACLEntryType ent = new ACLEntryType();
				ent.setType(rs.getShort(1));
				String name = nameIdToName(rs.getInt(2));
				if (name == null) {
					// unexpected
					throw new SQLException("unexpected:unknown nameid");
				}
				if (DUMMY.equals(name)) {
					name = null;
				}
				ent.setName(name);
				ent.setPerm(rs.getShort(3));
				al.add(ent);
			}
			return al.toArray(new ACLEntryType[0]);
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbGetACL:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psAddNameToNames = null;
	private static PreparedStatement psAddNameToNames_id = null;

	private static int addNameToNames(String name) throws SQLException {
		ResultSet rs = null;
		try {
			if (psAddNameToNames == null) {
				String sql = "INSERT INTO Names (name) VALUES (?)";
				psAddNameToNames = conn.prepareStatement(sql);
			}
			psAddNameToNames.setString(1, name);
			psAddNameToNames.execute();

			if (psAddNameToNames_id == null) {
				String sql = "VALUES IDENTITY_VAL_LOCAL()";
				psAddNameToNames_id = conn.prepareStatement(sql);
			}
			rs = psAddNameToNames_id.executeQuery();
			if (rs.next()) {
				return rs.getInt(1);
			}
			return -1;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:addNameToNames:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	/* synchronized by nameToNameId */
	private static ReferenceMap namesCache2 = new ReferenceMap(
			ReferenceMap.SOFT, ReferenceMap.SOFT);

	private static PreparedStatement psNameToNameId = null;

	/* synchronized by caller methods */
	private static int nameToNameId(String name, boolean regist)
			throws SQLException {
		Integer i = (Integer) namesCache2.get(name);
		if (i != null) {
			return i.intValue();
		}
		ResultSet rs = null;
		try {
			if (psNameToNameId == null) {
				String sql = "SELECT nameid FROM Names WHERE name = ?";
				psNameToNameId = conn.prepareStatement(sql);
			}
			psNameToNameId.setString(1, name);
			rs = psNameToNameId.executeQuery();
			if (rs.next()) {
				int i2 = rs.getInt(1);
				namesCache2.put(name, Integer.valueOf(i2));
				return i2;
			}
			if (regist) {
				return addNameToNames(name);
			} else {
				return -1;
			}
		} catch (SQLException e) {
			logger.error("RNSDBDerby:nameIdToName:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psTypeExists = null;

	private static long typeExists(long dirid, short type) throws SQLException {
		ResultSet rs = null;
		try {
			if (psTypeExists == null) {
				String sql = "SELECT nameid FROM Acl WHERE dirid = ? AND type = ?";
				psTypeExists = conn.prepareStatement(sql);
			}
			psTypeExists.setLong(1, dirid);
			psTypeExists.setShort(2, type);
			rs = psTypeExists.executeQuery();
			if (rs.next()) {
				return rs.getLong(1);
			}
			return -1;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:typeExists:" + e.getMessage());
			throw e;
		} finally {
			if (rs != null) {
				try {
					rs.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	private static PreparedStatement psUpdateAcl = null;

	private static boolean updateAcl(long dirid, short type, int nameid,
			short perm) throws SQLException {
		try {
			if (psUpdateAcl == null) {
				String sql = "UPDATE Acl SET perm = ? WHERE dirid = ? AND type = ? AND nameid = ?";
				psUpdateAcl = conn.prepareStatement(sql);
			}
			psUpdateAcl.setShort(1, perm);
			psUpdateAcl.setLong(2, dirid);
			psUpdateAcl.setShort(3, type);
			psUpdateAcl.setInt(4, nameid);
			psUpdateAcl.execute();
			return true;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:updateAcl:" + e.getMessage());
			throw e;
		}
	}

	private static PreparedStatement psInsertAcl = null;

	private static boolean insertAcl(long dirid, short type, int nameid,
			short perm) throws SQLException {
		try {
			if (psInsertAcl == null) {
				String sql = "INSERT INTO Acl (dirid,type,nameid,perm) VALUES (?,?,?,?)";
				psInsertAcl = conn.prepareStatement(sql);
			}
			psInsertAcl.setLong(1, dirid);
			psInsertAcl.setShort(2, type);
			psInsertAcl.setInt(3, nameid);
			psInsertAcl.setShort(4, perm);
			psInsertAcl.execute();
			return true;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:insertAcl:" + e.getMessage());
			return false;
		}
	}

	private synchronized static void dbSetACL(long dirid, ACLEntryType[] list)
			throws SQLException {
		for (ACLEntryType ent : list) {
			short type = ent.getType();
			if (type == ACL.TYPE_OWNER || type == ACL.TYPE_OWNERGROUP
					|| type == ACL.TYPE_MASK || type == ACL.TYPE_OTHER
					|| type == ACL.TYPE_DEFAULT_OWNER
					|| type == ACL.TYPE_DEFAULT_OWNERGROUP
					|| type == ACL.TYPE_DEFAULT_MASK
					|| type == ACL.TYPE_DEFAULT_OTHER) {
				long l = typeExists(dirid, type);
				if (l >= 0) {
					dbRemoveACL(dirid, type, null); /* remove all entries of this type */
				}
			}
			String name = ent.getName();
			if (name == null) {
				name = DUMMY;
			}
			int nameid = nameToNameId(name, true);
			short perm = ent.getPerm();
			if (insertAcl(dirid, type, nameid, perm) == false) {
				logger.debug("retry UPDATE instead of INSERT");
				updateAcl(dirid, type, nameid, perm);
			}
		}
	}

	private synchronized static void dbRemoveACL(long dirid, short type,
			String[] names) throws SQLException {
		PreparedStatement ps = null;
		try {
			StringBuilder sql = new StringBuilder(
					"DELETE FROM Acl WHERE dirid = ? AND type = ? ");
			if (names != null && names.length > 0) {
				for (int i = 0; i < names.length; i++) {
					if (i == 0) {
						sql.append("AND (nameid = ?");
					} else {
						sql.append("OR nameid = ?");
					}
				}
				sql.append(")");
			}
			ps = conn.prepareStatement(sql.toString());
			ps.setLong(1, dirid);
			ps.setShort(2, type);
			if (names != null) {
				for (int i = 0; i < names.length; i++) {
					String name = names[i];
					if (name == null) {
						name = DUMMY;
					}
					int nameid = nameToNameId(name, false);
					if (nameid < 0) {
						throw new SQLException("unknown name: " + name);
					}
					ps.setInt(3 + i, nameid);
				}
			}
			ps.execute();
			return;
		} catch (SQLException e) {
			logger.error("RNSDBDerby:dbRemoveACL:" + e.getMessage());
			throw e;
		} finally {
			if (ps != null) {
				try {
					ps.close();
				} catch (SQLException e) {
				}
			}
		}
	}

	/* ------------------------------------------------------------------- */
	public RNSDBDerby() throws Exception {
		dbConnect();
	}

	public void createAndSetNewRootID() {
		id = ROOTID;
	}

	public void createAndSetNewID() throws SQLException {
		long newid = dbCreateNewDir();
		if (newid > 0) {
			id = newid;
		}
		return;
	}

	public boolean setID(String id) throws NumberFormatException, SQLException {
		boolean ret = dbExistID(Long.parseLong(id));
		if (ret) {
			this.id = Long.parseLong(id);
		}
		return ret;
	}

	public String getRootID() {
		return Long.toString(ROOTID);
	}

	public String getID() {
		return Long.toString(id);
	}

	public Object getLockAndStartTransaction() {
		return globalLock;
	}

	public void commit() throws Exception {
		conn.commit();
	}

	public void rollback() {
		try {
			conn.rollback();
		} catch (SQLException e) {
			e.printStackTrace();
			logger.info("rollback error: " + e.getMessage());
		}
	}

	public long getListSize() throws SQLException {
		return dbGetListSize(id);
	}

	public List<String> getList() throws Exception {
		List<String> l;
		try {
			l = dbGetList(id);
		} catch (SQLException e) {
			logger.fatal(e.getMessage());
			throw e;
		}
		return l;
	}

	public RNSDirectoryProperties getDirectoryProperties() throws Exception {
		try {
			return dbGetDirProps(id);
		} catch (SQLException e) {
			logger.fatal(e.getMessage());
			throw e;
		}
	}

	public void setDirectoryProperties(RNSDirectoryProperties props)
			throws SQLException {
		dbUpdateDirProps(id, props);
	}

	public void setAccessTime(Calendar t) throws SQLException {
		dbSetTime(id, t, "accessTime");
	}

	public void setCreateTime(Calendar t) throws SQLException {
		dbSetTime(id, t, "createTime");
	}

	public void setModificationTime(Calendar t) throws SQLException {
		dbSetTime(id, t, "modificationTime");
	}

	public RNSEntryData getEntryData(String name) throws Exception {
		return dbGetEntryData(id, name);
	}

	public void insertEntryData(String name, RNSEntryData ent) throws Exception {
		dbInsertEntry(id, name, ent);
	}

	public void removeEntryData(String name) throws SQLException {
		dbDeleteEntry(id, name);
	}

	public void destroy() throws Exception {
		if (id == ROOTID) {
			throw new Exception("Don't destroy root directory");
		}
		dbDeleteDir(id);
	}

	public void rename(String from, String to) throws Exception {
		dbRename(id, from, to);
	}

	public void replaceMetadata(String name, MessageElement[] xmls)
			throws Exception {
		dbReplaceMetadata(id, name, xmls);
	}

	public ACL getACL() throws Exception {
		ACLEntryType[] list = dbGetACL(id);
		return new ACL(list);
	}

	public void setACL(ACL acl) throws Exception {
		dbSetACL(id, acl.toACLEntries());
	}

	public void removeACL(short type, String[] names) throws Exception {
		dbRemoveACL(id, type, names);
	}
}
