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
package org.naregi.rns;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;

import org.naregi.rns.stubs.ACLEntryType;

/**
 * ACL structure for Server side and Client side.
 */
public class ACL {

	/* ----------------------------------- */

	/*
	 * owner, ownerGroup
	 * null : not set a name
	 * "" : not change the name (change perm only)
	 * other string = change the name
	 */
	private String owner;
	private short ownerPerm;

	private String ownerGroup;
	private short ownerGroupPerm;

	/* ----------------------------------- */

	private static final String DUMMY = "__dummy__";

	/*
	 * null : not set the ACL entry
	 * dummy(any string) : set the ACL entry
	 */
	private String otherDummy;
	private short otherPerm;

	private String defaultOwnerDummy;
	private short defaultOwnerPerm;

	private String defaultOwnerGroupDummy;
	private short defaultOwnerGroupPerm;

	private String defaultOtherDummy;
	private short defaultOtherPerm;

	private String maskDummy;
	private short mask;

	private String defaultMaskDummy;
	private short defaultMask;

	/* ----------------------------------- */
	private Map<String, Short> userMap;
	private Map<String, Short> groupMap;
	private Map<String, Short> defaultUserMap;
	private Map<String, Short> defaultGroupMap;

	/*----------------------------------- */
	/**
	 * String of anonymous user and group
	 */
	public static final String ANONYMOUS = "anonymous";

	/**
	 * owner
	 */
	public static final short TYPE_OWNER = 1;

	/**
	 * owner group
	 */
	public static final short TYPE_OWNERGROUP = 2;

	/**
	 * user
	 */
	public static final short TYPE_USER = 3;

	/**
	 * group
	 */
	public static final short TYPE_GROUP = 4;

	/**
	 * mask
	 */
	public static final short TYPE_MASK = 5;

	/**
	 * other
	 */
	public static final short TYPE_OTHER = 6;

	/**
	 * default owner
	 */
	public static final short TYPE_DEFAULT_OWNER = 7;

	/**
	 * default owner group
	 */
	public static final short TYPE_DEFAULT_OWNERGROUP = 8;

	/**
	 * default user
	 */
	public static final short TYPE_DEFAULT_USER = 9;

	/**
	 * default group
	 */
	public static final short TYPE_DEFAULT_GROUP = 10;

	/**
	 * default mask
	 */
	public static final short TYPE_DEFAULT_MASK = 11;

	/**
	 * default other
	 */
	public static final short TYPE_DEFAULT_OTHER = 12;

	/**
	 * allow to read
	 */
	public static final short PERM_READ = 4;

	/**
	 * allow to write
	 */
	public static final short PERM_WRITE = 2;

	/**
	 * allow to execute (This is not used in RNS)
	 */
	public static final short PERM_EXEC = 1;

	/**
	 * allow to do all operations
	 */
	public static final short PERM_ALL = 7;

	/**
	 * do not allow to do all operations
	 */
	public static final short PERM_ZERO = 0;

	/* ACLEntryType
		<xsd:complexType name="ACLEntryType">
			<xsd:sequence>
				<xsd:element name="type" type="xsd:short" minOccurs="1"
					maxOccurs="1" nillable="false" />
				<xsd:element name="name" type="xsd:string" minOccurs="0"
					maxOccurs="1" nillable="true" />
				<xsd:element name="perm" type="xsd:short" minOccurs="1"
					maxOccurs="1" nillable="false" />
			</xsd:sequence>
		</xsd:complexType>
	*/

	/**
	 * Create a new ACL instance.
	 */
	public ACL() {
		this((ACLEntryType[]) null);
	}

	/**
	 * Create a new ACL instance.
	 *
	 * This aclSpec is the following format.
	 *
	 * <pre>
	 * --- aclSpec format ---
	 * default(d):owner(ou):[name]:rwx
	 * default(d):ownergroup(og):[name]:rwx
	 * default(d):user(u):name:rwx
	 * default(d):group(g):name:rwx
	 * default(d):mask(m):rwx
	 * default(d):other(o):rwx
	 * owner(ou):[name]:rwx
	 * ownergroup(og):[name]:rwx
	 * user(u):name:rwx
	 * group(g):name:rwx
	 * mask(m):rwx
	 * other(o):rwx
	 * </pre>
	 *
	 * @param aclSpecs aclSpec array
	 */
	public ACL(String[] aclSpecs) throws Exception {
		this((ACLEntryType[]) null);
		setACLString(aclSpecs);
	}

