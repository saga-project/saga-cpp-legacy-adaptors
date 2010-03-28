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
 * $RCS_file$ $Revision: 1.10 $ $Date: 2005/10/25 10:48:23 $
 */
//package diag;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

import org.gridforum.gridrpc.GrpcException;
import org.gridforum.gridrpc.GrpcFunctionHandle;
import org.gridforum.gridrpc.GrpcClient;
import org.gridforum.gridrpc.GrpcClientFactory;

/**
 * Diagnostic program for Ninf-G
 * This program checks file type parameter.
 */
public class FileTest {
	static final String modulename = "file/filename_test";
	static final String workdir_name = "tmpDirForFileTest";

	/* target files */
	static String pathToFiles = ".";
	static String text_in;
	static String text_inout;
	static String text_out;
	static String bin_in;
	static String bin_inout;
	static String bin_out;
	static String in_file;
	static String inout_file;
	static String out_file;
	static String in_orig_file;
	static String inout_orig_file;
	static String out_orig_file;
	static {
		setTargetFilename();
	}

	/* for test environment */
	static final int	TEXT_FILE = 0;
	static final int	BIN_FILE = 1;
	static final int	WITH_OUTFILE = 1;
	static final int	WITHOUT_OUTFILE = 0;

	public static void main (String args[])
	{
		/* check arguments */
		if (args.length < 1) {
			System.err.println ("must specify name of configfile.");
			System.exit(1);
		} else if (new File(args[0]).exists() != true) {
			System.err.println ("specified configfile( " +
			args[0] + " ) does not exist.");
			System.exit(1);
		}
		
		/* get path of work directory */
		if (args.length > 1) {
			pathToFiles = args[1];
			setTargetFilename();
		}
		
		/* check if work directory does already exist */
		File workdir = new File (workdir_name);
		if (workdir.exists() == true) {
			System.err.println ("Directory " + workdir_name + " exist");
			System.exit(1);
		}
		/* create work directory */
		if (workdir.mkdir() != true) {
			System.err.println ("Couldn't make " + workdir_name + "...");
			System.exit(1);
		}
		/* set files delete on exit */
		workdir.deleteOnExit();
		new File(in_file).deleteOnExit();
		new File(inout_file).deleteOnExit();
		new File(out_file).deleteOnExit();
		new File(in_orig_file).deleteOnExit();
		new File(inout_orig_file).deleteOnExit();
		new File(out_orig_file).deleteOnExit();
	
		System.out.println("=====    FileTest start    =====");
		for (int i = 0; i < 4; i++) {
			/* specify target                */
			/* 1: TEXT_FILE, WITH_OUTFILE    */
			/* 2: TEXT_FILE, WITHOUT_OUTFILE */
			/* 3: BIN_FILE, WITH_OUTFILE     */
			/* 4: BIN_FILE, WITHOUT_OUTFILE  */

			/* Test: do a same test on specified environment */
			int fileType = i / 2;
			int with_outfile = i % 2;
			
			System.out.print ("File Test (" + (i+1) +  ") ");
			/* Prepare Input files */
			try {
				if (fileType == TEXT_FILE ) {
					/* 1st : FileTest with text file */
					System.out.print ("(text, ");
					prepareTextFiles ();
				} else if (fileType == BIN_FILE) {
					/* 2nd : FileTest with binary file */
					System.out.print ("(binary, ");
					prepareBinaryFiles ();
				}
				prepareInputFiles ();
			} catch (IOException e) {
				System.err.println("Can't prepare Input files");
				e.printStackTrace();
				System.exit(1);
			}
	
			/* Prepare Output file */
			if (with_outfile == WITH_OUTFILE) {
				/* call doTest with output file */
				System.out.print ("outfile exists) : ");
				try {
					copyFile(out_orig_file, out_file);
				} catch (IOException e1) {
					System.err.println("Can't prepare Output files");
					e1.printStackTrace();
					System.exit(1);
				}
			} else if (with_outfile == WITHOUT_OUTFILE) {
				/* call doTest without output file */
				System.out.print ("no outfile exists) : ");
				if (new File(out_file).exists() == true)
				new File(out_file).delete();
			}
	
			/* exec Test! */
			if (doTest(args[0]) == true) System.out.println ("OK!");
			else                         System.out.println ("FAILED!");
			
			clearInputFiles();
		}
		
		System.out.println("===== FileTest was finished =====");
		/* Exit normally */
		System.exit(0);
	}

