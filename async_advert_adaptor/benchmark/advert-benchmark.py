#!/usr/bin/env python

from optparse import OptionParser
from optparse import OptionGroup

import random
import traceback
import logging
import time
import saga
import sys
import os

CODE_TEMPLATE = """#!/usr/bin/env python

from time import gmtime, strftime


import traceback
import logging
import random
import string
import time
import saga
import sys
import os

def random_str(n):
    return "".join([random.choice(string.ascii_lowercase)
        for x in xrange(n)])

if __name__ == "__main__":

  logfilename = "advbench-###COMPONENT_ID###-"+str(os.getpid())+".log"

  logging.basicConfig(filename="###LOGDIR###/"+logfilename,
                      level=logging.DEBUG, 
                      format='%(asctime)s %(message)s', 
                      datefmt='%m/%d/%Y %I:%M:%S %p')

  logging.info("================== COMPONENT START ==================")
  
  try:
   
    logging.info("pid: " + str(os.getpid()))

    logging.info("connecting to advert entry: ###PARAMS_URL###")
    params = saga.advert.entry("###PARAMS_URL###", saga.advert.ReadWrite)
    numiterations = params.get_attribute("numiterations")
    numattributes = params.get_attribute("numattributes")
    params.close()

    logging.info("connecting to advert entry: ###JOBSTAT_URL###")
    jobstat = saga.advert.entry("###JOBSTAT_URL###", saga.advert.ReadWrite)
    status = list(jobstat.get_vector_attribute("###COMPONENT_ID###"))
  
    workspace = status[0].replace("WORKSPACE:","") # the url of our working directory     
    
    t = strftime("STIME:%a, %d %b %Y %H:%M:%S +0000", gmtime())
    status[1] = "STATUS:ONLINE"
    status[2] = t
    logging.info("setting status to ONLINE")
    jobstat.set_vector_attribute("###COMPONENT_ID###", status)

    logging.info("creating workspace at: " + workspace)
    workdir = saga.advert.directory(workspace, saga.advert.Create | saga.advert.CreateParents | saga.advert.ReadWrite)
    
    ###### STARTUP PROCESS
    ##
    startup_start_time = time.time()
    
    logging.info("populating workspace with " + str(numattributes) + " attributes")
    for a in range(int(numattributes)):
      key   = str(a)
      value = random_str(16)
      logging.info("attribute " + key + " : " + value)
      workdir.set_attribute(key, value)
      
    startup_duration = (time.time() - startup_start_time)
    logging.info("startup (workspace creation) took" + str(startup_duration) + " seconds")
    ##
    #########################

    ###### SWAP ITERATIONS
    ##
    iteration_durations = []
    
    for i in range(int(numiterations)):
      start_time = time.time()
    
      logging.info("sleeping for five seconds - iteration: " + str(i))
      time.sleep(1)      
      
      duration = (time.time() - start_time)
      logging.info("iteration took " + str(duration)+ " seconds")
    
      iteration_durations.append("ITERTIME"+str(i)+":"+str(duration))
    ##
    #########################

    ###### SHUTDOWN PROCESS
    ##

    t = strftime("ETIME:%a, %d %b %Y %H:%M:%S +0000", gmtime())
    status[3] = t
    
    logging.info("writing measurements to 'jobstats' advert & setting status to DONE")
    for i in iteration_durations:
      status.append(str(i))
    
    status[1] = "STATUS:DONE"
    
    jobstat.set_vector_attribute("###COMPONENT_ID###", status)
  
    jobstat.close()
    ##
    #########################

    
  except saga.exception, e:
    logging.error("OH NOES!! A SAGA Exception: "+str(e.get_all_messages()))
    sys.exit(-1)
    
  except Exception, e:
    logging.error("OH NOES!! An Exception: "+str(e))
    sys.exit(-1)
    
  logging.info("================== COMPONENT STOP ==================")
  sys.exit(-1)
"""

