package com.fujitsu.arcon.servlet;

import org.unicore.AJOIdentifier;
import org.unicore.Vsite;
import org.unicore.ajo.AbstractAction;
import org.unicore.ajo.AbstractJob;
import org.unicore.outcome.AbstractActionStatus;
import org.unicore.outcome.AbstractJob_Outcome;
import org.unicore.outcome.Outcome;
import org.unicore.upl.ConsignJob;
import org.unicore.upl.ConsignJobReply;
import org.unicore.upl.ListVsites;
import org.unicore.upl.ListVsitesReply;
import org.unicore.upl.Reply;
import org.unicore.upl.RetrieveOutcome;
import org.unicore.upl.RetrieveOutcomeAck;
import org.unicore.upl.RetrieveOutcomeReply;
import org.unicore.upl.UnicoreResponse;
import org.unicore.utility.ConsignForm;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.NotSerializableException;
import java.io.ObjectInputStream;
import java.security.SignatureException;
import java.util.Collection;
import java.util.HashSet;
import java.util.StringTokenizer;
import java.util.zip.ZipEntry;
import java.util.zip.ZipException;
import java.util.zip.ZipInputStream;

/**
 * This class provides a thread safe JobManager class.
 * Run jobs, monitor them and fetch back results.
 * 
 * @author Sven van den Berghe, fujitsu
 * @version $Revision: 1.1 $ $Date: 2007/01/16 04:13:58 $
 * 
 */
public class TsJobManager {

	private boolean always_poll;

	private File outcome_dir;

	private int fetchedDataSize;

	private int exitcode;

	private int exitsignal;

	private String err;

	public TsJobManager() {
		fetchedDataSize = 0;
		exitcode = 0;
		exitsignal = 0;
		err = null;
	}

	/**
	 * @param ajo
	 * @param portfolios
	 * @exception TsJobManager.Exception
	 */
	private void addPortfolios(AbstractJob ajo, PortfolioTh[] portfolios)
			throws TsJobManager.Exception {

		if (portfolios != null) {
			// Add the Portfolios to be transferred to the AJO (not necessary
			// but best for completeness), while we are at it check that the
			// files
			// are OK
			for (int i = 0; i < portfolios.length; i++) {

				File[] files = portfolios[i].getFiles();
				for (int j = 0; j < files.length; j++) {
					if (!files[j].canRead()) {
						// V4 UPL change, directories allowed to mark
						// no-overwrite Portfolios, these will not exist
						if (!files[j].getName().equals("NO_OVERWRITE")) {
							throw new TsJobManager.Exception(
									"Cannot read file: " + files[j]);
						}
					}
				}
				ajo.addStreamed(portfolios[i].getPortfolio());
			}
		}
	}

	/**
	 * @return boolean
	 */
	boolean alwaysPoll() {
		return always_poll;
	}

	/**
	 * @param reply
	 * @exception TsJobManager.Exception
	 */
	private void checkUPLReply(Reply reply) throws TsJobManager.Exception {

		UnicoreResponse[] ur = reply.getTrace();

		if (ur[ur.length - 1].getReturnCode() == -2) {
			String message = "TRY AGAIN: " + ur[ur.length - 1].getComment();
			throw new TsJobManager.Exception(message);
		} else if (ur[ur.length - 1].getReturnCode() != 0) {
			String message = "UPL Reply reports an error: "
					+ ur[ur.length - 1].getReturnCode() + ":"
					+ ur[ur.length - 1].getComment();
			throw new TsJobManager.Exception(message);
		}
	}

