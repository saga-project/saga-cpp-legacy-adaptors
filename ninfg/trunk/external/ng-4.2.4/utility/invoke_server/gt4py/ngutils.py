
# vi:set ts=3 sw=3:
# vim:set sts=0 noet:

# $RCSfile: ngutils.py,v $ $Revision: 1.6 $ $Date: 2006/10/11 08:13:50 $
# $AIST_Release: 4.2.4 $
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

"""Utilities for NG4 invoke server."""

__version__ = "$Revision: 1.6 $"

import os
import select
import sys
import tempfile
import time


class Buffer(object):
	"""The line buffer for dispatcher."""

	def __init__(self, readlineCallback):
		self.buf = ''
		self.readlineCallback = readlineCallback

	def write(self, str):
		"""Append str to the buffer.

		Append str to the buffer.  If the buffer has more than one line,
		drop line and call callback with the content of the line.

		"""

		self.buf += str
		lines = self.buf.splitlines(True)
		if lines:
			last = len(lines) - 1
			for i in xrange(last):
				self.readlineCallback(lines[i])
			lastline = lines[last]
			if len((lastline + ' ').splitlines()) > 1:
				self.readlineCallback(lastline)
				self.buf = ''
			else:
				self.buf = lastline
		else:
			self.buf = ''

	def flush(self):
		"""Flush the buffer anyway."""
		if self.buf:
			self.readlineCallback(self.buf)
			self.buf = ''

	def close(self):
		"""Dispose the buffer."""
		self.readlineCallback(self.buf)
		if self.buf:
			self.readlineCallback('')
		self.buf = None
		self.readlineCallback = None


class Dispatcher(object):
	"""The class that handle file descriptors and associated callbacks."""

	def __init__(self):
		try:
			self.poller = select.poll()
		except:
			exit(2)
		self.buffers = {}

	def enter(self, fd, readlineCallback):
		"""Register a file descriptor and the associated callback."""
		self.buffers[fd] = Buffer(readlineCallback)
		self.poller.register(fd, select.POLLIN | select.POLLPRI)

	def poll_dispatch(self, timeout):
		"""Do poll, and dispatch to callback."""
		ls = self.poller.poll(timeout)
		for fd, ev in ls:
			doClose = False
			if ev & (select.POLLIN | select.POLLPRI):
				buf = os.read(fd, 65536)
				if buf:
					self.buffers[fd].write(buf)
				else:
					doClose = True
			elif ev & (select.POLLERR | select.POLLHUP | select.POLLNVAL):
				doClose = True
			if doClose:
				self.buffers[fd].close()
				self.poller.unregister(fd)
				os.close(fd)
				del self.buffers[fd]


class ProcManager(object):
	"""The class that fork process and dispatch exiting status callback."""
	pid_callbacks = {}

	def start_proc(func, callback):
		"""Do fork, and call func in child process."""
		pid = None
		try:
			pid = os.fork()
		except:
			return False
		if pid == None:
			return False
		if pid < 0:
			return False
		elif pid == 0:
			try:
				func() # we expect that do exec
			finally:
				os._exit(0)
		else:
			ProcManager.pid_callbacks[pid] = callback
			return True
	start_proc = staticmethod(start_proc)

	def wait_any():
		"""If any child process exited, get exiting status and call callback."""
		if ProcManager.pid_callbacks:
			pid, stat = os.waitpid(-1, os.WNOHANG)
			while not ((pid == 0) and (stat == 0)):
				callback = ProcManager.pid_callbacks[pid]
				del ProcManager.pid_callbacks[pid]
				callback(stat)
				if not ProcManager.pid_callbacks:
					break
				pid, stat = os.waitpid(-1, os.WNOHANG)
	wait_any = staticmethod(wait_any)


def backquote(cmd, args, callback):
	"""Execute command, and calls callback with status and string of stdout output."""
	fd, pathstr = tempfile.mkstemp()
	def ex():
		os.dup2(fd, 1)
		os.close(fd)
		os.execv(cmd, args)
	def cb(stat):
		fd2 = os.open(pathstr, os.O_RDONLY)
		try:
			os.remove(pathstr)
			buf = []
			tmp = os.read(fd2, 65536)
			while tmp:
				buf.append(tmp)
				tmp = os.read(fd2, 65536)
		finally:
			os.close(fd2)
		callback(stat, ''.join(buf))
	try:
		return ProcManager.start_proc(ex, cb)
	finally:
		os.close(fd)

def ppopen(cmd, args):
	"""Execute command, and returns file descriptor which associated with stdout of new process."""
	fd_r, fd_w = os.pipe()
	def ex():
		os.close(fd_r)
		os.dup2(fd_w, 1)
		os.close(fd_w)
		os.execv(cmd, args)
	def cb(stat):
		pass
	try:
		if ProcManager.start_proc(ex, cb):
			return fd_r
		else:
			return None
	finally:
		os.close(fd_w)

def ppopen2(cmd, args):
	"""Execute command, and returns file descriptor which associated with stdout of new process."""
	fd_r, fd_w = os.pipe()
	def ex():
		os.close(fd_r)
		os.dup2(fd_w, 2)
		os.close(fd_w)
		os.execv(cmd, args)
	def cb(stat):
		pass
	try:
		if ProcManager.start_proc(ex, cb):
			return fd_r
		else:
			return None
	finally:
		os.close(fd_w)


class EventPoolEntry(object):

	def __init__(self, priority, val):
		self.priority = priority
		self.val = val

	def __repr__(self):
		return 'priority=%s val=%s' % (repr(self.priority), repr(self.val))


class EventPool(object):

	def __init__(self):
		self.evpool = []

	def enter(self, entry):
		i = len(self.evpool)
		while i:
			tmp = i - 1
			if self.evpool[tmp].priority <= entry.priority:
				break
			i = tmp
		self.evpool.insert(i, entry)

	def pickup(self, priority):
		result = []
		while self.evpool:
			if self.evpool[0].priority <= priority:
				result.append(self.evpool.pop(0).val)
			else:
				break
		return result


class EventTimer(object):
	pool = EventPool()

	def enter(cls, delta, callback):
		t = time.time()
		wakeuptime = t + delta
		cls.pool.enter(EventPoolEntry(wakeuptime, callback))
	enter = classmethod(enter)

	def action(cls):
		currenttime = time.time()
		actions = cls.pool.pickup(currenttime)
		if actions:
			for callback in actions:
				callback()
			return True
		else:
			return False
	action = classmethod(action)

