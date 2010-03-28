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
 * $RCSfile: LdifUtils.java,v $ $Revision: 1.3 $ $Date: 2006/10/11 08:13:49 $
 */
package org.apgrid.ninf.ldifutils;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.axis.encoding.Base64;

public class LdifUtils {
	public static class ParseException extends Exception {
		private ParseException() {
		}

		private ParseException(final String message) {
			super(message);
		}

		private ParseException(final Throwable cause) {
			super(cause);
		}
	}

	public static Map parse(final BufferedReader bufferedReader) throws IOException, ParseException {
		final Map map = new HashMap();
		final StringBuffer buf = new StringBuffer("");
		{
			String line = bufferedReader.readLine();
			while (line != null) {
				if ((line.length() == 0) || (line.charAt(0) == '#')) {
					line = bufferedReader.readLine();
				} else if (line.charAt(0) == ' ') {
					throw new ParseException("illegal occurrence of continuation line");
				} else {
					for (;;) {
						buf.append(line);
						line = bufferedReader.readLine();
						if ((line == null) || (line.length() == 0) || (line.charAt(0) != ' ')) {
							break;
						}
						line = line.substring(1);
					}
				}
				flush(map, buf);
			}
		}
		flush(map, buf);
		return map;
	}

	private static void flush(final Map map, final StringBuffer buf) throws ParseException {
		boolean isBase64 = false;
		if (buf.length() == 0) {
			return;
		}
		final int index = buf.indexOf(":");
		if (index < 0) {
			throw new ParseException("cannot find ':'");
		}
		final String key = buf.substring(0, index);
		String value_spec = buf.substring(index + 1);
		if (value_spec.length() > 0) {
			if (value_spec.charAt(0) == ':') {
				isBase64 = true;
				value_spec = value_spec.substring(1);
				if (value_spec.length() == 0) {
					throw new ParseException("base64 string is empty");
				}
			} else if (value_spec.charAt(0) == '<') {
				throw new ParseException("'<' (URL refer) is not supported yet, sorry ... orz");
			}
			{
				int i = 0;
				for (
						final int len = value_spec.length();
						(i < len) && (value_spec.charAt(i) == ' ');
						++i) {
					/* nop */
				}
				value_spec = value_spec.substring(i);
			}
		}
		if (isBase64) {
			value_spec = new String(Base64.decode(value_spec));
		}
		map.put(key, value_spec);
		buf.setLength(0);
	}

	public static void main(final String[] args) throws Exception {
		for (
				final Iterator iter = parse(new BufferedReader(new FileReader(new File(args[0])))).entrySet().iterator();
				iter.hasNext();
				/* nop */) {
			final Map.Entry entry = (Map.Entry)iter.next();
			System.out.println("---------------------------------------------");
			System.out.println("key");
			System.out.println("-----");
			final Object key = entry.getKey();
			System.out.println( (key == null) ? "(null)" : ("\"" + key.toString() + "\"") );
			System.out.println("---------------------------------------------");
			System.out.println("value");
			System.out.println("-----");
			final Object value = entry.getValue();
			System.out.println( (value == null) ? "(null)" : ("\"" + value.toString() + "\"") );
			System.out.println("---------------------------------------------");
		}
	}
}