	/**
	 * Create a new ACL instance.
	 *
	 * @param list array of ACLEntryType
	 */
	public ACL(ACLEntryType[] list) {
		userMap = Collections.synchronizedMap(new HashMap<String, Short>());
		groupMap = Collections.synchronizedMap(new HashMap<String, Short>());
		defaultUserMap = Collections.synchronizedMap(new HashMap<String, Short>());
		defaultGroupMap = Collections.synchronizedMap(new HashMap<String, Short>());
		mask = defaultMask = 7;
		owner = ownerGroup = otherDummy = defaultOwnerDummy = defaultOwnerGroupDummy = defaultOtherDummy = maskDummy = defaultMaskDummy = null;
		if (list == null) {
			return;
		}
		for (ACLEntryType e : list) {
			switch (e.getType()) {
			case TYPE_OWNER:
				owner = e.getName();
				ownerPerm = e.getPerm();
				break;
			case TYPE_OWNERGROUP:
				ownerGroup = e.getName();
				ownerGroupPerm = e.getPerm();
				break;
			case TYPE_OTHER:
				otherDummy = DUMMY;
				otherPerm = e.getPerm();
				break;
			case TYPE_DEFAULT_OTHER:
				defaultOtherDummy = DUMMY;
				defaultOtherPerm = e.getPerm();
				break;
			case TYPE_MASK:
				maskDummy = DUMMY;
				mask = e.getPerm();
				break;
			case TYPE_DEFAULT_MASK:
				defaultMaskDummy = DUMMY;
				defaultMask = e.getPerm();
				break;
			case TYPE_USER:
				userMap.put(e.getName(), Short.valueOf(e.getPerm()));
				break;
			case TYPE_GROUP:
				groupMap.put(e.getName(), Short.valueOf(e.getPerm()));
				break;
			case TYPE_DEFAULT_USER:
				defaultUserMap.put(e.getName(), Short.valueOf(e.getPerm()));
				break;
			case TYPE_DEFAULT_GROUP:
				defaultGroupMap.put(e.getName(), Short.valueOf(e.getPerm()));
				break;
			case TYPE_DEFAULT_OWNER:
				defaultOwnerDummy = DUMMY;
				defaultOwnerPerm = e.getPerm();
				break;
			case TYPE_DEFAULT_OWNERGROUP:
				defaultOwnerGroupDummy = DUMMY;
				defaultOwnerGroupPerm = e.getPerm();
				break;
			default:
				// do nothing
			}
		}
	}

	private static boolean can(short perm, short mode) {
		return (perm & mode) > 0 ? true : false;
	}

	/**
	 * Check whether the permission bits has PERM_READ bit.
	 *
	 * @param perm a permission bits
	 * @return true if the permission bits has PERM_READ bit.
	 */
	public static boolean canRead(short perm) {
		return can(perm, PERM_READ);
	}

	/**
	 * Check whether the permission bits has PERM_WRITE bit.
	 *
	 * @param perm a permission bits
	 * @return true if the permission bits has PERM_WRITE bit.
	 */
	public static boolean canWrite(short perm) {
		return can(perm, PERM_WRITE);
	}

	/**
	 * Check whether the permission bits has PERM_EXEC bit.
	 *
	 * @param perm a permission bits
	 * @return true if the permission bits has PERM_EXEC bit.
	 */
	public static boolean canExec(short perm) {
		return can(perm, PERM_EXEC);
	}

	/**
	 * Convert permission bits to String.
	 *
	 * @param perm permission bits
	 * @return String of rwx from
	 */
	public static String permToString(short perm) {
		String str;
		if (canRead(perm)) {
			str = "r";
		} else {
			str = "-";
		}
		if (canWrite(perm)) {
			str += "w";
		} else {
			str += "-";
		}
		if (canExec(perm)) {
			str += "x";
		} else {
			str += "-";
		}
		return str;
	}

