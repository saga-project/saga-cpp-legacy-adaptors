package org.naregi.rns.client;

import java.util.Iterator;

/**
 * Iterator interface for List and Search operations.
 */
public interface RNSIterator<E> extends Iterator<E> {

	/**
	 * Return the RNSError which was occurred in Iterator#next() or
	 * Iterator#hasNext() methods.
	 *
	 * @return null if no error occurs
	 */
	public RNSError getError();
}
