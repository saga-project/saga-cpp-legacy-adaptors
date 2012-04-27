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
package org.naregi.rns.client;

/**
 * RNS errors.
 */
public class RNSError extends Exception {

	private static final long serialVersionUID = -7882183259786507356L;

	/**
	 * Error types and messages.
	 */
	public enum Errno {
		/**
		 * "Permission denied"
		 */
		EACCES("Permission denied"),
		/**
		 * "File exists"
		 */
		EEXIST("File exists"),
		/**
		 * "No such junction or directory"
		 */
		ENOENT("No such junction or directory"),
		/**
		 * "Not a directory"
		 */
		ENOTDIR("Not a directory"),
		/**
		 * "Network error"
		 */
		ENET("Network error"),
		/**
		 * "Invalid argument"
		 */
		EINVAL("Invalid argument"),
		/**
		 * "Is a directory"
		 */
		EISDIR("Is a directory"),
		/**
		 * "Device or resource busy"
		 */
		EBUSY("Device or resource busy"),
		/**
		 * "Directory not empty"
		 */
		ENOTEMPTY("Directory not empty"),
		/**
		 * "Too many levels of references"
		 */
		ELOOP("Too many levels of references"),
		/**
		 * "Unexpected error"
		 */
		EUNEXPECTED("Unexpected error"),
		/**
		 * "Operation not permitted"
		 */
		EPERM("Operation not permitted"),
		/**
		 * "Operation not supported"
		 */
		ENOTSUPP("Operation not supported");

		private String message;

		private Errno(String message) {
			this.message = message;
		}

		public String toString() {
			return message;
		}
	};

	private Errno errno;
	private String path;

	private RNSError(Errno n, String path, String message, Throwable cause) {
		super(message, cause);
		errno = n;
		this.path = path;
	}

	public String getMessage() {
		/* message: path: errno message */
		String msg = errno.toString();
		String superMsg = super.getMessage();
		if (path != null && !path.equals("")) {
			msg = path + ": " + msg;
		}
		if (superMsg != null && !superMsg.equals("")) {
			msg = superMsg + ": " + msg;
		}
		return msg;
	}

	/**
	 * Get an error.
	 *
	 * @return Errno
	 */
	public Errno getError() {
		return errno;
	}

	/**
	 * Create EACCES error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createEACCES(String path, String msg) {
		return new RNSError(Errno.EACCES, path, msg, null);
	}

	/**
	 * Create EEXIST error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createEEXIST(String path, String msg) {
		return new RNSError(Errno.EEXIST, path, msg, null);
	}

	/**
	 * Create ENOENT error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createENOENT(String path, String msg) {
		return new RNSError(Errno.ENOENT, path, msg, null);
	}

	/**
	 * Create ENOTDIR error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createENOTDIR(String path, String msg) {
		return new RNSError(Errno.ENOTDIR, path, msg, null);
	}

	/**
	 * Create EISDIR error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createEISDIR(String path, String msg) {
		return new RNSError(Errno.EISDIR, path, msg, null);
	}

	/**
	 * Create EBUSY error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createEBUSY(String path, String msg) {
		return new RNSError(Errno.EBUSY, path, msg, null);
	}

	/**
	 * Create ENOTEMPTY error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createENOTEMPTY(String path, String msg) {
		return new RNSError(Errno.ENOTEMPTY, path, msg, null);
	}

	/**
	 * Create ELOOP error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createELOOP(String path, String msg) {
		return new RNSError(Errno.ELOOP, path, msg, null);
	}

	/**
	 * Create ENOTSUPP error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createENOTSUP(String path, String msg) {
		return new RNSError(Errno.ENOTSUPP, path, msg, null);
	}

	/**
	 * Create ENET error.
	 *
	 * @param path
	 * @param msg
	 * @param cause
	 * @return RNSError
	 */
	public static RNSError createENET(String path, String msg, Throwable cause) {
		return new RNSError(Errno.ENET, path, msg, cause);
	}

	/**
	 * Create EINVAL error.
	 *
	 * @param path
	 * @param msg
	 * @param cause
	 * @return RNSError
	 */
	public static RNSError createEINVAL(String path, String msg, Throwable cause) {
		return new RNSError(Errno.EINVAL, path, msg, cause);
	}

	/**
	 * Create EUNEXPECTED error.
	 *
	 * @param path
	 * @param msg
	 * @param cause
	 * @return RNSError
	 */
	public static RNSError createEUNEXPECTED(String path, String msg, Throwable cause) {
		return new RNSError(Errno.EUNEXPECTED, path, msg, cause);
	}

	/**
	 * Create EPERM error.
	 *
	 * @param path
	 * @param msg
	 * @return RNSError
	 */
	public static RNSError createEPERM(String path, String msg) {
		return new RNSError(Errno.EPERM, path, msg, null);
	}
}