	/**
	 * Convert String of ACL type to TYPE_*.
	 *
	 * @param type a name of ACL type
	 * @return a TYPE_* of ACL type
	 */
	public static short typeStringToShort(String type) {
		String[] s = type.split(":");
		if (s.length == 2 && "default".endsWith(s[0]) || "d".equals(s[0])) {
			if ("owner".equals(s[1]) || "ou".equals(s[1])) {
				return TYPE_DEFAULT_OWNER;
			} else if ("ownergroup".equals(s[1]) || "og".equals(s[1])) {
				return TYPE_DEFAULT_OWNERGROUP;
			} else if ("user".equals(s[1]) || "u".equals(s[1])) {
				return TYPE_DEFAULT_USER;
			} else if ("group".equals(s[1]) || "g".equals(s[1])) {
				return TYPE_DEFAULT_GROUP;
			} else if ("mask".equals(s[1]) || "m".equals(s[1])) {
				return TYPE_DEFAULT_MASK;
			} else if ("other".equals(s[1]) || "o".equals(s[1])) {
				return TYPE_DEFAULT_OTHER;
			}
		} else {
			if ("owner".equals(s[0]) || "ou".equals(s[0])) {
				return TYPE_OWNER;
			} else if ("ownergroup".equals(s[0]) || "og".equals(s[0])) {
				return TYPE_OWNERGROUP;
			} else if ("user".equals(s[0]) || "u".equals(s[0])) {
				return TYPE_USER;
			} else if ("group".equals(s[0]) || "g".equals(s[0])) {
				return TYPE_GROUP;
			} else if ("mask".equals(s[0]) || "m".equals(s[0])) {
				return TYPE_MASK;
			} else if ("other".equals(s[0]) || "o".equals(s[0])) {
				return TYPE_OTHER;
			}
		}
		return -1;
	}

	private static short charToShort(char c) {
		if (c == 'r') {
			return PERM_READ;
		} else if (c == 'w') {
			return PERM_WRITE;
		} else if (c == 'x') {
			return PERM_EXEC;
		}
		return 0;
	}

	/**
	 * Convert rwx form to permission bits
	 *
	 * @param permStr String of rwx form (ex. rw-, r--, r-x, ---)
	 * @return permission bits
	 */
	public static short permStringToShort(String permStr) {
		if (permStr == null) {
			return 0;
		}
		short ret = 0;
		int len = permStr.length();
		if (len >= 1) {
			ret |= charToShort(permStr.charAt(0));
		}
		if (len >= 2) {
			ret |= charToShort(permStr.charAt(1));
		}
		if (len >= 3) {
			ret |= charToShort(permStr.charAt(2));
		}
		return ret;
	}

