package com.fujitsu.arcon.servlet;

import org.unicore.*;
import org.unicore.outcome.*;

import java.util.*;

/**
 * The results of a job. This can include either the Outcome object, or handles
 * on retrieved Outcome files or both of these, depending on how the instances
 * was obtained.
 * 
 * @author Sven van den Berghe, Fujitsu Laboratories of Europe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 * 
 */
public class OutcomeTh {

	public OutcomeTh() {
		ids_to_files = new HashMap();
	}

	private AbstractJob_Outcome ajoo;

	/**
	 * @return The Job Outcome object. This includes the status of the Job and
	 *         the status of all the AbstractActions that the Job contains. It
	 *         may also include the results of the AbstractAction's execution if
	 *         these are held in the Outcome object. This maybe null if the
	 *         OutcomeTh was created by a method that does not fetch the Outcome
	 *         object.
	 *  
	 */
	public AbstractJob_Outcome getOutcome() {
		return ajoo;
	}

	public void setOutcome(AbstractJob_Outcome ajoo) {
		this.ajoo = ajoo;
	}

	/**
	 * @return The Outcome object of a particular AbstractAction (or null if
	 *         there is no Outcome object or the AbstractAction is not part of
	 *         the Job).
	 *  
	 */
	public Outcome getActionOutcome(AAIdentifier id) {
		if (ajoo == null) {
			return null;
		} else {
			return ajoo.getOutcome(id);
		}
	}

	private Map ids_to_files;

	/**
	 * @return A mapping of AbstractAction Identifiers to local copies of
	 *         Outcome files created by the execution of the AbstractAction. The
	 *         keys are instances of org.unicore.AAIdentifier, the values are
	 *         collections (java.util.Collection) of instances of java.io.File.
	 *         The Map will be empty if there are no files or if the method that
	 *         created the instance did not retrieve the Outcome files.
	 *  
	 */
	public Map getFilesMapping() {
		return ids_to_files;
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
