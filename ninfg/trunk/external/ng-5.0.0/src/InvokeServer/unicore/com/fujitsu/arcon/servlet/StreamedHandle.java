package com.fujitsu.arcon.servlet;

import org.unicore.*;
import org.unicore.ajo.AbstractAction;
import org.unicore.ajo.Portfolio;

import java.io.File;
import java.util.*;

/**
 * 
 * A handle on the files streamed back for an AJO
 * 
 * @author Sven van den Berghe, fujitsu
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 *  
 */
public class StreamedHandle {

	private Map ids_to_files;

	public StreamedHandle(Map ids_to_files) {
		this.ids_to_files = ids_to_files;
	}

	/**
	 * List all the files in the portfolio that were created as a result of
	 * executing this task (and streamed back)
	 *  
	 */
	public File[] getFiles(AbstractAction task, Portfolio portfolio) {

		Integer task_id = new Integer(task.getId().getValue());

		Collection task_files = (Collection) ids_to_files.get(task_id);

		if (task_files == null) {
			return new File[0];
		}

		Set collected = new HashSet();

		// Got some files, look for the ones in the requested Portfolio

		Iterator i = task_files.iterator();
		while (i.hasNext()) {
			File f = (File) i.next();
			if (f.getPath().indexOf(portfolio.getUPLDirectoryName()) > 0) {
				collected.add(f);
			}
		}

		return (File[]) collected.toArray(new File[collected.size()]);

	}

	public File[] getFiles(AbstractAction task, PortfolioIdentifier pfid) {
		return getFiles(task, new Portfolio(pfid.getName(), pfid.getValue()));
	}

	/**
	 * List all files from the AJO
	 *  
	 */
	public File[] getFiles() {

		// Combine the collections in the Map into one

		Set all = new HashSet();

		Iterator i = ids_to_files.values().iterator();
		while (i.hasNext()) {
			all.addAll((Collection) i.next());
		}

		// Convert to an array
		return (File[]) all.toArray(new File[1]);
	}

}
//
//                   Copyright (c) Fujitsu Ltd 2000 - 2004
//
//                Use and distribution is subject a License.
// A copy was supplied with the distribution (see documentation or the jar
// file).
//
// This product includes software developed by Fujitsu Limited
// (http://www.fujitsu.com).