	/**
	 * Set (replace or add) an ACL entry.
	 *
	 * The aclSpec format is described at {@link ACL#ACL(String[])}.
	 *
	 * @param aclSpec aclSpec
	 * @throws Exception if an error occurs
	 */
	public void setACLString(String aclSpec) throws Exception {
		/*
		 * s.length
		 * 4 default(d):owner(ou):[name]:rwx
		 * 4 default(d):ownergroup(og):[name]:rwx
		 * 4 default(d):user(u):name:rwx
		 * 4 default(d):group(g):name:rwx
		 * 3 default(d):mask(m):rwx
		 * 3 default(d):other(o):rwx
		 * 3 owner(ou):[name]:rwx
		 * 3 ownergroup(og):[name]:rwx
		 * 3 user(u):name:rwx
		 * 3 group(g):name:rwx
		 * 2 mask(m):rwx
		 * 2 other(o):rwx
		 */
		String[] s = aclSpec.split(":");
		if (s == null || s.length < 2) {
			throw new Exception("syntax error: " + aclSpec);
		}
		String type = s[0];
		if ("default".endsWith(type) || "d".equals(type)) {
			type = s[1];
			if (s.length == 4) {
				if ("owner".equals(type) || "ou".equals(type)) {
					defaultOwnerDummy = DUMMY;
					defaultOwnerPerm = permStringToShort(s[3]);
					return;
				} else if ("ownergroup".equals(type) || "og".equals(type)) {
					defaultOwnerGroupDummy = DUMMY;
					defaultOwnerGroupPerm = permStringToShort(s[3]);
					return;
				} else if ("user".equals(type) || "u".equals(type)) {
					defaultUserMap.put(s[2],
							Short.valueOf(permStringToShort(s[3])));
					return;
				} else if ("group".equals(type) || "g".equals(type)) {
					defaultGroupMap.put(s[2],
							Short.valueOf(permStringToShort(s[3])));
					return;
				}
			} else if (s.length == 3) {
				if ("mask".equals(type) || "m".equals(type)) {
					defaultMaskDummy = DUMMY;
					defaultMask = permStringToShort(s[2]);
					return;
				} else if ("other".equals(type) || "o".equals(type)) {
					defaultOtherDummy = DUMMY;
					defaultOtherPerm = permStringToShort(s[2]);
					return;
				}
			}
		} else {
			if (s.length == 3) {
				if ("owner".equals(type) || "ou".equals(type)) {
					owner = s[1]; // "" = not change owner
					ownerPerm = permStringToShort(s[2]);
					return;
				} else if ("ownergroup".equals(type) || "og".equals(type)) {
					ownerGroup = s[1]; // "" = not change ownerGroup
					ownerGroupPerm = permStringToShort(s[2]);
					return;
				} else if ("user".equals(type) || "u".equals(type)) {
					userMap.put(s[1], Short.valueOf(permStringToShort(s[2])));
					return;
				} else if ("group".equals(type) || "g".equals(type)) {
					groupMap.put(s[1], Short.valueOf(permStringToShort(s[2])));
					return;
				}
			} else if (s.length == 2) {
				if ("mask".equals(type) || "m".equals(type)) {
					maskDummy = DUMMY;
					mask = permStringToShort(s[1]);
					return;
				} else if ("other".equals(type) || "o".equals(type)) {
					otherDummy = DUMMY;
					otherPerm = permStringToShort(s[1]);
					return;
				}
			}
		}
		throw new Exception("syntax error: " + aclSpec);
	}

	/**
	 * Set (replace or add) ACL entries.
	 *
	 * The aclSpec format is described at {@link ACL#ACL(String[])}.
	 *
	 * @param aclSpecs aclSpec array (not null)
	 * @throws Exception if an error occurs
	 */
	public void setACLString(String[] aclSpecs) throws Exception {
		for (String aclSpec : aclSpecs) {
			setACLString(aclSpec);
		}
	}

	private void setACLEntryType(ArrayList<ACLEntryType> al, short type,
			String name, short perm) {
		if (name == null) {
			return;
		}
		if (DUMMY.equals(name)) {
			name = null;
		}
		ACLEntryType acle = new ACLEntryType(type, name, perm);
		al.add(acle);
	}

	private void setACLEntryTypeForMap(ArrayList<ACLEntryType> al, short type,
			Map<String, Short> map) {
		Set<Entry<String, Short>> es = map.entrySet();
		for (Entry<String, Short> e : es) {
			setACLEntryType(al, type, e.getKey(), e.getValue());
		}
	}

