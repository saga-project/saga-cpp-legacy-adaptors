
# vi:set ts=3 sw=3:
# vim:set sts=0 noet:

# $RCSfile: ng_invoke_server.GT4.py,v $ $Revision: 1.2 $ $Date: 2007/02/02 03:15:55 $
# $AIST_Release: 5.0.0 $
# $AIST_Copyright:
#  Copyright 2003, 2004, 2005, 2006 Grid Technology Research Center,
#  National Institute of Advanced Industrial Science and Technology
#  Copyright 2003, 2004, 2005, 2006 National Institute of Informatics
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#  $

__version__ = "$Revision: 1.2 $"

import getopt
import os
import sys
import select

# version check
if sys.version < "2.3":
	errorMessage = "Invoke Server GT4py requires python version >= 2.3."
	print errorMessage
	sys.exit(errorMessage)

import gt4invokeserver
import ngutils

# poll() check
try:
	poller = select.poll()
	del poller
except:
	errorMessage = "select.poll() do not work on this python. exit."
	print errorMessage
	sys.exit(errorMessage)

def open_dup2(path, mode, fd):
	"""Open path, and dup to fd."""
	nfd = os.open(path, mode)
	os.dup2(nfd, fd)
	os.close(nfd)

def pipe_w_dup2(fd):
	"""Pipe, and dup the endpoint for writing to fd."""
	fd_r, fd_w = os.pipe()
	os.dup2(fd_w, fd)
	os.close(fd_w)
	return fd_r

optinfo, rest = getopt.getopt(sys.argv[1:], 'l:')
if (optinfo is None) or (rest is None):
        sys.exit(2)
if len(rest) > 0:
        sys.exit(2)
if len(optinfo) > 1:
        sys.exit(2)
log_path = None
if len(optinfo) == 1:
        log_path = optinfo[0][1]

try:  # make pipe for redirection, and fork
	ofd_r, ofd_w = os.pipe()
	efd_r, efd_w = os.pipe()
	pid = os.fork()
except:
	sys.exit(2)
if pid == 0:
	# logger process
	try:
		os.close(ofd_w)
		os.close(efd_w)
		open_dup2('/dev/null', os.O_RDONLY, 0)
		log_fd = os.open('/dev/null', os.O_WRONLY)
		try:
			if log_path:
				fd = os.open(log_path, os.O_WRONLY | os.O_APPEND | os.O_CREAT, 0666)
				if fd >= 0:
					try:
						os.dup2(fd, log_fd)
					finally:
						os.close(fd)
		except:
			pass
		self_ofd = pipe_w_dup2(1)
		self_efd = pipe_w_dup2(2)
	except:
		os._exit(2)
	dispatcher = ngutils.Dispatcher()
	def write_to_log(line):
		os.write(log_fd, line)
	dispatcher.enter(ofd_r, write_to_log)
	dispatcher.enter(efd_r, write_to_log)
	dispatcher.enter(self_ofd, write_to_log)
	dispatcher.enter(self_efd, write_to_log)
	while len(dispatcher.buffers) > 2:
		dispatcher.poll_dispatch(-1)
	os.close(self_ofd)
	os.close(self_efd)
	os.close(log_fd)
	os._exit(0)
elif pid > 0:
	# main process
	try:
		os.close(ofd_r)
		os.close(efd_r)
		cmd_fd = os.dup(0)
		open_dup2('/dev/null', os.O_RDONLY, 0)
		resp_fd = os.dup(1)
		os.dup2(ofd_w, 1)
		os.close(ofd_w)
		notif_fd = os.dup(2)
		os.dup2(efd_w, 2)
		os.close(efd_w)
	except:
		sys.exit(2)
	# make and start server
	server = gt4invokeserver.GT4InvokeServer(cmd_fd, resp_fd, notif_fd)
	server.start()
	# exit
	sys.exit(0)
else:
	sys.exit(2)

