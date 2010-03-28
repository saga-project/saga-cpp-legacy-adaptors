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
 * $RCSfile: NgExpression.java,v $ $Revision: 1.11 $ $Date: 2005/04/18 09:55:04 $
 */
package org.apgrid.grpc.ng;

import java.util.List;
import java.util.Stack;
import java.util.Vector;

import org.apgrid.grpc.util.*;
import org.gridforum.gridrpc.GrpcException;
import org.w3c.dom.Node;

public class NgExpression {
	public static final String elemName = "expression";
	
	/* members */
	private int index;
	private List type;
	private List val;
	
	/* expression */
	public static final int OP_VALUE_PLUS  = 1;
	public static final int OP_VALUE_MINUS = 2;
	public static final int OP_VALUE_MUL   = 3;
	public static final int OP_VALUE_DIV   = 4;
	public static final int OP_VALUE_MOD   = 5;
	public static final int OP_VALUE_UN_MINUS = 6;
	public static final int OP_VALUE_POW = 7;
	public static final int OP_VALUE_EQ    = 8;    /*  ==  */
	public static final int OP_VALUE_NEQ   = 9;    /*  !=  */
	public static final int OP_VALUE_GT    = 10;   /*  >  */
	public static final int OP_VALUE_LT    = 11;   /*  <  */
	public static final int OP_VALUE_GE    = 12;   /*  >=  */
	public static final int OP_VALUE_LE    = 13;   /*  <=  */
	public static final int OP_VALUE_TRY   = 14;   /*  ? : */
	public static final int OP_END = 99;

	/* definitions for XML */
	public static final int VALUE_ERROR = -1;
	/** default */
	public static final int VALUE_NONE = 0;
	/** default, give by constant */
	public static final int VALUE_CONST = 1;
	/** specified by IN scalar parameter */
	public static final int VALUE_IN_ARG = 2;
	/** operation code */
	public static final int VALUE_OP = 3;
	/** end of expression */
	public static final int VALUE_END_OF_OP = 4;

	/* String array for indicate operator */
	private static final String opArray[] = {
		"DUMMY",
		"add",
		"sub",
		"mul",
		"div",
		"mod",
		"neg", 
		"pow", 
		"eq", 
		"neq", 
		"gt", 
		"lt", 
		"ge", 
		"le", 
		"tri"
	};

	/* number of element for each operator */
	private static final int opLen[] = {
		0, /* DUMMY */
		2, /* add */
		2, /* sub */
		2, /* mul */
		2, /* div */ 
		2, /* mod */ 
		1, /* neg */
		2, /* pow */
		2, /* eq */
		2, /* neq */
		2, /* gt */
		2, /* lt */
		2, /* qe */
		2, /* le */
		3  /* tri */
	};

	/* String of indicate number of element for operator */
	public static final String NGEXP_STR_NOVAL = "noValue";
	public static final String NGEXP_STR_IMMEDIATE = "immediate";
	public static final String NGEXP_STR_SCALARREF = "scalarref";
	public static final String NGEXP_STR_MONOARITH = "monoArithmetic";
	public static final String NGEXP_STR_BIARITH = "biArithmetic";
	public static final String NGEXP_STR_TRIARITH = "triArithmetic";
	
	private static String arith[] = {
		"DUMMY", 
		NGEXP_STR_MONOARITH, 
		NGEXP_STR_BIARITH, 
		NGEXP_STR_TRIARITH
	};

	/**
	 * 
	 */
	public NgExpression() {
		type = new Vector();
		val = new Vector();
	}
	
	/**
	 * @param node
	 */
	public NgExpression(Node node) throws GrpcException {
		this();
		traverseExpression(node);
		addElem(VALUE_END_OF_OP, 0);
	}
	
	/**
	 * @param val
	 * @return
	 */
	String makeVarName(int val){
	  return (new Character((char)('A' + val))).toString();
	}
	