	/**
	 * Convert this object to ACLEntryType array.
	 *
	 * @return ACLEntryType array
	 */
	public ACLEntryType[] toACLEntries() {
		ArrayList<ACLEntryType> al = new ArrayList<ACLEntryType>();
		setACLEntryType(al, TYPE_OWNER, owner, ownerPerm);
		setACLEntryType(al, TYPE_OWNERGROUP, ownerGroup, ownerGroupPerm);
		setACLEntryTypeForMap(al, TYPE_USER, userMap);
		setACLEntryTypeForMap(al, TYPE_GROUP, groupMap);
		setACLEntryType(al, TYPE_MASK, maskDummy, mask);
		setACLEntryType(al, TYPE_OTHER, otherDummy, otherPerm);
		setACLEntryType(al, TYPE_DEFAULT_OWNER, defaultOwnerDummy,
				defaultOwnerPerm);
		setACLEntryType(al, TYPE_DEFAULT_OWNERGROUP, defaultOwnerGroupDummy,
				defaultOwnerGroupPerm);
		setACLEntryTypeForMap(al, TYPE_DEFAULT_USER, defaultUserMap);
		setACLEntryTypeForMap(al, TYPE_DEFAULT_GROUP, defaultGroupMap);
		setACLEntryType(al, TYPE_DEFAULT_MASK, defaultMaskDummy, defaultMask);
		setACLEntryType(al, TYPE_DEFAULT_OTHER, defaultOtherDummy,
				defaultOtherPerm);

		return al.toArray(new ACLEntryType[0]);
	}

	/**
	 * Complete missing ACL entries automatically. This must be used after
	 * reading from DB or before doing toString(). This should not be used
	 * before writing into DB or sending Server.
	 */
	public void autoComplete() {
		if (owner == null) {
			owner = ANONYMOUS;
			ownerPerm = ACL.PERM_ALL;
		}
		if (ownerGroup == null) {
			ownerGroup = ANONYMOUS;
			ownerGroupPerm = ACL.PERM_ALL;
		}
		if (otherDummy == null) {
			otherDummy = DUMMY;
			otherPerm = ACL.PERM_ALL;
		}
		if (maskDummy == null && (userMap.size() > 0 || groupMap.size() > 0)) {
			short newMask = ownerGroupPerm;
			Collection<Short> v = userMap.values();
			for (Short i : v) {
				newMask |= i.shortValue();
			}
			v = groupMap.values();
			for (Short i : v) {
				newMask |= i.shortValue();
			}
			maskDummy = DUMMY;
			mask = newMask;
		}
		if (defaultUserMap.size() > 0 || defaultGroupMap.size() > 0) {
			if (defaultMaskDummy == null) {
				short newMask = defaultOwnerGroupPerm;
				Collection<Short> v = defaultUserMap.values();
				for (Short i : v) {
					newMask |= i.shortValue();
				}
				v = defaultGroupMap.values();
				for (Short i : v) {
					newMask |= i.shortValue();
				}
				defaultMaskDummy = DUMMY;
				defaultMask = newMask;
			}
			if (defaultOwnerDummy == null) {
				defaultOwnerDummy = DUMMY;
				defaultOwnerPerm = ownerPerm;
			}
			if (defaultOwnerGroupDummy == null) {
				defaultOwnerGroupDummy = DUMMY;
				defaultOwnerGroupPerm = ownerGroupPerm;
			}
			if (defaultOtherDummy == null) {
				defaultOtherDummy = DUMMY;
				defaultOtherPerm = otherPerm;
			}
		}
	}

	private static String ls = System.getProperty("line.separator");

	private void generatePermList(StringBuilder sb, String type, String name,
			short perm, short mask) {
		if (name == null) {
			return;
		}
		sb.append(type);
		if (DUMMY.equals(name) == false) {
			sb.append(name);
			sb.append(":");
		}
		sb.append(permToString(perm));
		if (mask >= 0) {
			short ef = (short) (perm & mask);
			if (perm != ef) {
				sb.append("    #effective:");
				sb.append(permToString(ef));
			}
		}
		sb.append(ls);
	}

	private void generatePermListForMap(StringBuilder sb, String type,
			Map<String, Short> map, short mask) {
		Set<Entry<String, Short>> es = map.entrySet();
		for (Entry<String, Short> e : es) {
			sb.append(type);
			sb.append(e.getKey());
			sb.append(":");
			sb.append(permToString(e.getValue()));
			short ef = (short) (e.getValue().shortValue() & mask);
			if (ef != e.getValue().shortValue()) {
				sb.append("    #effective:");
				sb.append(permToString(ef));
			}
			sb.append(ls);
		}
	}