	/**
	 * Execute a job (AJO) on a Vsite asynchronously (not waiting for the
	 * result). Poll for job completion using {@link #getOutcome}.
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param portfolios
	 *            The files to send with the execute job request
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public void consignAsynchronous(AbstractJob ajo, PortfolioTh[] portfolios,
			VsiteTh vsite) throws TsJobManager.Exception, Connection.Exception {

		addPortfolios(ajo, portfolios);
		ConsignJob to_consign = makeConsignJob(ajo, false, vsite);
		Connection c = Connection.getConnection(vsite.getReference());

		try {
			Reply from_consign = c.send(to_consign, portfolios);
			checkUPLReply(from_consign);
		} finally {
			c.done();
		}
	}

	/**
	 * Execute a job (AJO) on a Vsite asynchronously (not waiting for the
	 * result). Poll for job completion using {@link #getOutcome}. new
	 * parameter added to select rewriting AJO-ID or not.
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param portfolio
	 *            The files to send with the execute job request
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @param change_ajoid
	 *            change AJO-ID or not.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public void consignAsynchronous(AbstractJob ajo, PortfolioTh[] portfolios,
			VsiteTh vsite, boolean change_ajoid) throws TsJobManager.Exception,
			Connection.Exception {

		addPortfolios(ajo, portfolios);
		ConsignJob to_consign = makeConsignJob(ajo, false, vsite, change_ajoid);
		Connection c = Connection.getConnection(vsite.getReference());

		try {
			Reply from_consign = c.send(to_consign, portfolios);
			checkUPLReply(from_consign);
		} finally {
			c.done();
		}
	}

	/**
	 * Execute a job (AJO) on a Vsite asynchronously (not waiting for the
	 * result). Poll for job completion using {@link #getOutcome}.
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public void consignAsynchronous(AbstractJob ajo, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {
		consignAsynchronous(ajo, null, vsite);
	}

	/**
	 * Execute a job (AJO) on a Vsite asynchronously (not waiting for the
	 * result). Poll for job completion using {@link #getOutcome}. new
	 * parameter added to select rewriting AJO-ID or not.
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @param change_ajoid
	 *            change AJO-ID or not.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public void consignAsynchronous(AbstractJob ajo, VsiteTh vsite,
			boolean change_ajoid) throws TsJobManager.Exception,
			Connection.Exception {
		consignAsynchronous(ajo, null, vsite);
	}

	/**
	 * Execute a job (AJO) on a Vsite synchronously (waiting for the result).
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param portfolio
	 *            The files to send with the execute job request
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @return OutcomeTh A handle to the results of executing the job.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public OutcomeTh consignSynchronous(AbstractJob ajo,
			PortfolioTh[] portfolios, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {

		addPortfolios(ajo, portfolios);

		ConsignJob to_consign = makeConsignJob(ajo, true, vsite);

		Connection c = Connection.getConnection(vsite.getReference());

		try {
			Reply reply = c.send(to_consign, portfolios);
			checkUPLReply(reply);
			OutcomeTh oth = handleConignSynchronousReply(reply, ajo, c, vsite);
			return oth;
		} finally {
			c.done();
		}
	}

	/**
	 * Execute a job (AJO) on a Vsite synchronously (waiting for the result).
	 * new parameter added to select rewriting AJO-ID or not.
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param portfolio
	 *            The files to send with the execute job request
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @param change_ajoid
	 *            change AJO-ID or not.
	 * @return OutcomeTh A handle to the results of executing the job.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public OutcomeTh consignSynchronous(AbstractJob ajo,
			PortfolioTh[] portfolios, VsiteTh vsite, boolean change_ajoid)
			throws TsJobManager.Exception, Connection.Exception {

		addPortfolios(ajo, portfolios);

		ConsignJob to_consign = makeConsignJob(ajo, true, vsite, change_ajoid);

		Connection c = Connection.getConnection(vsite.getReference());

		try {
			Reply reply = c.send(to_consign, portfolios);
			checkUPLReply(reply);
			OutcomeTh oth = handleConignSynchronousReply(reply, ajo, c, vsite);
			return oth;
		} finally {
			c.done();
		}
	}

	/**
	 * Execute a job (AJO) on a Vsite synchronously (waiting for the result).
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @return OutcomeTh A handle to the results of executing the job.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public OutcomeTh consignSynchronous(AbstractJob ajo, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {
		return consignSynchronous(ajo, null, vsite);
	}

	/**
	 * Execute a job (AJO) on a Vsite synchronously (waiting for the result).
	 * added new parameter to select rewriting AJO-ID or not.
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @param change_ajoid
	 *            change AJO-ID ot not.
	 * @return OutcomeTh A handle to the results of executing the job.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public OutcomeTh consignSynchronous(AbstractJob ajo, VsiteTh vsite,
			boolean change_ajoid) throws TsJobManager.Exception,
			Connection.Exception {
		return consignSynchronous(ajo, null, vsite, change_ajoid);
	}

	/**
	 * Wrap the AbstractAction in an AJO and consign it to the Vsite -
	 * synchronously.
	 * 
	 * @param action
	 * @param vsite
	 * @return OutcomeTh
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public OutcomeTh executeAction(AbstractAction action, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {

		AbstractJob ajo = new AbstractJob("AJO wrapper for " + action.getName());
		ajo.add(action);

		OutcomeTh tho = consignSynchronous(ajo, vsite);
		return tho;
	}

	/**
	 * @param ajo_id
	 * @param vsite
	 * @return OutcomeTh
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	private OutcomeTh getActualOutcome(AJOIdentifier ajo_id, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {

		if (vsite == null) {
			throw new TsJobManager.Exception("Vsite is null.");
		}

		RetrieveOutcome ro = new RetrieveOutcome(ajo_id, new Vsite(vsite
				.getReference().getAddress(), vsite.getName()));

		Connection c = Connection.getConnection(vsite.getReference());

		try {
			Reply from_ro = c.send(ro);
			checkUPLReply(from_ro);
			OutcomeTh result = processROR((RetrieveOutcomeReply) from_ro, c);
			return result;
		} finally {
			c.done();
		}
	}

	/**
	 * Get a snapshot of the results of a job executing on a Vsite. This can be
	 * called until the job has completed execution. The first call that detects
	 * a complete job wil fetch the complete results and then clean the job from
	 * the Vsite. Subsequent calls will, therefore, fail as the job no longer
	 * exists on the Vsite.
	 * 
	 * @param ajo_id
	 *            The Identifier of the job to query for results.
	 * @param vsite
	 *            Wher the target job is executing.
	 * @return OutcomeTh The current results.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public OutcomeTh getOutcome(AJOIdentifier ajo_id, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {

		OutcomeTh reply = getActualOutcome(ajo_id, vsite);

		// From 3.2 onwards always expect an AbstractJobOutcome
		AbstractJob_Outcome ajoo;
		try {
			ajoo = ConsignForm.convertFrom((AbstractJob_Outcome) reply
					.getOutcome());
		} catch (ClassCastException ccex) {
			throw new TsJobManager.Exception(
					"Did not get AbstractJob_Outcome from NJS, versioning?");
		} catch (java.lang.Exception ex) {
			throw new TsJobManager.Exception(
					"Problems deserialising Outcomes in AJO Outcome");
		}
		reply.setOutcome(ajoo);
		if (ajoo.getStatus().isEquivalent(AbstractActionStatus.DONE)) {
			//2004/12/31 comment out not removing server side ajo
			//sendROA(ajo_id, vsite);
		}
		return reply;
	}

	/**
	 * @param from_consign
	 * @param ajo
	 * @param c
	 * @param vsite
	 * @return OutcomeTh
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	private OutcomeTh handleConignSynchronousReply(Reply from_consign,
			AbstractJob ajo, Connection c, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {

		OutcomeTh reply;

		if (from_consign instanceof ConsignJobReply) {
			// NJS did not want to do synchronously, revert to polling
			// but do not return from method until AJO is done
			CLogger
					.status("NJS does not want to execute synchronously, polling");

			do {
				try {
					Thread.currentThread().sleep(5000);
				} catch (java.lang.Exception ex) {
				}
				CLogger.status("Waiting for NJS to finish execution");
				reply = getActualOutcome((AJOIdentifier) ajo.getId(), vsite);
				if (reply == null) {
					return null;
				}
			} while (!(reply.getOutcome().getStatus()
					.isEquivalent(AbstractActionStatus.DONE)));

			// and acknowledge receipt
			sendROA((AJOIdentifier) ajo.getId(), vsite);
		} else {
			reply = processROR((RetrieveOutcomeReply) from_consign, c);
		}

		AbstractJob_Outcome ajoo;
		try {
			ajoo = ConsignForm.convertFrom(reply.getOutcome());
		} catch (java.lang.Exception ex) {
			throw new TsJobManager.Exception(
					"Problems deserialising Outcomes in AJO Outcome");
		}
		reply.setOutcome(ajoo);
		return reply;
	}

	/**
	 * Get a list of the Vsites (and Ports) provided by a Gateway. Updates the
	 * Gateway instance with current Vsites.
	 * 
	 * @param gateway
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public void listVsites(Gateway gateway) throws TsJobManager.Exception,
			Connection.Exception {

		Connection c = Connection.getConnection(gateway.getReference());

		try {

			Reply from_lv = c.send(new ListVsites());

			checkUPLReply(from_lv);

			gateway.clearVsites();

			ListVsitesReply lvr = (ListVsitesReply) from_lv;
			// OK now, Gateway returns Reply iff error

			Vsite[] temp = lvr.getList();
			for (int i = 0; i < temp.length; i++) {
				CLogger.status("Vsite: " + temp[i] + " "
						+ temp[i].getAJOSpecificationTitle() + " "
						+ temp[i].getAJOSpecificationVersion());
				gateway.addVsite(new VsiteTh(gateway.getReference(), temp[i]));
			}

			Reply from_lp = c.send(new com.fujitsu.arcon.upl.ListPorts());

			checkUPLReply(from_lp);

			gateway.clearPorts();

			com.fujitsu.arcon.upl.ListPortsReply lpr = (com.fujitsu.arcon.upl.ListPortsReply) from_lp;

			temp = lpr.getList();
			for (int i = 0; i < temp.length; i++) {
				CLogger.status("Port:  " + temp[i] + " "
						+ temp[i].getAJOSpecificationTitle() + " "
						+ temp[i].getAJOSpecificationVersion());
				gateway.addPort(new VsiteTh(gateway.getReference(), temp[i]));
			}
		} finally {
			c.done();
		}

	}

	/**
	 * makeConsignJob
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param synchronous
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @return ConsignJob
	 * @exception TsJobManager.Exception
	 */
	private ConsignJob makeConsignJob(AbstractJob ajo, boolean synchronous,
			VsiteTh vsite) throws TsJobManager.Exception {
		// V4, NJS requires a set Vsite
		if (ajo.getVsite() == null) {
			ajo.setVsite(new org.unicore.Vsite(vsite.getName()));
		}

		boolean polling_mode = !synchronous;

		Identity identity = ((Reference.SSL) vsite.getReference())
				.getIdentity();

		AbstractJob converted_ajo;
		try {
			converted_ajo = ConsignForm.convertTo(ajo, identity.getSignature());
		} catch (NotSerializableException nsex) {
			throw new TsJobManager.Exception(
					"Problems serialising AJO. Class Paths OK?");
		} catch (IOException ioex) {
			throw new TsJobManager.Exception(
					"Internal IO problems serialising AJO.");
		} catch (SignatureException sex) {
			throw new TsJobManager.Exception(
					"Problems serialising AJO, cryptography not initialised? "
							+ sex);
		} catch (NullPointerException nex) {
			throw new TsJobManager.Exception("Null pointer serialising AJO.\n"
					+ nex);
		}
		// Check to see if the "synchronous" request should be done as polling
		// or
		// non-polling (this can be toggled in the UI for testing)
		if (alwaysPoll()) {
			polling_mode = true;
		}
		ConsignJob reply = new ConsignJob((AJOIdentifier) ajo.getId(),
				new Vsite(vsite.getReference().getAddress(), vsite.getName()),
				identity.getUser(), converted_ajo.getConsignForm(),
				converted_ajo.getSignature(), (byte[]) null,
				ajo.getStreamed() != null, polling_mode);
		return reply;
	}

