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

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.AbstractList;

import org.naregi.rns.RNSLog;

/**
 * Array List (for Strings) which is stored into a file in a temporary
 * directory.
 */
public class TempOneFileStringList extends AbstractList<String> {
	private File f;
	private RandomAccessFile raf = null;

	private int index = 0; /* cursor */
	private int num = 0; /* total */

	private long time;

	/**
	 * This call is equivalent to that of calling
	 * {@link TempOneFileStringList#TempOneFileStringList(String, String)
	 * TempOneFileStringList(null, "tmp")}.
	 */
	public TempOneFileStringList() throws IOException {
		this(null, "tmp");
	}

	/**
	 * Initialize List for String array.
	 *
	 * @param tmpdir a temporary directory. Using default temporary directory if
	 *            this is null.
	 * @param prefix a prefix of the name for the temporary file
	 * @throws IOException
	 */
	public TempOneFileStringList(String tmpdir, String prefix)
			throws IOException {
		if (tmpdir == null) {
			tmpdir = System.getProperty("java.io.tmpdir");
		}
		File dir = new File(tmpdir);
		if (dir.exists() == false) {
			if (dir.mkdir() == false) {
				dir = new File(System.getProperty("java.io.tmpdir"));
			}
		} else if (dir.isDirectory() == false) {
			dir = new File(System.getProperty("java.io.tmpdir"));
		}
		f = File.createTempFile(prefix, ".tmp", dir);
		f.deleteOnExit();
		if (f.setReadable(false, false) == false
				|| f.setReadable(true, true) == false
				|| f.setWritable(false, false) == false
				|| f.setWritable(true, true) == false) {
			throw new IOException("cannot read/write: " + f.getAbsolutePath());
		}
		raf = new RandomAccessFile(f, "rw");
		time = System.currentTimeMillis();
		CloseThread ct = getCloseThread(); /* global Thread */
		ct.add(this);
	}

	/* ----------------------------------------- */

	private static int expireTime = 300000; /* second */
	private static int ARRAY_SIZE = 100; /* manage opened files */

	private static TempOneFileStringList[] array = new TempOneFileStringList[ARRAY_SIZE];
	private static int arrayIndex = 0;

	private static CloseThread closeThread = null;

	private synchronized static CloseThread getCloseThread() {
		if (closeThread != null) {
			return closeThread;
		}
		closeThread = new CloseThread();
		closeThread.start();
		return closeThread;
	}

	private static class CloseThread extends Thread {
		public CloseThread() {
			for (int i = 0; i < ARRAY_SIZE; i++) {
				array[i] = null;
			}
		}

		public void run() {
			while (true) {
				for (int i = 0; i < ARRAY_SIZE; i++) {
					try {
						Thread.sleep(60000); /* 60 sec. */
					} catch (InterruptedException e) {
						/* ignore */
					}
					synchronized (array) {
						TempOneFileStringList ent = array[i];
						if (ent != null) {
							synchronized (ent) {
								if (System.currentTimeMillis() > ent.time
										+ expireTime) {
									ent.close();
									array[i] = null; /* for GC */
								}
							}
						}
					}
				}
			}
		}

		public void add(TempOneFileStringList tofsl) {
			synchronized (array) {
				if (array[arrayIndex] != null) {
					array[arrayIndex].close();
					/* replace */
				}
				array[arrayIndex] = tofsl;
				arrayIndex++;
				if (arrayIndex >= ARRAY_SIZE) {
					arrayIndex = 0;
				}
			}
		}
	}

	/* ----------------------------------------- */

	private void close() {
		/* This 'synchronized' protects 'raf' */
		synchronized (this) {
			try {
				raf.close();
			} catch (IOException e) {
				e.printStackTrace();
				RNSLog.getLog().error(
						"unexpected: close error: " + f.getAbsolutePath());
			}
			raf = null;
		}
	}

	@Override
	protected void finalize() {
		try {
			if (raf != null) {
				raf.close();
			}
			if (f.delete() == false) {
				RNSLog.getLog().error(
						"unexpected: cannot delete: " + f.getAbsolutePath());
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		try {
			super.finalize();
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}

	private void reopen() {
		try {
			raf = new RandomAccessFile(f, "rw");
			index = 0;
			time = System.currentTimeMillis();
			CloseThread ct = getCloseThread();
			ct.add(this);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
			RNSLog.getLog().error(
					"unexpected: file not found: " + f.getAbsolutePath());
			throw new RuntimeException(e);
		}
	}

	@Override
	public boolean add(String value) {
		synchronized (this) {
			if (raf == null) {
				reopen();
			} else {
				time = System.currentTimeMillis(); /* update */
			}
			try {
				if (index != num) {
					raf.seek(raf.length());
					index = num;
				}
				raf.writeUTF(value);
				index++;
				num++;
				return true;
			} catch (IOException e) {
				e.printStackTrace();
				RNSLog.getLog().error(
						"unexpected: write error: " + f.getAbsolutePath());
				return false;
			}
		}
	}

	@Override
	public String get(int i) {
		synchronized (this) {
			if (raf == null) {
				reopen();
			} else {
				time = System.currentTimeMillis(); /* update */
			}
			try {
				if (i < 0) {
					return null;
				} else if (i >= num) {
					return null;
				} else if (i != index) {
					raf.seek(0);
					for (int j = 0; j < i; j++) {
						raf.readUTF(); /* skip */
					}
					index = i;
				}
				String res = raf.readUTF();
				index++;
				return res;
			} catch (IOException e) {
				e.printStackTrace();
				RNSLog.getLog().error(
						"unexpected: read error: " + f.getAbsolutePath());
				return null;
			}
		}
	}

	@Override
	public int size() {
		return num;
	}
}