	public String toString() {
		StringBuilder sb = new StringBuilder();

		sb.append("# owner: ");
		if (owner != null) {
			sb.append(owner);
		} else {
			sb.append(ANONYMOUS);
		}
		sb.append(ls);

		sb.append("# group: ");
		if (ownerGroup != null) {
			sb.append(ownerGroup);
		} else {
			sb.append(ANONYMOUS);
		}
		sb.append(ls);

		sb.append("user::");
		sb.append(permToString(ownerPerm));
		sb.append(ls);

		generatePermListForMap(sb, "user:", userMap, mask);

		sb.append("group::");
		sb.append(permToString(ownerGroupPerm));
		short ef = (short) (ownerGroupPerm & mask);
		if (ef != ownerGroupPerm) {
			sb.append("    #effective:");
			sb.append(permToString(ef));
		}
		sb.append(ls);

		generatePermListForMap(sb, "group:", groupMap, mask);
		generatePermList(sb, "mask:", maskDummy, mask, (short) -1);

		sb.append("other:");
		sb.append(permToString(otherPerm));
		sb.append(ls);

		generatePermList(sb, "default:user::", defaultOwnerDummy,
				defaultOwnerPerm, (short) -1);
		generatePermListForMap(sb, "default:user:", defaultUserMap, defaultMask);
		generatePermList(sb, "default:group::", defaultOwnerGroupDummy,
				defaultOwnerGroupPerm, mask);
		generatePermListForMap(sb, "default:group:", defaultGroupMap,
				defaultMask);
		generatePermList(sb, "default:mask:", defaultMaskDummy, defaultMask,
				(short) -1);
		generatePermList(sb, "default:other:", defaultOtherDummy,
				defaultOtherPerm, (short) -1);

		return sb.toString();
	}

	/**
	 * Convert this ACL to String[] of AclSpecs
	 *
	 * @return String[] of AclSpecs
	 */
	public String[] toAclSpecs() {
		ArrayList<String> al = new ArrayList<String>();

		if (owner != null) {
			al.add("ou:" + owner + ":" + permToString(ownerPerm));
		}
		if (ownerGroup != null) {
			al.add("og:" + ownerGroup + ":" + permToString(ownerGroupPerm));
		}
		Set<Entry<String, Short>> um = userMap.entrySet();
		for (Entry<String, Short> e : um) {
			al.add("u:" + e.getKey() + ":" + permToString(e.getValue()));
		}
		Set<Entry<String, Short>> gm = groupMap.entrySet();
		for (Entry<String, Short> e : gm) {
			al.add("g:" + e.getKey() + ":" + permToString(e.getValue()));
		}
		if (maskDummy != null) {
			al.add("m:" + permToString(mask));
		}
		if (otherDummy != null) {
			al.add("o:" + permToString(otherPerm));
		}

		Set<Entry<String, Short>> dum = defaultUserMap.entrySet();
		for (Entry<String, Short> e : dum) {
			al.add("d:u:" + e.getKey() + ":" + permToString(e.getValue()));
		}
		Set<Entry<String, Short>> dgm = defaultGroupMap.entrySet();
		for (Entry<String, Short> e : dgm) {
			al.add("d:g:" + e.getKey() + ":" + permToString(e.getValue()));
		}
		if (defaultMaskDummy != null) {
			al.add("d:m:" + permToString(defaultMask));
		}
		if (defaultOtherDummy != null) {
			al.add("d:o:" + permToString(defaultOtherPerm));
		}

		return (String[]) al.toArray(new String[0]);
	}

	/**
	 * Get the name of owner.
	 *
	 * @return a name
	 */
	public String getOwner() {
		return owner;
	}

	/**
	 * Get the permission bits of owner.
	 *
	 * @return the permission bits
	 */
	public short getOwnerPerm() {
		return ownerPerm;
	}

	/**
	 * Set the ACL entry of owner.
	 *
	 * @param name a name
	 * @param perm permission bits
	 */
	public void setOwner(String name, short perm) {
		owner = name;
		ownerPerm = perm;
	}

	/**
	 * Get the name of owner group
	 *
	 * @return a name
	 */
	public String getOwnerGroup() {
		return ownerGroup;
	}

