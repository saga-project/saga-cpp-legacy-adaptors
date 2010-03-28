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
 * $RCSfile: CreateCommand.java,v $ $Revision: 1.2 $ $Date: 2005/11/01 07:46:58 $
 */
package org.apgrid.grpc.tools.invokeServer.unicore;
import java.io.*;
import java.util.*;

class CreateCommand extends Command implements Runnable{
  class Attribute {
	String key;
	String value;
	Attribute(String key, String value){
	  this.key = key;
	  this.value = value.trim();
	}
  }


  class AttributeList {
	ArrayList <Attribute> list = new ArrayList<Attribute>();
	void put(String key, String value){
	  list.add(new Attribute(key, value));
	}

	String get(String key){
	  return get(key, 0);
	}

	String get(String key, int occurence){
	  int counter = 0;
	  for (Attribute a: list){
		if (a.key.equals(key)){
		  if (counter >= occurence)
			return a.value;
		  else
			counter++;
		}
	  }
	  return null;
	}
  }


  private static final String CMD_CREATE_END = "JOB_CREATE_END";

  String requestId;
  AttributeList list = new AttributeList();
  boolean terminated = false;
  UnicoreJob job;

  CreateCommand(UnicoreInvokeServer is, String requestId) throws Command.Exception{
	super(is);
	this.requestId = requestId;
	String line;
	while ((line = is.readLine()) != null){
	  if (line.trim().equals(CMD_CREATE_END)){
		terminated = true;
		break;
	  }
	  else {
		try {
		  StringTokenizer st = new StringTokenizer(line);
		  String key = st.nextToken();
		  String value = line.substring(key.length());
		  list.put(key, value);
		} catch (java.lang.Exception e){
		  Log.printStackTrace(e);
		}
	  }
	}
	if (terminated){
	  job = new UnicoreJob(this);
	} else {
	  new Command.Exception("the command was not terminated!");
	}
  }

  /** create UnicoreJob and put it to the list */
  public void run(){
	if (job.start())
	  is.registerJob(job);
  }


  void handle(){
	if (terminated){
	  (new Thread(this)).start();
	  reply("S");
	}
	else
	  reply("F failed to read command, somehow");
  }
  
  public String toString(){
	return "CreateCommand: job = \n"  + job;
  }

  void dump(PrintWriter pw){
	pw.println("CreateCommand");
	pw.println(job);
  }

}