	/**
	 * @return
	 */
	public String toText(){
	  Stack stack = new Stack();
	  try {
		for (int i = 0; i < type.size(); i++){
	  switch (((Integer) (type.get(i))).intValue()){
	  case NgParamTypes.NG_EXP_VALUE_NONE:
		stack.push("");
		break;
	  case NgParamTypes.NG_EXP_VALUE_CONST:
		stack.push(""+ ((Integer) (val.get(i))).intValue());
		break;
	  case NgParamTypes.NG_EXP_VALUE_IN_ARG:
		stack.push(makeVarName(((Integer) (val.get(i))).intValue()));
		break;	
	  case NgParamTypes.NG_EXP_VALUE_OP:
		switch((((Integer) (val.get(i))).intValue())){
		case OP_VALUE_PLUS:
		  stack.push("("+(String)(stack.pop()) + " + " +(String)(stack.pop())+ ")");
		  break;
		case OP_VALUE_MINUS:
			  stack.push("("+(String)(stack.pop()) + " - " +(String)(stack.pop())+ ")");
		  break;
		case OP_VALUE_MUL:
		  stack.push("("+(String)(stack.pop()) + " * " +(String)(stack.pop())+ ")");
		  break;
		case OP_VALUE_DIV:
			  stack.push("("+(String)(stack.pop()) + " / " +(String)(stack.pop())+ ")");
		  break;
		case OP_VALUE_MOD:
			  stack.push("("+(String)(stack.pop()) + " % " +(String)(stack.pop())+ ")");
		  break;
		case OP_VALUE_UN_MINUS:
		  stack.push("-(" + (String) (stack.pop()) + ")");
		  break;
		case OP_VALUE_POW:
			  stack.push("("+(String)(stack.pop()) + " ^ " +(String)(stack.pop())+ ")");
		  break;
		case OP_VALUE_EQ:{
			  stack.push("("+(String)(stack.pop()) + " == " +(String)(stack.pop())+ ")");
		  break;
		}
		case OP_VALUE_NEQ:{
			  stack.push("("+(String)(stack.pop()) + " != " +(String)(stack.pop())+ ")");
		  break;
		}
		case OP_VALUE_GT:{
			  stack.push("("+(String)(stack.pop()) + " > " +(String)(stack.pop())+ ")");
		  break;
		}
		case OP_VALUE_LT:{
			  stack.push("("+(String)(stack.pop()) + " < " +(String)(stack.pop())+ ")");
		  break;
		}
		case OP_VALUE_GE:{
			  stack.push("("+(String)(stack.pop()) + " >= " +(String)(stack.pop())+ ")");
		  break;
		}
		case OP_VALUE_LE:{
			  stack.push("("+(String)(stack.pop()) + " <= " +(String)(stack.pop())+ ")");
		  break;
		}
		case OP_VALUE_TRY:{
		  stack.push("(" + (String) stack.pop() + " ? " + (String) stack.pop() + ":" + (String) stack.pop() + ")");
		  break;
		}
	  
		default:
		  throw new NgArgTypeException("NGEXPRESSION: Unrecognize type");
		}
		break;
	  case NgParamTypes.NG_EXP_VALUE_END_OF_OP:
		return (String) (stack.pop());
	  default:
	  	throw new NgArgTypeException("NGEXPRESSION: Unrecognize type");
	  }
		}
		return (String) (stack.pop());
	  } catch (Exception e) {
		e.printStackTrace();
	  }
	  return "";
	}