################################################################################
##
class Component (Exception):
  """Represents a component"""
  
  def __init__(self, id, manager, logdir):
    """Constructor"""
    self.id      = id
    self.manager = manager
    self.logdir  = logdir

    # Create the source code
    self.sourcecode = CODE_TEMPLATE
    self.sourcecode = self.sourcecode.replace("###JOBSTAT_URL###", manager.getJobstatDirAsString())
    self.sourcecode = self.sourcecode.replace("###PARAMS_URL###", manager.getParamsDirAsString())
    self.sourcecode = self.sourcecode.replace("###COMPONENT_ID###", self.id)
    self.sourcecode = self.sourcecode.replace("###LOGDIR###", self.logdir)
    
  def getPythonSource(self):
    """Introspection: Returns the "worker" Python code"""
    return self.sourcecode
    
  def getState(self):
    """Returns the current component state"""
    return self.state
    
  def getID(self):
    """Returns the component ID"""
    return self.id
    
  def getAdvertEndpoint(self):
    """Returns the advert endpoint for this component"""
    return self.advertendpoint
    
  def start(self, jobmanager):
    """Runs the component via SAGA as a job on a given jobmanager"""
    try:
      jd = saga.job.description()
      jd.set_attribute("Executable", "python")
      args = list()
      args.append("-c")
      args.append(self.getPythonSource())
      jd.set_vector_attribute("Arguments", args)
            
      js = saga.job.service(jobmanager)
      j = js.create_job(jd)
      j.run()
      
      logging.info(self.id+': Launching on ' + jobmanager)
    
    except saga.exception, e:
      for err in e.get_all_messages():
        print err

    


################################################################################
##
class ComponentManager (Exception):
  """Launches the components."""
  
  def __init__(self, adverturl): 
    """Constructor"""
    self.adverturl = adverturl
    self.complist = []
    
    logging.info('CM: Connecting to advert endpoint: '+self.adverturl)
    self.jobstat_dir = saga.advert.entry(self.adverturl+"/jobstats")
    self.params_dir = saga.advert.entry(self.adverturl+"/params")

  def getJobstatDirAsString(self):
    return self.jobstat_dir.get_url().url
    
  def getParamsDirAsString(self):
    return self.params_dir.get_url().url
    
  def __del__(self):
    """Destructor"""
    self.jobstat_dir.close()
    self.params_dir.close()
    
      
  def createComponent(self, id, logdir):
    """Creates a new component"""
    comp_id = "comp"+id
    logging.info('CM: Creating component object for: '+comp_id)
    component = Component(id=comp_id, manager=self, logdir=logdir)
    self.complist.append(component)
    return component

      
  def startAll(self, jobmanager):
    """Launch (start) the components using the given jobmanager""" 
    for c in self.complist:
      c.start(jobmanager=jobmanager)
      
  def listComponents(self):
    return self.complist


################################################################################
##
class BenchmarkMaster (Exception):
  """The wrapper class for the benchmark master process"""

  def __init__(self, adverturl):
    """Document Me"""
    self.adverturl = saga.url(adverturl)

  def __del__(self):
    """Document Me"""
    #

  def setup(self, numcomponents, numattributes, numiterations, purge=False):
    """Document Me"""
    
    logging.info("================== BEGIN SETUP PHASE ==================")

    logging.info('Setting up advert benchmark at: ' + self.adverturl.url)

    base_path = self.adverturl.path
    self.adverturl.path = ""

    if base_path == "/":
      logging.info("Benchmark base directory cannot be '/'. Please add a subdirectory to the URL.\n")
      sys.exit(-1)

    d = saga.advert.directory(self.adverturl)
    if d.exists(base_path):
      if False == purge: 
        logging.error('The directory \'' + base_path + '\' already exists. Use the -p/--purge option to delete it explicitly.' )
        sys.exit(-1)
      else:
        logging.info('Purging directory \'' + base_path + '\' (already exists).')
        d.remove(base_path, saga.advert.Recursive)
    
    base_url = self.adverturl
    base_url.path = base_path
    master_dir = d.open_dir(base_url, saga.advert.CreateParents | saga.advert.Create)
        
    logging.info('Creating \'params\' entry in '+master_dir.get_url().url)
    params = master_dir.open("params", saga.advert.Create)
    
    logging.info('Adding benchmark parameter \'numcomponents\' = '+str(numcomponents)+ ' to \'params\'')
    params.set_attribute("numcomponents", str(numcomponents))
    logging.info('Adding benchmark parameter \'numattributes\' = '+str(numattributes)+ ' to \'params\'')
    params.set_attribute("numattributes", str(numattributes))
    logging.info('Adding benchmark parameter \'numiterations\' = '+str(numiterations)+ ' to \'params\'')
    params.set_attribute("numiterations", str(numiterations))
    
    logging.info('Creating \'jobstats\' entry in '+master_dir.get_url().url)
    jobstatus = master_dir.open("jobstats", saga.advert.Create)

    for i in range(numcomponents):
      workspace = master_dir.get_url()
      workspace.path += "/comp"+str(i)
      
      vector = ["WORKSPACE:"+workspace.url, "STATUS:OFFLINE", "STIME:0","ETIME:0"]
      logging.info('Adding \'comp' + str(i) + ' \'to \'jobstats\': ' + str(vector))
      jobstatus.set_vector_attribute("comp"+str(i), vector)
      
    logging.info("================== END SETUP PHASE ==================")
    
    params.close()
    master_dir.close()
    

