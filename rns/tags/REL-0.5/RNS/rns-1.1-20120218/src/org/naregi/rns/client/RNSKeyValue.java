package org.naregi.rns.client;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;
import java.util.regex.Pattern;

import org.apache.axis.message.MessageElement;
import org.naregi.rns.util.RNSUtil;

/**
 * Key/Value operations to RNS Metadata. The key/value mapping is following
 * format in RNS Metadata.
 *
 * <pre>
 * &lt;rnskv key="key"&gt;value&lt;/rnskv&gt;
 * </pre>
 *
 * This Metadata usage is not a specification in RNS 1.1.
 *
 * This does not implement java.util.Map interface.
 */
public class RNSKeyValue {
	private RNSClient client;
	private RNSSearchClient searchClient;
	private String path;

	/**
	 * Initialize RNSKeyValue.
	 *
	 * @param home RNSClientHome
	 * @param path a pathname of a RNS entry.
	 */
	public RNSKeyValue(RNSClientHome home, String path) {
		client = home.getRNSClient();
		searchClient = home.getRNSSearchClient();
		this.path = path;
	}

	/**
	 * Returns true if this Metadata contains a mapping for the specified key.
	 *
	 * @param key key whose presence in this Metadata is to be tested
	 * @return true if this Metadata contains a mapping for the specified key
	 * @throws RNSError
	 */
	public boolean containsKey(String key) throws RNSError {
		String val = get(key);
		if (val != null) {
			return true;
		} else {
			return false;
		}
	}

	/**
	 * Returns the value to which the specified key is mapped, or null if this
	 * map contains no mapping for the key.
	 *
	 * @param key the key whose associated value is to be returned
	 * @return the value to which the specified key is mapped, or null if this
	 *         map contains no mapping for the key
	 * @throws RNSError
	 */
	public String get(String key) throws RNSError {
		key = RNSUtil.escapeXML(key);
		String xq = RNSUtil.generateXQueryForRNSSearch(
				"<rnskv key=\"{$key}\">{$value}</rnskv>", key);

		RNSSearchResult result = searchClient.search(path, xq);
		if (result == null || result.getMetadataString() == null
				|| result.getMetadataString().length() == 0) {
			/* no such key */
			return null;
		}
		MessageElement resXml;
		String str;
		try {
			resXml = RNSUtil.toMessageElement(result.getMetadataString());
			str = resXml.getValue();
		} catch (Exception e) {
			throw RNSError.createEINVAL(path, e.getMessage(), e);
		}
		if (str.length() == 0) {
			/* zero length */
			return "";
		}
		str = RNSUtil.unescapeXML(str);
		return str;
	}

	/**
	 * Returns a java.util.Set view of the keys contained in this map. The set
	 * is backed by the map, so changes to the map are reflected in the set, and
	 * vice-versa. If the map is modified while an iteration over the set is in
	 * progress (except through the iterator's own remove operation), the
	 * results of the iteration are undefined. The set supports element removal,
	 * which removes the corresponding mapping from the map, via the
	 * Iterator.remove, Set.remove, removeAll, retainAll, and clear operations.
	 * It does not support the add or addAll operations.
	 *
	 * @return a set view of the keys contained in this Metadata
	 * @throws RNSError
	 */
	public Set<String> keySet() throws RNSError {
		Set<String> ret = new HashSet<String>();
		String xq = RNSUtil.generateXQueryForRNSSearch("(for $m in $meta/* let $n := local-name($m) let $k := string($m/@key) where $n != \"supports-rns\" return <k>{$k}</k>)");

		RNSSearchResult result = searchClient.search(path, xq);
		if (result != null && result.getMetadataString() != null
				&& result.getMetadataString().length() != 0) {
			String[] s = Pattern.compile("<.+?>", Pattern.MULTILINE).split(
					result.getMetadataString());

			for (String key : s) {
				key = key.replace("\r", "").replace("\n", "");
				if (key != null && key.length() != 0) {
					key = RNSUtil.unescapeXML(key);
					ret.add(key);
				}
			}
		}
		return ret;
	}

	/**
	 * Associates the specified value with the specified key in this Metadata.
	 * If the Metadata previously contained a mapping for the key, the old value
	 * is replaced by the specified value. If the value is null, the map is
	 * removed.
	 *
	 * @param key
	 * @param value
	 * @throws RNSError
	 */
	public void put(String key, String value) throws RNSError {
		Map<String, String> map = new HashMap<String, String>();
		map.put(key, value);
		putAll(map);
	}