	/**
	 * @param arg
	 * @return
	 * @throws NgArgTypeException
	 */
	public long calc(int arg[]) throws NgArgTypeException {
		Stack stack = new Stack();
		for (int i = 0; i < type.size(); i++) {
			switch ((((Integer) (type.get(i))).intValue())){
			case NgParamTypes.NG_EXP_VALUE_NONE:
	  			stack.push(new Long(0));
	  			break;
			case NgParamTypes.NG_EXP_VALUE_CONST:
	  			stack.push(new Long(((Integer) (val.get(i))).intValue()));
	  			break;
			case NgParamTypes.NG_EXP_VALUE_IN_ARG:
	  			stack.push(new Long(arg[((Integer) (val.get(i))).intValue()]));
	  			break;	
			case NgParamTypes.NG_EXP_VALUE_OP:
	  			switch(((Integer) (val.get(i))).intValue()){
	  			case OP_VALUE_PLUS:
					stack.push(new Long( ((Long)(stack.pop())).intValue() + 
					 						((Long)(stack.pop())).intValue()));
					break;
	  			case OP_VALUE_MINUS:
					stack.push(new Long( ((Long)(stack.pop())).intValue() - 
	 										((Long)(stack.pop())).intValue()));
	  				break;
	  			case OP_VALUE_MUL:
					stack.push(new Long (((Long)(stack.pop())).intValue() *
								((Long)(stack.pop())).intValue()));
					break;
		  		case OP_VALUE_DIV:
					stack.push(new Long( ((Long)(stack.pop())).intValue() / 
								((Long)(stack.pop())).intValue()));
		  			break;
		  		case OP_VALUE_MOD:
					stack.push(new Long( ((Long)(stack.pop())).intValue() %
								((Long)(stack.pop())).intValue()));
		  			break;
		  		case OP_VALUE_UN_MINUS:
						stack.push(new Long(- (((Long)(stack.pop())).intValue())));
						break;
		  		case OP_VALUE_POW:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						stack.push(new Long((int)Math.pow((double)tmp1,(double)tmp2)));
						break;
		  			}
		  		case OP_VALUE_EQ:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						stack.push(new Long(tmp1 == tmp2 ? 1 : 0));
						break;
		  			}
		  		case OP_VALUE_NEQ:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						stack.push(new Long(tmp1 != tmp2 ? 1 : 0));
						break;
		  			}
		  		case OP_VALUE_GT:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						stack.push(new Long(tmp1 > tmp2 ? 1 : 0));
						break;
		  			}
		  		case OP_VALUE_LT:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						stack.push(new Long(tmp1 < tmp2 ? 1 : 0));
						break;
		  			}
		  		case OP_VALUE_GE:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						stack.push(new Long(tmp1 >= tmp2 ? 1 : 0));
						break;
		  			}
		  		case OP_VALUE_LE:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						stack.push(new Long(tmp1 <= tmp2 ? 1 : 0));
						break;
		  			}
		  		case OP_VALUE_TRY:
		  			{
						int tmp1 = ((Long)(stack.pop())).intValue();
						int tmp2 = ((Long)(stack.pop())).intValue();
						int tmp3 = ((Long)(stack.pop())).intValue();
						stack.push(new Long(tmp1 == 1 ? tmp2 : tmp3));
						break;
		  			}
				default:
					throw new NgArgTypeException("NgExpression: fail to calc.");
	  			}
	  			break;
			case NgParamTypes.NG_EXP_VALUE_END_OF_OP:
	  			return ((Long)(stack.pop())).intValue();
			default:
				throw new NgArgTypeException("NgExpression: fail to calc.");
			}
	  	}
	  	return ((Long)(stack.pop())).intValue();
	}
	
	/* methods for XML */
	/**
	 * @param code
	 * @return
	 */
	private static String getOperatorString(int code) {
		return opArray[code];
	}

	/**
	 * @param code
	 * @return
	 */
	private static int getOperatorLen(int code) {
		return opLen[code];
	}

	/**
	 * @param str
	 * @return
	 */
	private static int getOperatorCode(String str) {
		for (int i = 0; i < opArray.length; i++) {
			if (str.equals(opArray[i]))
				return i;
		}
		return -1;
	}

	/**
	 * @param type
	 * @param val
	 */
	private void addElem(int type, int val) {
		this.type.add(new Integer(type));
		this.val.add(new Integer(val));
		index++;
	}

	/**
	 * @param node
	 * @throws XMLReadException
	 */
	private void traverseExpression(Node node) throws NgXMLReadException {
		Node next = null;
		if ((next = XMLUtil.getChildNodeGentle(node,
			NGEXP_STR_MONOARITH)) != null) {
			traverseExpressionMono(next);
			return;
		}
		if ((next = XMLUtil.getChildNodeGentle(node,
			NGEXP_STR_BIARITH)) != null) {
			traverseExpressionBi(next);
			return;
		}
		if ((next = XMLUtil.getChildNodeGentle(node,
			NGEXP_STR_TRIARITH)) != null) {
			traverseExpressionTri(next);
			return;
		}
		if ((next = XMLUtil.getChildNodeGentle(node,
			NGEXP_STR_IMMEDIATE)) != null) {
			traverseExpressionImmediate(next);
			return;
		} if ((next = XMLUtil.getChildNodeGentle(node,
			NGEXP_STR_SCALARREF)) != null) {
			traverseExpressionScalarref(next);
			return;
		}
		if ((next = XMLUtil.getChildNodeGentle(node,
			NGEXP_STR_NOVAL)) != null) {
			traverseExpressionNoValue(next);
			return;
		}
		throw new NgXMLReadException("no proper child node for expression");
	}
	
	/**
	 * @param node
	 * @throws XMLReadException
	 */
	private void traverseExpressionMono(Node node)
		throws NgXMLReadException {

		Node node0 = XMLUtil.getChildNode(node, elemName, 0);
		traverseExpression(node0);
		int op = getOperatorCode(XMLUtil.getAttributeValue(node, "name"));
		addElem(VALUE_OP, op);
	}

	/**
	 * @param node
	 * @throws NgXMLReadException
	 */
	private void traverseExpressionBi(Node node) throws NgXMLReadException {
		Node node0, node1;
		node0 = XMLUtil.getChildNode(node, elemName, 0);
		node1 = XMLUtil.getChildNode(node, elemName, 1);
		traverseExpression(node0);
		traverseExpression(node1);
		int op = getOperatorCode(XMLUtil.getAttributeValue(node, "name"));
		addElem(VALUE_OP, op);
	}
	
	/**
	 * @param node
	 * @throws XMLReadException
	 */
	private void traverseExpressionTri(Node node) throws NgXMLReadException {
		Node node0 = XMLUtil.getChildNode(node, elemName, 0);
		Node node1 = XMLUtil.getChildNode(node, elemName, 1);
		Node node2 = XMLUtil.getChildNode(node, elemName, 2);
		traverseExpression(node0);
		traverseExpression(node1);
		traverseExpression(node2);
		int op = getOperatorCode(XMLUtil.getAttributeValue(node, "name"));
		addElem(VALUE_OP, op);
	}

	/**
	 * @param node
	 * @throws NgXMLReadException
	 */
	private void traverseExpressionImmediate(Node node) throws NgXMLReadException {
		String val = XMLUtil.getAttributeValue(node, "val");
		addElem(VALUE_CONST, Integer.parseInt(val));
	}
	
	/**
	 * @param node
	 * @throws XMLReadException
	 */
	private void traverseExpressionScalarref(Node node) throws NgXMLReadException {
		String val = XMLUtil.getAttributeValue(node, "val");
		addElem(VALUE_IN_ARG, Integer.parseInt(val));
	}

	/**
	 * @param node
	 * @throws NgXMLReadException
	 */
	private void traverseExpressionNoValue(Node node) throws NgXMLReadException {
		addElem(VALUE_NONE, 0);
	}
	
	/**
	 * @return
	 */
	private int getLastIndex() {
		for (int i = type.size() - 1; i >= 0; i--) {
			if ((((Integer) (type.get(i))).intValue()) == VALUE_END_OF_OP)
				return i-1;
		}
		if ((((Integer) (type.get(0))).intValue()) == VALUE_NONE)
			return 0;
		return -1;
	}
	
	/**
	 * @return
	 */
	public String toXMLString() {
		StringBuffer sb = new StringBuffer();
		index = getLastIndex();
		toXMLSub(sb);
		return sb.toString();
	}

	/**
	 * @param sb
	 */
	private void toXMLSub(StringBuffer sb) {
		int type  = (((Integer) (this.type.get(index))).intValue());
		int value = (((Integer) (this.val.get(index))).intValue());
		index--;
		sb.append("<" + elemName + ">");

		switch (type) {
			case VALUE_NONE:
				sb.append("<" + NGEXP_STR_NOVAL + "/>");
				break;
			case VALUE_CONST:
				sb.append("<" + NGEXP_STR_IMMEDIATE + " val=\""+ value +"\"/>");
				break;
			case VALUE_IN_ARG:
				sb.append("<" + NGEXP_STR_SCALARREF + " val=\""+ value +"\"/>");
				break;
			case VALUE_OP:
				String operator = getOperatorString(value);
				int len = getOperatorLen(value);
				sb.append("<" + arith[len] + " name=\""+ operator + "\">");
				for (int i = 0; i <len; i++)
					toXMLSub(sb);
				sb.append("</" + arith[len] + ">");
				break;
			default:
		}
		sb.append("</" + elemName + ">");
	}
}