if __name__ == "__main__":
  """Program entry point"""
  
  logging.basicConfig(level=logging.DEBUG, 
                      format='%(asctime)s %(message)s', 
                      datefmt='%m/%d/%Y %I:%M:%S %p')
  
  usage = "usage: %prog [options] advert-url"
  parser = OptionParser(usage=usage, version="%prog 1.0")
  
  # add program parameter / options
  
  
  parser.add_option("-v", "--verbose",
                  action="store_true", dest="verbose", default=True,
                  help="make lots of noise [default: %default]")


  group = OptionGroup(parser, "Benchmark Options",
                    "These options control the benchmark parameters. The benchmark is based on 'components', saga processes (jobs) which connect back to the advert service and produce specific load and access pattern.")

  group.add_option("-c", "--components", dest="numcomponents", type="int", metavar="COUNT",
                  help="number of concurrent components [default: %default]", default=2
                  )

  group.add_option("-a", "--attributes", dest="numattributes", type="int", metavar="COUNT",
                  help="number of attributes per component [default: %default]", default=1
                  )

  group.add_option("-i", "--iterations", dest="numiterations", type="int", metavar="COUNT",
                  help="number of iterations per component [default: %default]", default=1
                  )
                  
  parser.add_option_group(group)


  group_r = OptionGroup(parser, "Run Options",
                    "These options control how and where the components are executed.")


  parser.add_option("-p", "--purge", action="store_true", dest="purge", default=False,
                     help="purge the advert service directory before running in case it already exists [default: %default]"
                     )
                     
  group_r.add_option("--jobmanager", dest="jobmanager", type="string", metavar="URL",
                  help="url of a (remote) job manager that is used to run the components [default: %default]", default="fork://localhost"
                  )

  group_r.add_option("--logdir", dest="logdir", type="string", metavar="PATH",
                  help="directory that is used by the components to write their local log-files [default: %default]", default="/tmp"
                  )
                  
  group_r.add_option("--logprefix", dest="logprefix", type="string", metavar="STRING",
                  help="string that is prepended to the component log files [default: %default]", default="<PID>"
                  )
                  
  parser.add_option_group(group_r)


  (options, args) = parser.parse_args()
  
  if len(args) < 1:
    parser.error("advert-url is required")
  elif len(args) > 1:
    parser.error("don't understand arguments other than advert-url")
  else:
    options.adverturl = args[0]
  
  try:
  
    bm = BenchmarkMaster(adverturl=options.adverturl)
    bm.setup(purge=options.purge, numcomponents=options.numcomponents,
             numattributes=options.numattributes, numiterations=options.numiterations)
  
    cm = ComponentManager(adverturl=options.adverturl)
    for c in range(options.numcomponents):
        cm.createComponent(str(c), options.logdir)
    
    cm.startAll(jobmanager=options.jobmanager);
    
    
    #for c in cm.listComponents():
    #  print "Component: " + str(c.getID()) + " - state: " + c.getState() + " - endpoint: " + c.getAdvertEndpoint();
  
  except:
    traceback.print_exc(file=sys.stderr)
  
