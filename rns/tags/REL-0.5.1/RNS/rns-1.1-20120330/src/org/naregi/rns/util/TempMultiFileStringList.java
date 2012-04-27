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

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.AbstractList;
import java.util.ArrayList;
import java.util.List;

/**
 * Array List (for many big Strings) which is stored into many files.
 */
public class TempMultiFileStringList extends AbstractList<String> {

	private List<String> list;
	private File dir;
	private String prefix;

	/**
	 * This call is equivalent to that of calling
	 * {@link TempMultiFileStringList#TempMultiFileStringList(String, String)
	 * TempMultiFileStringList(null, "tmp")}.
	 */
	public TempMultiFileStringList() {
		this(null, "tmp");
	}

	/**
	 * Initialize List for many big String array.
	 *
	 * @param tmpdir a temporary directory. Using default temporary directory if
	 *            this is null.
	 * @param prefix a prefix of the name for the temporary file
	 */
	public TempMultiFileStringList(String tmpdir, String prefix) {
		this.prefix = prefix;
		list = new ArrayList<String>();
		if (tmpdir == null) {
			tmpdir = System.getProperty("java.io.tmpdir");
		}
		dir = new File(tmpdir);
		if (dir.exists() == false) {
			if (dir.mkdir() == false) {
				dir = new File(System.getProperty("java.io.tmpdir"));
			}
		} else if (dir.isDirectory() == false) {
			dir = new File(System.getProperty("java.io.tmpdir"));
		}
	}

	@Override
	protected void finalize() {
		//System.out.println("finalize TempMultiFileStringList: " + this.hashCode());
		for (String name : list) {
			File f = new File(dir, name);
			if (f.exists() && f.delete() == false) {
				System.err.println("cannot delete a temp File:"
						+ f.getAbsolutePath());
			}
		}
		try {
			super.finalize();
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}

	@Override
	public boolean add(String value) {
		File f;
		try {
			f = File.createTempFile(prefix, ".tmp", dir);
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		f.deleteOnExit();
		if (f.setReadable(false, false) == false
				|| f.setReadable(true, true) == false
				|| f.setWritable(false, false) == false
				|| f.setWritable(true, true) == false) {
			return false;
		}

		FileWriter fw = null;
		try {
			fw = new FileWriter(f);
			fw.append(value);
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		} finally {
			if (fw != null) {
				try {
					fw.close();
				} catch (IOException e) {
				}
			}
		}
		list.add(f.getName());
		return true;
	}

	private static final String LS = System.getProperty("line.separator");

	@Override
	public String get(int i) {
		String name = list.get(i);
		if (name == null) {
			return null;
		}
		StringBuilder sb = new StringBuilder();
		BufferedReader br = null;
		try {
			br = new BufferedReader(new FileReader(new File(dir, name)), 2048);
			String s;
			while ((s = br.readLine()) != null) {
				sb.append(s + LS);
			}
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		} finally {
			if (br != null) {
				try {
					br.close();
				} catch (IOException e) {
				}
			}
		}
		return sb.toString();
	}

	@Override
	public int size() {
		return list.size();
	}
}
