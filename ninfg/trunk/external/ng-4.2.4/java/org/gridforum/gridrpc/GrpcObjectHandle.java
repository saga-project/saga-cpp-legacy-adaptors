/*
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
 * $RCSfile: GrpcObjectHandle.java,v $ $Revision: 1.13 $ $Date: 2006/08/22 10:54:33 $
 */
package org.gridforum.gridrpc;

import java.util.List;
import java.util.Properties;
import java.util.Vector;

/**
 * Provides interfaces to control RemoteObjects.<br>
 * This class keeps information about a server and a RemoteObject,
 * can call a RemoteObject with arguments and receive a result of it.<br>
 * RemoteObjects have several methods, so you have to specify which method to call.<br>
 * 
 * If an error occurred in this class, then it will be notified by throwing
 * {@link org.gridforum.gridrpc.GrpcException}.
 */
public abstract class GrpcObjectHandle {
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * You have to specify a name of method.
	 * 
	 * @param args List of arguments for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public abstract GrpcExecInfo invokeWith(String methodName,
												Properties sessionAttr,
												List args) throws GrpcException;
	
	/**
	 * Cancels a current session.
	 * 
	 * @throws GrpcException if it failed to cancel.
	 */
	public abstract void cancel() throws GrpcException;
	
	/**
	 * Dispose this handle.<br>
	 * This method must be called when you don't need this handle any more.
	 * 
	 * @throws GrpcException if it failed to dispose.
	 */
	public abstract void dispose() throws GrpcException;
	