	/**
	 * makeConsignJob new parameter added to select rewriting AJO-ID or not.
	 * 
	 * @param ajo
	 *            The AbstractJob to execute.
	 * @param synchronous
	 * @param vsite
	 *            Where to execute the AbstractJob.
	 * @param change_ajoid
	 *            change AJO-ID or not.
	 * @return ConsignJob
	 * @exception TsJobManager.Exception
	 */
	private ConsignJob makeConsignJob(AbstractJob ajo, boolean synchronous,
			VsiteTh vsite, boolean change_ajoid) throws TsJobManager.Exception {
		// V4, NJS requires a set Vsite
		if (ajo.getVsite() == null) {
			ajo.setVsite(new org.unicore.Vsite(vsite.getName()));
		}

		if (change_ajoid == true) {
			org.unicore.utility.ResetIdentifiers.doit(ajo);
		}

		boolean polling_mode = !synchronous;

		Identity identity = ((Reference.SSL) vsite.getReference())
				.getIdentity();

		AbstractJob converted_ajo;
		try {
			converted_ajo = ConsignForm.convertTo(ajo, identity.getSignature());
		} catch (NotSerializableException nsex) {
			throw new TsJobManager.Exception(
					"Problems serialising AJO. Class Paths OK?");
		} catch (IOException ioex) {
			throw new TsJobManager.Exception(
					"Internal IO problems serialising AJO.");
		} catch (SignatureException sex) {
			throw new TsJobManager.Exception(
					"Problems serialising AJO, cryptography not initialised? "
							+ sex);
		} catch (NullPointerException nex) {
			throw new TsJobManager.Exception("Null pointer serialising AJO.\n"
					+ nex);
		}

		if (alwaysPoll()) {
			polling_mode = true;
		}
		ConsignJob reply = new ConsignJob((AJOIdentifier) ajo.getId(),
				new Vsite(vsite.getReference().getAddress(), vsite.getName()),
				identity.getUser(), converted_ajo.getConsignForm(),
				converted_ajo.getSignature(), (byte[]) null,
				ajo.getStreamed() != null, polling_mode);
		return reply;
	}

