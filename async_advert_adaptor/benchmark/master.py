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
    d = saga.advert.directory(self.adverturl)
    print d.is_dir(".")
    
    del d

if __name__ == "__main__":
  """Program entry point"""
  
  parser = OptionParser()
  
  # add program parameter / options
  parser.add_option("-u", "--url", dest="adverturl",
                  help="The url of the advert server", metavar="URL")  
  parser.add_option("-n", "--numworker", dest="numworker",
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
    bm = BenchmarkMaster(adverturl=options.adverturl, numworkers=16)
    bm.start()
  
  except:
    print "Problem running the master"
    traceback.print_exc(file=sys.stderr)
  