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
package org.naregi.rns.util;

import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;
import java.util.AbstractList;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.Inflater;

/**
 * Compressed String array.
 */
public class ZipList extends AbstractList<String> {
	private boolean enable = true;
	private List<Object> list;

	public ZipList() {
		list = new ArrayList<Object>();
	}

	public ZipList(int initialCapacity) {
		list = new ArrayList<Object>(initialCapacity);
	}

	public boolean add(String str) {
		try {
			byte[] b = str.getBytes("UTF-8");
			Deflater compresser = new Deflater();
			// compresser.setLevel(Deflater.BEST_COMPRESSION);
			compresser.setInput(b);
			compresser.finish();
			ByteArrayOutputStream baos = new ByteArrayOutputStream(b.length);
			byte[] buf = new byte[128];
			int count;
			while (compresser.finished() == false) {
				count = compresser.deflate(buf);
				baos.write(buf, 0, count);
			}
			byte[] res = baos.toByteArray();
			// System.out.println(b.length + " -> " + res.length + " (" +
			// (((double)res.length / (double)b.length) * 100) + "%)"); //
			return list.add(res);
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
			enable = false;
			return list.add(str);
		}
	}

	public String get(int i) {
		if (enable) {
			byte[] b = (byte[]) list.get(i);
			Inflater decompresser = new Inflater();
			decompresser.setInput(b);
			ByteArrayOutputStream baos = new ByteArrayOutputStream(b.length);
			byte[] buf = new byte[128];
			int count;
			while (decompresser.finished() == false) {
				try {
					count = decompresser.inflate(buf);
				} catch (DataFormatException e) {
					throw new RuntimeException(e);
				}
				baos.write(buf, 0, count);
			}
			byte[] b2 = baos.toByteArray();
			try {
				return new String(b2, 0, b2.length, "UTF-8");
			} catch (UnsupportedEncodingException e) {
				throw new RuntimeException(e);
			}
		} else {
			return (String) list.get(i);
		}
	}

	@Override
	public int size() {
		return list.size();
	}
}
