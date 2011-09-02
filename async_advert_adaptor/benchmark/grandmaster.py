#!/usr/bin/env python

from optparse import OptionParser

import traceback
import sys
import time
import os

import saga

class BenchmarkMaster (Exception):
  """The wrapper class for the benchmark master process"""

  def __init__(self, adverturl, numworkers):
    """Document Me"""
    self.adverturl = saga.url(adverturl)
    self.numworkers = numworkers

  def __del__(self):
    """Document Me"""
    #

  def start(self):
    """Document Me"""
    base_path = self.adverturl.path
    self.adverturl.path = ""
    print "Benchmark base path: "+base_path
    if base_path == "/":
      print "Benchmark base path can't be the advert root. please define a subdirectory.\n"
      sys.exit(-1)

    d = saga.advert.directory(self.adverturl)
    if d.exists(base_path) :
      print "Base path already exists. Purging..."
      d.remove(base_path, saga.advert.Recursive)
    
    
    base_url = self.adverturl
    base_url.path = base_path
    master_dir = d.open_dir(base_url, saga.advert.CreateParents | saga.advert.Create)
        
    # create subdirectories 
    for i in range(self.numworkers):
      s = saga.url(base_url.url)
      s.path += "/"+str(i)
      print "Creating dir: "+s.url

      worker_dir = master_dir.open_dir(s, saga.advert.Create)
      print "YYY"

      worker_dir.set_vector_attribute("WORK", ["ss","tt"])
      
    
if __name__ == "__main__":
  """Program entry point"""
  
  parser = OptionParser()
  
  # add program parameter / options
  parser.add_option("-u", "--url", dest="adverturl",
                  help="The url of the advert server", metavar="URL")  
  parser.add_option("-n", "--numworker", dest="numworker", type="int",
                  help="Number of worker agents", metavar="COUNT")
  parser.add_option("-i", "--increment", dest="increment",
                  help="Document me ", metavar="COUNT", default=10)
  parser.add_option("-m", "--updateinterval", dest="updateinterval",
                  help="Document me ", metavar="COUNT", default=10)
  parser.add_option("-p", "--pollinginterval", dest="pollinginterval",
                  help="Document me ", metavar="COUNT", default=10)
  (options, args) = parser.parse_args()
  
  # check required args
  if not options.adverturl : 
    print "\nerror: the -u/--url parameter is required."
    sys.exit(-1)  
  if not options.numworker : 
    print "\nerror: the -n/--numworker parameter is required."
    sys.exit(-1)
    
  # now we're good to start the master
  try:
    bm = BenchmarkMaster(adverturl=options.adverturl, numworkers=options.numworker)
    bm.start()
  
  except:
    print "Problem running the master"
    traceback.print_exc(file=sys.stderr)
  