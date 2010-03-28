package com.fujitsu.arcon.servlet;

import org.unicore.ajo.Portfolio;

import java.io.File;

/**
 * Client view of a org.unicore.ajo.Portfolio, includes the Portfolio and a list
 * of the actuals files that will be transferred to the Vsite as ths Portfolio.
 * File can be renamed in transfer.
 * 
 * @author Sven van den Berghe, fujitsu
 * @version $Revision: 1.1 $ $Date: 2005/11/01 05:52:24 $
 *  
 */
public class PortfolioTh {

	private Portfolio portfolio;

	private File[] files;

	private String[] destination_names;

	/**
	 * @param portfolio
	 *            The Portfolio that is used within the job (AJO) to refer to
	 *            this set of files.
	 * @param files
	 *            The files making up the Portfolio.
	 * @param destination_names
	 *            The names at the Vsite (parallel array to files, null array if
	 *            no renaming required, otherwise populated with file names).
	 *  
	 */
	public PortfolioTh(Portfolio portfolio, File[] files,
			String[] destination_names) {
		this.portfolio = portfolio;
		this.files = files;
		this.destination_names = destination_names;
	}

	public PortfolioTh(Portfolio portfolio, File[] files) {
		this.portfolio = portfolio;
		this.files = files;
		destination_names = null;
	}

	public Portfolio getPortfolio() {
		return portfolio;
	}

	public File[] getFiles() {
		return files;
	}

	public String[] getDestinationNames() {
		return destination_names;
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
