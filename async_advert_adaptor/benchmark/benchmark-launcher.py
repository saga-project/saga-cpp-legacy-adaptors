#!/usr/bin/env python

from optparse import OptionParser
from optparse import OptionGroup

import traceback
import sys
import time
import os
import logging

import saga

################################################################################
##
class Component (Exception):
  """Represents a component"""
  
  def __init__(self, id, advertendpoint):
    """Constructor"""
    self.id        = id
    self.state     = "new" 
    self.advertendpoint = advertendpoint
    
  def getState(self):
    """Returns the current component state"""
    return self.state
    
  def getID(self):
    """Returns the component ID"""
    return self.id
    
  def getAdvertEndpoint(self):
    """Returns the advert endpoint for this component"""
    return self.advertendpoint
    
  def run(self, jobmanager):
    """Runs the component via SAGA as a job on a given jobmanager"""
    try:
      print "RUN"
    
    except saga.exception:
      print "ERROR"
    


################################################################################
##
class ComponentManager (Exception):
  """Launches the components."""
  
  def __init__(self, jobmanager, count): 
    """Constructor"""
    self.jobmanager = jobmanager
    self.count = count
    
    self.complist = [] # holds the components 
    for c in range(count): 
      component = Component(id=c, advertendpoint="advert")
      self.complist.append(component)
      
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
      vector = ["OFFLINE", "STIME:0","ETIME:0"]
      logging.info('Adding namespace for component comp' + str(i) + ' to \'jobstats\': ' + str(vector))
      params.set_vector_attribute("comp"+str(i), vector)
      

    
    #master_dir.set_attribute("POLL_INTERVAL", "1000")
    #master_dir.set_attribute("MAX_COUNT", "10")

    # create subdirectories 
    #for i in range(self.numworkers):
    #  s = saga.url(base_url.url)
    #  s.path += "/"+str(i)
      #print "Creating dir: "+s.url

    #  worker_dir = master_dir.open_dir(s, saga.advert.Create)
    #  worker_dir.set_attribute("WORK", "0")
      
    
    
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
  
    cm = ComponentManager(jobmanager=options.jobmanager, count=options.numcomponents)
    for c in cm.listComponents():
      print "Component: " + str(c.getID()) + " - state: " + c.getState() + " - endpoint: " + c.getAdvertEndpoint();
  
  except:
    traceback.print_exc(file=sys.stderr)
  