	/**
	 * Get the permission bits of owner group
	 *
	 * @return permission bits
	 */
	public short getOwnerGroupPerm() {
		short ret = ownerGroupPerm;
		if (maskDummy != null) {
			ret &= mask;
		}
		return ret;
	}

	/**
	 * Set the ACL entry of owner group.
	 *
	 * @param name a name
	 * @param perm permission bits
	 */
	public void setOwnerGroup(String name, short perm) {
		ownerGroup = name;
		ownerGroupPerm = perm;
	}

	/**
	 * Set the permission bits of other (not match any other entry)
	 *
	 * @param perm permission bits
	 */
	public void setOther(short perm) {
		otherDummy = DUMMY;
		otherPerm = perm;
	}

	/**
	 * Get the permission bits of the specified user.
	 *
	 * @param name a user name
	 * @return permission bits as Short. return null if the name does not exist
	 */
	public Short getUserPerm(String name) {
		Short i = userMap.get(name);
		if (i == null) {
			return null;
		}
		short ret = i.shortValue();
		if (maskDummy != null) {
			ret &= mask;
		}
		return ret;
	}

	/**
	 * Clear permission bits of all users.
	 */
	public void clearUserPerm() {
		userMap.clear();
	}

	/**
	 * Get the permission bits of the specified group
	 *
	 * @param name a group name
	 * @return permission bits as Short. return null if the name does not exist
	 */
	public Short getGroupPerm(String name) {
		Short i = groupMap.get(name);
		if (i == null) {
			return null;
		}
		short ret = i.shortValue();
		if (maskDummy != null) {
			ret &= mask;
		}
		return ret;
	}

	/**
	 * Clear permission bits of all groups.
	 */
	public void clearGroupPerm() {
		groupMap.clear();
	}

	/**
	 * Reset the ACL entry of mask.
	 */
	public void clearMask() {
		maskDummy = null;
		mask = 7;
	}

	/**
	 * Get the permission bits of other.
	 *
	 * @return permission bits
	 */
	public short getOtherPerm() {
		return otherPerm;
	}

	/**
	 * Get Map for user.
	 *
	 * @return Map (a name to permission bits)
	 */
	public Map<String, Short> getUserMap() {
		return userMap;
	}

	/**
	 * Get Map for group.
	 *
	 * @return Map (a name to permission bits)
	 */
	public Map<String, Short> getGroupMap() {
		return groupMap;
	}

	/**
	 * Get Map for default user.
	 *
	 * @return Map (a name to permission bits)
	 */
	public Map<String, Short> getDefaultUserMap() {
		return defaultUserMap;
	}

	/**
	 * Get Map for default group.
	 *
	 * @return Map (a name to permission bits)
	 */
	public Map<String, Short> getDefaultGroupMap() {
		return defaultGroupMap;
	}

	/**
	 * Inherit default ACL.
	 */
	public void copyAllDefaultPermToNormal() {
		Set<Entry<String, Short>> es = defaultUserMap.entrySet();
		for (Entry<String, Short> e : es) {
			userMap.put(e.getKey(), e.getValue());
		}
		Set<Entry<String, Short>> es2 = defaultGroupMap.entrySet();
		for (Entry<String, Short> e : es2) {
			groupMap.put(e.getKey(), e.getValue());
		}
		if (defaultMaskDummy != null) {
			maskDummy = DUMMY;
			mask = defaultMask;
		}
		if (defaultOtherDummy != null) {
			otherDummy = DUMMY;
			otherPerm = defaultOtherPerm;
		}
	}

	/**
	 * Check whether the ACL has extended ACL entries.
	 * (whether this ACL consists ACL entries of owner, ownergroup and other)
	 *
	 * @return true if an extended ACL entry exists in this ACL.
	 */
	public boolean hasExtension() {
		if (userMap.size() > 0 || groupMap.size() > 0 || maskDummy != null
				|| defaultOwnerDummy != null || defaultOwnerGroupDummy != null
				|| defaultUserMap.size() > 0 || defaultGroupMap.size() > 0
				|| defaultMaskDummy != null || defaultOtherDummy != null) {
			return true;
		} else {
			return false;
		}
	}
}
