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
 * $RCS_file$ $Revision: 1.4 $ $Date: 2005/10/25 10:48:23 $
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
 * This program checks array of file type parameter.
 */
public class FileArrayTest extends FileTest {
	static final String modulename = "file/filename_array_test";
	/* number of array */
	static final int numOfArray = 4;

	/* target files */
	static String in_file_array[] = new String[numOfArray];
	static String inout_file_array[] = new String[numOfArray];
	static String out_file_array[] = new String[numOfArray];

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
		for (int i = 0; i < numOfArray; i++) {
			new File(in_file_array[i]).deleteOnExit();
			new File(inout_file_array[i]).deleteOnExit();
			new File(out_file_array[i]).deleteOnExit();
		}
		new File(in_orig_file).deleteOnExit();
		new File(inout_orig_file).deleteOnExit();
		new File(out_orig_file).deleteOnExit();
	
		System.out.println("=====    FileArrayTest start    =====");
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
					/* 1st : FileArrayTest with text file */
					System.out.print ("(text, ");
					prepareTextFiles ();
				} else if (fileType == BIN_FILE) {
					/* 2nd : FileArrayTest with binary file */
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
					for (int j = 0; j < numOfArray; j++) {
						copyFile(out_orig_file, out_file_array[j]);
					}
				} catch (IOException e1) {
					System.err.println("Can't prepare Output files");
					e1.printStackTrace();
					System.exit(1);
				}
			} else if (with_outfile == WITHOUT_OUTFILE) {
				/* call doTest without output file */
				System.out.print ("no outfile exists) : ");
				for (int j = 0; j < numOfArray; j++) {
					if (new File(out_file_array[j]).exists() == true)
						new File(out_file_array[j]).delete();
				}
			}
	
			/* exec Test! */
			if (doTest(args[0]) == true) System.out.println ("OK!");
			else                         System.out.println ("FAILED!");
			
			clearInputFiles();
		}
		
		System.out.println("===== FileArrayTest was finished =====");
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
			handle.call(new Integer(numOfArray), in_file_array, inout_file_array, out_file_array);

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
			for (int i = 0; i < numOfArray; i++) {
				orig = new FileInputStream(OrigFile);
				result = new FileInputStream(ResultFile + "-" + i);
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
			}
		} catch (IOException e) {
			e.printStackTrace();
			return false;
		}
		return return_value;
	}

	static void prepareInputFiles() throws IOException {
		copyFiles (in_orig_file, in_file);
		copyFiles (inout_orig_file, inout_file);
	}

	static void copyFiles (String fromFile, String toFile) throws IOException 
	{
		byte[] readBuffer = new byte [2048];
		
		for (int i = 0; i < numOfArray; i++) {
			FileInputStream fin = new FileInputStream(fromFile);
			FileOutputStream fout = new FileOutputStream(toFile + "-" + i);
			int nread;
			while ((nread = fin.read(readBuffer)) != -1) {
				fout.write(readBuffer, 0, nread);
			}
			fin.close();
			fout.close();
		}
	}
	
	static void setTargetFilename() {
		FileTest.setTargetFilename();
		for (int i = 0; i < numOfArray; i++) {
			in_file_array[i] = workdir_name + "/in-" + i;
			inout_file_array[i] = workdir_name + "/inout-" + i;
			out_file_array[i] = workdir_name + "/out-" + i;
		}
	}
	
	static void clearInputFiles() {
		for (int i = 0; i < numOfArray; i++) {
			if ((new File(in_file_array[i]).exists()) &&
				(! new File(in_file_array[i]).delete())) {
				System.err.println ("failed to delete " + in_file_array[i] + ".");
			} else if ((new File(inout_file_array[i]).exists()) &&
				(! new File(inout_file_array[i]).delete())) {
				System.err.println ("failed to delete " + inout_file_array[i] + ".");
			} else if ((new File(out_file_array[i]).exists()) &&
				(! new File(out_file_array[i]).delete())) {
				System.err.println ("failed to delete " + out_file_array[i] + ".");
			}
		}
		if (! new File(in_orig_file).delete()) {
			System.err.println ("failed to delete " + in_orig_file + ".");
		} else if (! new File(inout_orig_file).delete()) {
			System.err.println ("failed to delete " + inout_orig_file + ".");
		} else if (! new File(out_orig_file).delete()) {
			System.err.println ("failed to delete " + out_orig_file + ".");
		}
	}
}
