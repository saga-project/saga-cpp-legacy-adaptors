#!/usr/bin/env python

from optparse import OptionParser
from optparse import OptionGroup

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
import time
import saga
import sys
import os


if __name__ == "__main__":

  try:
    jobstat_dir = saga.advert.directory("###JOBSTAT_URL###")
    status = list(jobstat_dir.get_vector_attribute("###COMPONENT_ID###"))
  
    workspace = status[0] # the url of our working directory     
    
    t = strftime("STIME:%a, %d %b %Y %H:%M:%S +0000", gmtime())
    status[1] = "STATUS:ONLINE"
    status[2] = t
    jobstat_dir.set_vector_attribute("###COMPONENT_ID###", status)
    
    time.sleep(5)
    
    t = strftime("ETIME:%a, %d %b %Y %H:%M:%S +0000", gmtime())
    status[1] = "STATUS:DONE"
    status[3] = t
    jobstat_dir.set_vector_attribute("###COMPONENT_ID###", status)
  
    jobstat_dir.close()


  except saga.exception:
    traceback.print_exc(file=sys.stderr)

"""

################################################################################
##
class Component (Exception):
  """Represents a component"""
  
  def __init__(self, id, manager):
    """Constructor"""
    self.id        = id
    self.manager = manager

    # Create the source code
    self.sourcecode = CODE_TEMPLATE
    self.sourcecode = self.sourcecode.replace("###JOBSTAT_URL###", manager.master_dir.get_url().url)
    self.sourcecode = self.sourcecode.replace("###COMPONENT_ID###", id)
    
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
    self.master_dir = saga.advert.directory(self.adverturl+"/jobstats")
    
  def __del__(self):
    """Destructor"""
    self.master_dir.close()
    
      
  def createComponent(self, id):
    """Creates a new component"""
    comp_id = "comp"+id
    logging.info('CM: Creating component object for: '+comp_id)
    component = Component(id=comp_id, manager=self)
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
  
  

  group = OptionGroup(parser, "Benchmark Options",
                    "These options control the various parameters of the benchmark.")

  group.add_option("-c", "--components", dest="numcomponents", type="int", metavar="COUNT",
                  help="Number of concurrent components [default: %default]", default=2
                  )

  group.add_option("-a", "--attributes", dest="numattributes", type="int", metavar="COUNT",
                  help="Number of attributes per component [default: %default]", default=1
                  )

  group.add_option("-i", "--iterations", dest="numiterations", type="int", metavar="COUNT",
                  help="Number of iterations per component [default: %default]", default=1
                  )
                  
  parser.add_option_group(group)


  group_r = OptionGroup(parser, "Run Options",
                    "These options control were the components are executed.")

  group_r.add_option("-v", "--verbose",
                  action="store_true", dest="verbose", default=True,
                  help="Make lots of noise [default: %default]")

  group_r.add_option("-p", "--purge", action="store_true", dest="purge", default=False,
                     help="Purge the advert directory before running in case it already exists."
                     )
                     
  group_r.add_option("-j", "--jobmanager", dest="jobmanager", type="string", metavar="URL",
                  help="Endpoint URL of a remote job manager [default: %default]", default="fork://localhost"
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
        cm.createComponent(str(c))
    
    cm.startAll(jobmanager=options.jobmanager);
    
    
    #for c in cm.listComponents():
    #  print "Component: " + str(c.getID()) + " - state: " + c.getState() + " - endpoint: " + c.getAdvertEndpoint();
  
  except:
    traceback.print_exc(file=sys.stderr)
  