	/**
	 * @param ror
	 * @param c
	 * @return OutcomeTh
	 * @exception TsJobManager.Exception
	 */
	private OutcomeTh processROR(RetrieveOutcomeReply ror, Connection c)
			throws TsJobManager.Exception {

		byte[] outcome_bytes = ror.getOutcome();

		Outcome njs_reply;
		try {
			njs_reply = (Outcome) (new ObjectInputStream(
					new ByteArrayInputStream(outcome_bytes))).readObject();
		} catch (java.lang.Exception ex) {
			throw new TsJobManager.Exception(
					"Problems deserialising Outcome (classes compatible?)");
		}

		OutcomeTh reply = new OutcomeTh();
		reply.setOutcome((AbstractJob_Outcome) njs_reply);

		// Process any streamed data too
		if (ror.hasStreamed()) {
			CLogger.status("Reading streamed data from a RetrieveOutcomeReply");
			ZipInputStream streamed = null;
			try {
				streamed = c.getStreamedInputStream();
			} catch (IOException ioex) {
				throw new TsJobManager.Exception(
						"Problems getting streamed data: " + ioex.getMessage());
			}

			try {
				// Ought to read whole stream, even if we have a problem
				String message = null;
				ZipEntry entry = streamed.getNextEntry();

				//preparation for getting total files size
				int total_file_size = 0;

				while (entry != null) {
					String mode = "no mode";
					if (entry.getExtra() != null) {
						mode = "mode " + Byte.toString(entry.getExtra()[0]);
					}
					if (entry.isDirectory()) {
						CLogger.status("Directory <" + entry.getName()+ "> skipped.");
					} else {
						
						String tmp = entry.getName();

						//cut directory name and get file name only.
						File entry_file;
						if (tmp.substring(tmp.lastIndexOf("/") + 1).equals("stderr") && (getErrFileName() != null)) {
							entry_file = new File(outcome_dir, getErrFileName());
						} else {
							entry_file = new File(outcome_dir, tmp.substring(tmp.lastIndexOf("/") + 1, tmp.length()));
						}

						int read;
						byte[] buffer = new byte[65536];
						FileOutputStream fos = new FileOutputStream(entry_file);

						do {
							read = streamed.read(buffer, 0, buffer.length);
							if (read > 0) {
								fos.write(buffer, 0, read);
							}
						} while (read >= 0);
						fos.close();

						//get file size
						total_file_size += entry_file.length();

						if (entry_file.getName().equals("stderr")) {
							//check exicode from sterr file
							setExitValue(entry_file);
							//rename error file (if user don't set error file
							// name or request not to fetch error file, delete stderr file)
							if (getErrFileName() != null) {
								entry_file.renameTo(new File(getErrFileName()));
							}else{
								entry_file.delete();
							}
						}
						
						if (entry_file.getName().equals("stdout")) {
							entry_file.delete();
						}
						

						String path = entry.getName();
						String the_action_id = null;

						StringTokenizer st = new StringTokenizer(path, "/");
						while (st.hasMoreTokens()) {
							String this_token = st.nextToken();

							if (!this_token.startsWith("AA")) {
								break;
							}
							the_action_id = this_token;
						}
						if (the_action_id == null) {
							message = "Streamed file names badly structured (expect AA)";
						}
						Integer id_value;
						try {
							long temp = Long.parseLong(the_action_id
									.substring(2), 16);
							if (temp > Integer.MAX_VALUE) {
								temp = temp + 2 * Integer.MIN_VALUE;
							}
							id_value = new Integer((int) temp);
						} catch (java.lang.Exception ex) {
							message = "Problems parsing directory as AA identifier: "
									+ the_action_id.substring(2);
							break;
						}
						Collection already_there = (Collection) reply
								.getFilesMapping().get(id_value);
						if (already_there == null) {
							already_there = new HashSet();
							reply.getFilesMapping()
									.put(id_value, already_there);
						}
						already_there.add(entry_file);
					}
					entry = streamed.getNextEntry();
				}
				//set total file size
				setFetchedDataSize(total_file_size);

				if (message != null) {
					throw new TsJobManager.Exception(message);
				}
			} catch (ZipException zex) {
				throw new TsJobManager.Exception(
						"Errors (zip) reading data streamed from NJS.");
			} catch (IOException ioex) {
				throw new TsJobManager.Exception(
						"Errors reading data streamed from NJS. "+
						ioex.getMessage());
			}
			try {
				streamed.close();
			} catch (java.lang.Exception ex) {
			}
			c.doneWithStreamedInputStream();
		}
		return reply;
	}

