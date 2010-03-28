/*
 * $RCSfile: ExtModuleNotifyParser.java,v $ $Revision: 1.2 $ $Date: 2008/02/01 06:29:27 $
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

import java.io.LineNumberReader;
import java.io.IOException;
import java.util.List;
import java.util.ArrayList;

/*
 * External module notify parameter parser.
 * 
 */
class ExtModuleNotifyParser {

	private ExtModuleNotifyParser() {}

	/**
	 * Parses the multi line notify
	 * 
	 * @param a reader for notify
	 * @param a notify name
	 * @throws NullPointerException if the given Reader is null.
	 * @throws IllegalArgumentException if the given String is null
	 */
	public static List<NgAttribute> parse(LineNumberReader reader,
	                                      String notifyName)
	throws IOException {
		if (reader == null) 
			throw new NullPointerException();
		if ( (notifyName == null) || (notifyName.length() == 0))
			throw new IllegalArgumentException();

		List<NgAttribute> list = new ArrayList<NgAttribute>();
		String line;
		String last_line = notifyName + "_END";
        for (;;) {
            line = reader.readLine();
            if (line == null)
                throw new IOException("read unexpected null");
            if (line.equals( last_line ))
                break;
            if (line.length() == 0)
                continue;

            int i = line.indexOf(" ");
            String name = line.substring(0, i);
            String val  = line.substring(i + 1);
            list.add(new NgAttribute(name, val));
        }
		return list;
	}

	/**
	 * Parses the single line notify
	 * 
	 * @param String for notify
	 * @throws NullPointerException if the specified String is null.
	 */
	public static List<String> parse(String line)
	throws IOException {
		if (line == null)
			throw new NullPointerException();

		List<String> list = new ArrayList<String>();

		int ios = 0, next = 0;
		int vlen = line.length();
		String val;
		for (;;) {
			ios = line.indexOf(" ", next);
			if ((ios < 0) && (next < vlen)) {
				val = line.substring(next);
				list.add(val);
				break;
			}
			val = line.substring(next, ios);
			list.add(val);
			next = ios + 1;
			if (next >= vlen)
				break;
        }
		return list;
	}

}

