package com.fujitsu.arcon.servlet;

import org.unicore.ajo.*;
import org.unicore.outcome.*;
import org.unicore.sets.*;

import java.io.*;
import java.util.*;

/**
 * Manage Vsites.
 * 
 * @author Sven van den Berghe
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 *  
 */
public class VsiteManager {

	/**
	 * Methods that must be supported by instances that wish to be informed of
	 * changes to the loaded Vsites.
	 *  
	 */
	public static interface Listener {

		/**
		 * A Vsite's resources have been updated.
		 * 
		 * @param vsite
		 *            The Updated Vsite
		 * @param r
		 *            The new Resource set
		 */
		public void newResourceSet(VsiteTh vsite, ResourceSet r);

		/**
		 * The Vsite list may have changed.
		 * 
		 * @param i
		 *            An Iteration over the current Vsite list (VsiteTh
		 *            instances)
		 */
		public void newVsiteList(Iterator i);

	}

	public static class Exception extends java.lang.Exception {
		public Exception(String message) {
			super(message);
		}
	}

	private static Set listeners = new HashSet();

	/**
	 * Add an object that wants to be informed of changes to the Vsites.
	 *  
	 */
	public static void addListener(Listener l) {
		listeners.add(l);
	}

	private static Set loaded_gateways = new HashSet();

	/**
	 * Initialise Vsites from a file.
	 * 
	 * Each line of this file starts with a Gateway address
	 * (ssl://machinename:port_number) and is followed by a list of (space
	 * separated) Vsite names
	 * 
	 * e.g. ssl://arcon.fle.fujitsu.com:4433 vsite1 vsite2 vsite3
	 * 
	 * Informs all registered listeners of the the new Vsites.
	 *  
	 */
	public static void initialise(File init_file, Identity current_identity)
			throws VsiteManager.Exception {

		CLogger.status("Reading Vsites from <" + init_file + ">");

		try {
			BufferedReader br = new BufferedReader(new FileReader(init_file));
			while (true) {
				String next_gw = br.readLine();
				if (next_gw == null)
					break;
				StringTokenizer st = new StringTokenizer(next_gw);
				if (st.hasMoreTokens()) { // safely skip blank lines
					Reference.SSL ref = new Reference.SSL(st.nextToken(),
							current_identity);
					Gateway new_gw = new Gateway(ref);
					addGateway(new_gw);
					while (st.hasMoreTokens()) { // get any Vsites
						VsiteTh vsite = new VsiteTh(ref, st.nextToken());
						new_gw.addVsite(vsite);

					}
				}
			}
		} catch (EOFException eofex) {
		} // OK way out of loop
		catch (java.lang.Exception ex) {
			throw new VsiteManager.Exception(
					"Problems reading Gateway init file: " + ex.getMessage());
		}

		Iterator ii = listeners.iterator();
		while (ii.hasNext()) {
			((Listener) ii.next()).newVsiteList(getVsites());
		}
	}

	public static void addGateway(Gateway gateway) {
		loaded_gateways.add(gateway);
	}

	public static Iterator getGateways() {
		return loaded_gateways.iterator();
	}

	/**
	 * @return An iterator over all the Vsites known (from all Gateways).
	 *  
	 */
	public static Iterator getVsites() {
		HashSet all_of_em = new HashSet();
		Iterator i = loaded_gateways.iterator();
		while (i.hasNext()) {
			all_of_em.addAll(((Gateway) i.next()).getVsites());
		}

		return all_of_em.iterator();
	}

	/**
	 * @return An iterator over all the Ports known (from all Gateways).
	 *  
	 */
	public static Iterator getPorts() {
		HashSet all_of_em = new HashSet();
		Iterator i = loaded_gateways.iterator();
		while (i.hasNext()) {
			all_of_em.addAll(((Gateway) i.next()).getPorts());
		}

		return all_of_em.iterator();
	}

	/**
	 * Get a list of Vsites and Ports from the Gateway.
	 * 
	 * If this is successful, then the Gateway is added to the list of Gateways
	 * (not if unsuccessful - but log to user) with the Vsites from the Gateway
	 * as VsiteTh instances.
	 * 
	 * Any instance of this Gateway already loaded will be updated with the
	 * values obtained from the Gateway.
	 * 
	 * Before calling this must be properly initialised to create SSL
	 * connections (Identity set up).
	 *  
	 */
	public static void doListVsites(Gateway gateway)
			throws VsiteManager.Exception {

		try {
			JobManager.listVsites(gateway);
		} catch (JobManager.Exception jex) {
			throw new VsiteManager.Exception(jex.getMessage());
		} catch (Connection.Exception cex) {
			throw new VsiteManager.Exception(cex.getMessage());
		}

		Iterator ii = listeners.iterator();
		while (ii.hasNext()) {
			((Listener) ii.next()).newVsiteList(getVsites());
		}
	}

	/**
	 * Update the resources a current Vsite. All registered listeners will be
	 * informed that it is possible that a Vite's resources may have changed.
	 * 
	 * This runs a job on the NJS so must have completed initialisation fot it
	 * to execute successfully (Identity loaded etc).
	 * 
	 * @param vsite
	 *            The Vsite to update
	 * 
	 * @throws VsiteManager.Exception
	 */
	public static void getResources(VsiteTh vsite)
			throws VsiteManager.Exception {

		// Create the AJO
		GetResourceDescription get_resources = new GetResourceDescription(
				"Auto-GetResourceDescription");

		// Execute it
		OutcomeTh tho;
		try {
			tho = JobManager.executeAction(get_resources, vsite);
		} catch (JobManager.Exception jex) {
			throw new VsiteManager.Exception(jex.getMessage());
		} catch (Connection.Exception cex) {
			throw new VsiteManager.Exception(cex.getMessage());
		}
		if (tho.getOutcome().getStatus().isEquivalent(
				org.unicore.outcome.AbstractActionStatus.NOT_SUCCESSFUL)) {
			throw new VsiteManager.Exception("Problems getting resources.");
		}

		GetResourceDescription_Outcome grd_outcome = (GetResourceDescription_Outcome) tho
				.getActionOutcome(get_resources.getId());

		// Process the Outcome, placing successful results
		// into the VsiteTh and writing to UI
		if (grd_outcome != null) {

			CLogger.status("Resource description successfully fetched");

			Iterator i = listeners.iterator();
			while (i.hasNext()) {
				((Listener) i.next()).newResourceSet(vsite, grd_outcome
						.getResources());
			}

			vsite.setResources(grd_outcome.getResources());

		} else {
			throw new VsiteManager.Exception(
					"Problems fetching Resource description: "
							+ get_resources.getId().getValue());
		}
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