	/* call test module for file */
	static boolean doTest (String configFile)
	{
		GrpcClient client = null;
		GrpcFunctionHandle handle = null;
		try {
			/* Get GrpcClient object */
			client = GrpcClientFactory.getClient("org.apgrid.grpc.ng.NgGrpcClient");
			client.activate(configFile);
	
			/* call executable */
			handle = client.getFunctionHandle(modulename);
			handle.call(in_file, inout_file, out_file);

		} catch (Exception e) {
			e.printStackTrace();
			return false;
		} finally {
			try {
				if (handle != null) {
					handle.dispose();
				}
				if (client != null) {
					/* deactivate client */
					client.deactivate();
				}
			} catch (GrpcException e) {
				e.printStackTrace();
			}
		}
		/* compare result */
		if (compareResult(in_orig_file, in_file) &&
			compareResult(in_orig_file, inout_file) &&
			compareResult(inout_orig_file, out_file)) {
			return true;
		} else {
			return false;
		}
	}

	static boolean compareResult(String OrigFile, String ResultFile) {
		FileInputStream orig = null, result = null;
		boolean return_value = true;
		try {
			orig = new FileInputStream(OrigFile);
			result = new FileInputStream(ResultFile);
			int orig_value = 0, result_value = 0;
			while (true) {
				orig_value = orig.read();
				result_value = result.read();
		
				if ((orig_value == -1) && (result_value == -1)) {
					/* EOF reached!!! */
					break;
				}
				if (orig_value != result_value) {
					return_value = false;
					break;
				}
			}
			orig.close();
			result.close();
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		return return_value;
	}

	static void prepareTextFiles() throws IOException {
		copyFile (text_in, in_orig_file);
		copyFile (text_inout, inout_orig_file);
		copyFile (text_out, out_orig_file);
	}

	static void prepareBinaryFiles() throws IOException {
		copyFile (bin_in, in_orig_file);
		copyFile (bin_inout, inout_orig_file);
		copyFile (bin_out, out_orig_file);
	}

	static void prepareInputFiles() throws IOException {
		copyFile (in_orig_file, in_file);
		copyFile (inout_orig_file, inout_file);
	}

	static void copyFile (String fromFile, String toFile) throws IOException 
	{
		byte[] readBuffer = new byte [2048];
		FileInputStream fin = new FileInputStream(fromFile);
		FileOutputStream fout = new FileOutputStream(toFile);
		int nread;
		while ((nread = fin.read(readBuffer)) != -1) {
			fout.write(readBuffer, 0, nread);
		}
		fin.close();
		fout.close();
	}
	
	static void setTargetFilename() {
		text_in = pathToFiles + "/FileTest.java";
		text_inout = pathToFiles + "/client.conf";
		text_out = pathToFiles + "/DataTest.java";
		bin_in = pathToFiles + "/FileTest.class";
		bin_inout = pathToFiles + "/CancelTest.class";
		bin_out = pathToFiles + "/DataTest.class";
		in_file = workdir_name + "/in";
		inout_file = workdir_name + "/inout";
		out_file = workdir_name + "/out";
		in_orig_file = workdir_name + "/in.orig";
		inout_orig_file = workdir_name + "/inout.orig";
		out_orig_file = workdir_name + "/out.orig";
	}
	
	static void clearInputFiles() {
		if (! new File(in_file).delete()) {
			System.err.println ("failed to delete " + in_file + ".");
		} else if (! new File(inout_file).delete()) {
			System.err.println ("failed to delete " + inout_file + ".");
		} else if (! new File(out_file).delete()) {
			System.err.println ("failed to delete " + out_file + ".");
		} else if (! new File(in_orig_file).delete()) {
			System.err.println ("failed to delete " + in_orig_file + ".");
		} else if (! new File(inout_orig_file).delete()) {
			System.err.println ("failed to delete " + inout_orig_file + ".");
		} else if (! new File(out_orig_file).delete()) {
			System.err.println ("failed to delete " + out_orig_file + ".");
		}
	}
}