	/**
	 * @param stderr
	 *            standard error file
	 * @return int exit code
	 * @throws IOException
	 * @throws NumberFormatException
	 */
	public void setExitValue(File stderr) {

		try {
			FileReader fr = new FileReader(stderr);
			BufferedReader br = new BufferedReader(fr);
			String line;

			while ((line = br.readLine()) != null) {
				if (line.startsWith("UNICORE EXIT STATUS")) {
					int tail = line.indexOf("+");
					int top = line.indexOf("UNICORE EXIT STATUS");
					String exit = line.substring(top + 20, tail - 1);
					//System.out.println("Exit code = " + exit);
					Integer exit_val = new Integer(exit);
					this.exitcode = exit_val.intValue();
					//exit signal
					int exit_signal = exit_val.intValue() - 128;
					if (exit_signal >= 0) {
						this.exitsignal = exit_signal;
					}
				}
			}
			br.close();
			fr.close();

		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Send a message to clean up a job from the Vsite. This is not usually
	 * necessary as the consign ot getOutcome calls will clean up when a job has
	 * completed execution.
	 * 
	 * @param ajo_id
	 *            The job to clean up.
	 * @param vsite
	 *            Wher the job is running.
	 * @exception TsJobManager.Exception
	 * @exception Connection.Exception
	 */
	public void sendROA(AJOIdentifier ajo_id, VsiteTh vsite)
			throws TsJobManager.Exception, Connection.Exception {

		RetrieveOutcomeAck roa = new RetrieveOutcomeAck(ajo_id, new Vsite(vsite
				.getReference().getAddress(), vsite.getName()));

		Connection c = Connection.getConnection(vsite.getReference());

		try {
			Reply from_roa = c.send(roa);
			checkUPLReply(from_roa);
		} finally {
			c.done();
		}
	}

	/**
	 * Set the mode for waiting for the end of a job. Generally best to leave at
	 * the deafult value.
	 * 
	 * @param b
	 */
	void setAlwaysPoll(boolean b) {
		always_poll = b;
	}

	/**
	 * Set the directory that the Client Libraries will use to write result
	 * files fetched from the NJS.
	 * 
	 * @param f
	 */
	public void setOutcomeRootDirectory(File f) {
		outcome_dir = f;
	}

	/**
	 * @param error_file_name
	 */
	public void setErrFileName(String error_file_name) {
		this.err = error_file_name;
	}

	/**
	 * @return String err
	 */
	public String getErrFileName() {
		return this.err;
	}

	/**
	 * @return int Returns the fetchedDataSize.
	 */
	public int getFetchedDataSize() {
		return fetchedDataSize;
	}

	/**
	 * @param int
	 *            fetchedDataSize The fetchedDataSize to set.
	 */
	public void setFetchedDataSize(int fetchedDataSize) {
		this.fetchedDataSize = fetchedDataSize;
	}

	/**
	 * @return int Return Exit Code
	 */
	public int getExitCode() {
		return this.exitcode;
	}

	/**
	 * @return int Return Exit Signal
	 */
	public int getExitSignal() {
		return this.exitsignal;
	}

	/**
	 * @author ??
	 * @version ??
	 */
	public class Exception extends java.lang.Exception {

		/**
		 * @param message
		 */
		public Exception(String message) {
			super(message);
		}
	}


}
// Changes
//
// 23-07-01 SvdB File mode information is transferred
//           Copyright (c) Fujitsu Ltd 2000 - 2001
//
// Use and distribution is subject to the Community Source Licence.
//
// A copy of the Community Source Licence came with this distribution.
// Copies are also available at http://www.fujitsu.co.uk/arcon or by
// email from arcon@fujitsu.co.uk.