	/**
	 * Calls a RemoteMethod without any arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName) throws GrpcException {
		/* no argument here */
		return invoke(methodName, (Properties)null);
	}
	
	/**
	 * Calls a RemoteMethod with a specified argument.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,
								Object a1) throws GrpcException{
		return invoke(methodName, (Properties)null, a1);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,
								Object a1, Object a2) throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,
								Object a1, Object a2, Object a3)
								throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,
								Object a1, Object a2, Object a3, Object a4)
								throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3, a4);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,
								Object a1, Object a2, Object a3, Object a4, Object a5)
								throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3, a4, a5);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6) throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3, a4, a5, a6);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7) throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3, a4, a5, a6, a7);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8) throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3, a4, a5, a6, a7, a8);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							  	Object a6, Object a7, Object a8, Object a9)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3, a4, a5, a6, a7, a8, a9);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12)
								throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
								 Object a6, Object a7, Object a8, Object a9, Object a10,
								 Object a11, Object a12, Object a13)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
								Object a11, Object a12, Object a13, Object a14)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23, a24);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23, a24, a25);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25,
							 	Object a26)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23, a24, a25, a26);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25,
							 	Object a26, Object a27)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23, a24, a25, a26, a27);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @param a28 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25,
							 	Object a26, Object a27, Object a28)
							 	throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23, a24, a25, a26, a27, a28);
	}

	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @param a28 an argument for a RemoteMethod.
	 * @param a29 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
								Object a6, Object a7, Object a8, Object a9, Object a10,
								Object a11, Object a12, Object a13, Object a14, Object a15,
								Object a16, Object a17, Object a18, Object a19, Object a20,
								Object a21, Object a22, Object a23, Object a24, Object a25,
								Object a26, Object a27, Object a28, Object a29)
								throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23, a24, a25, a26, a27, a28, a29);
	}
	
	/**
	 * Calls a RemoteMethod with specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @param a28 an argument for a RemoteMethod.
	 * @param a29 an argument for a RemoteMethod.
	 * @param a30 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
								Object a6, Object a7, Object a8, Object a9, Object a10,
								Object a11, Object a12, Object a13, Object a14, Object a15,
								Object a16, Object a17, Object a18, Object a19, Object a20,
								Object a21, Object a22, Object a23, Object a24, Object a25,
								Object a26, Object a27, Object a28, Object a29, Object a30)
								throws GrpcException{
		return invoke(methodName, (Properties)null,
						a1,  a2,  a3,  a4,  a5,  a6,  a7,  a8,  a9,  a10,
						a11, a12, a13, a14, a15, a16, a17, a18, a19, a20,
						a21, a22, a23, a24, a25, a26, a27, a28, a29, a30);
	}
	/**
	 * Calls a RemoteMethod with attributes and no arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, Properties sessionAttr) throws GrpcException {
		/* no argument here */
		Vector arg = new Vector();
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and a specified argument.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, Properties sessionAttr, 
								Object a1) throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, Properties sessionAttr, 
								Object a1, Object a2) throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, Properties sessionAttr, 
								Object a1, Object a2, Object a3)
								throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4)
								throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName, Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5)
								throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6) throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7) throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8) throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							  	Object a6, Object a7, Object a8, Object a9)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12)
								throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
								 Object a6, Object a7, Object a8, Object a9, Object a10,
								 Object a11, Object a12, Object a13)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
								Object a11, Object a12, Object a13, Object a14)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		arg.addElement(a24);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		arg.addElement(a24);
		arg.addElement(a25);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25,
							 	Object a26)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		arg.addElement(a24);
		arg.addElement(a25);
		arg.addElement(a26);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25,
							 	Object a26, Object a27)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		arg.addElement(a24);
		arg.addElement(a25);
		arg.addElement(a26);
		arg.addElement(a27);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @param a28 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
							 	Object a6, Object a7, Object a8, Object a9, Object a10,
							 	Object a11, Object a12, Object a13, Object a14, Object a15,
							 	Object a16, Object a17, Object a18, Object a19, Object a20,
							 	Object a21, Object a22, Object a23, Object a24, Object a25,
							 	Object a26, Object a27, Object a28)
							 	throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		arg.addElement(a24);
		arg.addElement(a25);
		arg.addElement(a26);
		arg.addElement(a27);
		arg.addElement(a28);
		return invokeWith(methodName, sessionAttr, arg);
	}

	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @param a28 an argument for a RemoteMethod.
	 * @param a29 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
								Object a6, Object a7, Object a8, Object a9, Object a10,
								Object a11, Object a12, Object a13, Object a14, Object a15,
								Object a16, Object a17, Object a18, Object a19, Object a20,
								Object a21, Object a22, Object a23, Object a24, Object a25,
								Object a26, Object a27, Object a28, Object a29)
								throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		arg.addElement(a24);
		arg.addElement(a25);
		arg.addElement(a26);
		arg.addElement(a27);
		arg.addElement(a28);
		arg.addElement(a29);
		return invokeWith(methodName, sessionAttr, arg);
	}
	
	/**
	 * Calls a RemoteMethod with attributes and specified arguments.<br>
	 * 
	 * @param methodName a name of a RemoteMethod.
	 * @param sessionAttr attributes for the session
	 * @param a1 an argument for a RemoteMethod.
	 * @param a2 an argument for a RemoteMethod.
	 * @param a3 an argument for a RemoteMethod.
	 * @param a4 an argument for a RemoteMethod.
	 * @param a5 an argument for a RemoteMethod.
	 * @param a6 an argument for a RemoteMethod.
	 * @param a7 an argument for a RemoteMethod.
	 * @param a8 an argument for a RemoteMethod.
	 * @param a9 an argument for a RemoteMethod.
	 * @param a10 an argument for a RemoteMethod.
	 * @param a11 an argument for a RemoteMethod.
	 * @param a12 an argument for a RemoteMethod.
	 * @param a13 an argument for a RemoteMethod.
	 * @param a14 an argument for a RemoteMethod.
	 * @param a15 an argument for a RemoteMethod.
	 * @param a16 an argument for a RemoteMethod.
	 * @param a17 an argument for a RemoteMethod.
	 * @param a18 an argument for a RemoteMethod.
	 * @param a19 an argument for a RemoteMethod.
	 * @param a20 an argument for a RemoteMethod.
	 * @param a21 an argument for a RemoteMethod.
	 * @param a22 an argument for a RemoteMethod.
	 * @param a23 an argument for a RemoteMethod.
	 * @param a24 an argument for a RemoteMethod.
	 * @param a25 an argument for a RemoteMethod.
	 * @param a26 an argument for a RemoteMethod.
	 * @param a27 an argument for a RemoteMethod.
	 * @param a28 an argument for a RemoteMethod.
	 * @param a29 an argument for a RemoteMethod.
	 * @param a30 an argument for a RemoteMethod.
	 * @return {@link org.gridforum.gridrpc.GrpcExecInfo}
	 * @throws GrpcException if it failed to call a RemoteMethod.
	 */
	public GrpcExecInfo invoke(String methodName,  Properties sessionAttr, 
								Object a1, Object a2, Object a3, Object a4, Object a5,
								Object a6, Object a7, Object a8, Object a9, Object a10,
								Object a11, Object a12, Object a13, Object a14, Object a15,
								Object a16, Object a17, Object a18, Object a19, Object a20,
								Object a21, Object a22, Object a23, Object a24, Object a25,
								Object a26, Object a27, Object a28, Object a29, Object a30)
								throws GrpcException{
		Vector arg = new Vector();
		arg.addElement(a1);
		arg.addElement(a2);
		arg.addElement(a3);
		arg.addElement(a4);
		arg.addElement(a5);
		arg.addElement(a6);
		arg.addElement(a7);
		arg.addElement(a8);
		arg.addElement(a9);
		arg.addElement(a10);
		arg.addElement(a11);
		arg.addElement(a12);
		arg.addElement(a13);
		arg.addElement(a14);
		arg.addElement(a15);
		arg.addElement(a16);
		arg.addElement(a17);
		arg.addElement(a18);
		arg.addElement(a19);
		arg.addElement(a20);
		arg.addElement(a21);
		arg.addElement(a22);
		arg.addElement(a23);
		arg.addElement(a24);
		arg.addElement(a25);
		arg.addElement(a26);
		arg.addElement(a27);
		arg.addElement(a28);
		arg.addElement(a29);
		arg.addElement(a30);
		return invokeWith(methodName, sessionAttr, arg);
	}
}