	@SuppressWarnings("unchecked")
	private List<MessageElement> getMessageElementChildren(MessageElement me) {
		return me.getChildren();
	}

	/**
	 * Copies all of the mappings from the specified map to this map. The effect
	 * of this call is equivalent to that of calling
	 * {@link RNSKeyValue#put(String, String) put(k, v)} on this map once for
	 * each mapping from key k to value v in the specified map.
	 *
	 * @param map mappings to be stored in this Metadata
	 * @throws RNSError
	 */
	public void putAll(Map<String, String> map) throws RNSError {
		/* query keys which is not used in the map */
		StringBuilder keysCondition = new StringBuilder();
		for (String key : map.keySet()) {
			if (key != null) {
				key = RNSUtil.escapeXML(key);
				keysCondition.append("and $key != \"" + key + "\" ");
			}
		}
		String xq = RNSUtil.generateXQueryForRNSSearch("(<r>{for $m in $meta/* let $n := local-name($m) let $key := if ($n = \"rnskv\") then string($m/@key) else \"\" where $n != \"supports-rns\" "
				+ keysCondition.toString() + " return $m}</r>)");

		RNSSearchResult result = searchClient.search(path, xq);
		if (result == null || result.getMetadataString() == null
				|| result.getMetadataString().length() == 0) {
			/* do nothing */
			return;
		}
		MessageElement resXml;
		try {
			resXml = RNSUtil.toMessageElement(result.getMetadataString());
		} catch (Exception e) {
			throw RNSError.createEINVAL(path, e.getMessage(), e);
		}

		/* <result>...</result> */
		List<MessageElement> l = getMessageElementChildren(resXml);
		if (l == null) {
			l = new ArrayList<MessageElement>();
		}
		for (Entry<String, String> ent : map.entrySet()) {
			String value = ent.getValue();
			if (value != null) {
				String key = RNSUtil.escapeXML(ent.getKey());
				value = RNSUtil.escapeXML(value);
				String addXml;
				if (value.length() == 0) {
					addXml = "<rnskv key=\"" + key + "\" />";
				} else {
					addXml = "<rnskv key=\"" + key + "\">" + value + "</rnskv>";
				}
				try {
					l.add(RNSUtil.toMessageElement(addXml));
				} catch (Exception e) {
					throw RNSError.createEINVAL(path, e.getMessage(), e);
				}
			}
			/* value == null -> remove */
		}
		MessageElement[] xmls = l.toArray(new MessageElement[0]);
		client.setMetadata(path, xmls);
	}

	/**
	 * Removes the mapping for a key from this map if it is present.
	 *
	 * @param key key whose mapping is to be removed from the map
	 * @throws RNSError
	 */
	public void remove(String key) throws RNSError {
		Map<String, String> map = new HashMap<String, String>();
		map.put(key, null);
		putAll(map);
	}

	/**
	 * Returns the number of key/value mappings in this Metadata.
	 *
	 * @return the number of key/value mappings in this Metadata
	 * @throws RNSError
	 */
	public int size() throws RNSError {
		return keySet().size();
	}

	/**
	 * Search the key recursively.
	 *
	 * @param key
	 * @param depth a number of the depth to search recursively
	 * @return path-to-value(result) pairs
	 * @throws RNSError
	 */
	public Map<String, String> searchChildren(String key, int depth)
			throws RNSError {
		key = RNSUtil.escapeXML(key);
		String xq = RNSUtil.generateXQueryForRNSSearch(
				"<rnskv key=\"{$key}\">{$value}</rnskv>", key);
		Map<String, String> map = new HashMap<String, String>();
		RNSSearchResultHandle results = searchClient.searchRecursive(path, xq,
				depth);
		if (results != null) {
			for (RNSSearchResult result : results) {
				String val = result.getMetadataString();
				if (val != null) {
					MessageElement resXml;
					try {
						resXml = RNSUtil.toMessageElement(val);
						val = resXml.getValue();
					} catch (Exception e) {
						throw RNSError.createEINVAL(path, e.getMessage(), e);
					}
					if (val.length() == 0) {
						val = "";
					} else {
						val = RNSUtil.unescapeXML(val);
					}
				} else if (result.getError() != null) {
					val = "ERROR: " + result.getError().getMessage();
				}
				map.put(result.getPath(), val);
			}
			RNSError e = results.getError();
			if (e != null) {
				throw e;
			}
		}
		return map;
	}
}
