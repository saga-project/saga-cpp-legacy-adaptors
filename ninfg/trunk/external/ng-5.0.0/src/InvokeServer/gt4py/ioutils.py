
# vi:set ts=3 sw=3:
# vim:set sts=0 noet:

# $RCSfile: ioutils.py,v $ $Revision: 1.1 $ $Date: 2007/01/16 04:13:56 $
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

import os

class LineScanner(object):
	def __init__(self, fd):
		self.fd = fd
		self.fifo = []
		self.rest = ''
	def _readMore(self):
		len_before = len(self.rest)
		self.rest += os.read(self.fd, 4096)
		eof_flag = False
		if len(self.rest) == len_before:
			eof_flag = True
		lines = self.rest.splitlines(True)
		n = len(lines) - 1
		if n >= 0:
			idx = 0
			while idx < n:
				self.fifo.append(lines[idx])
				idx += 1
			last = lines[n]
			if len((last + ' ').splitlines()) > 1:
				self.fifo.append(last)
				self.rest = ''
			else:
				self.rest = last
		if eof_flag:
			self.fifo.append(self.rest)
			if self.rest:
				self.fifo.append('')
				self.rest = ''
	def readLine(self):
		while len(self.fifo) == 0:
			self._readMore()
		return self.fifo.pop(0)

