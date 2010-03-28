
# vi:set ts=3 sw=3:
# vim:set sts=0 noet:

# $RCSfile: uuid.py,v $ $Revision: 1.3 $ $Date: 2006/10/11 08:13:50 $
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

"""UUID generator"""

__version__ = "$Revision: 1.3 $"

import random
import sha


def randStr512(randGen):
		strList = []
		for _ in xrange(16):
			r = randGen.randint(0, 0xFFFFFFFFL)
			strList.append(chr(r & 0xFF))
			r >>= 8
			strList.append(chr(r & 0xFF))
			r >>= 8
			strList.append(chr(r & 0xFF))
			strList.append(chr((r >> 8) & 0xFF))
		return ''.join(strList)

def UUIDGen():
	"""A factory of the UUID generator for UUID (ver 4, see RFC 4122) ."""
	randGen = random.Random()
	randGen.seed()
	hashGen = sha.new(randStr512(randGen))
	while 1:
		hashGen.update(randStr512(randGen))
		hashed = hashGen.digest()
		yield '%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x' % (
			ord(hashed[0]), ord(hashed[1]), ord(hashed[2]), ord(hashed[3]),
			ord(hashed[4]), ord(hashed[5]),
			ord(hashed[6]) & 0x0F | 0x40, ord(hashed[7]),
			ord(hashed[8]) & 0x3F | 0x80, ord(hashed[9]),
			ord(hashed[10]), ord(hashed[11]),
			ord(hashed[12]), ord(hashed[13]), ord(hashed[14]), ord(hashed[15]) )

