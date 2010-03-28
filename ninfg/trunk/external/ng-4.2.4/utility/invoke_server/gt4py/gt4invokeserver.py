
# vi:set ts=3 sw=3:
# vim:set sts=0 noet:

# $RCSfile: gt4invokeserver.py,v $ $Revision: 1.48 $ $Date: 2006/10/11 08:13:50 $
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

"""NG4 invoke server for GT4."""

__version__ = "$Revision: 1.48 $"

import os
import re
import sets
import sys
import tempfile
#import xml.sax.saxutils

import ngutils
import uuid

import gt4invokeserverconfig

import logging

GT2_JOB_MANAGERS = {}
GT2_JOB_MANAGERS['jobmanager-fork'] = 'Fork'
GT2_JOB_MANAGERS['jobmanager-pbs'] = 'PBS'
GT2_JOB_MANAGERS['jobmanager-sge'] = 'SGE'
GT2_JOB_MANAGERS['jobmanager-condor'] = 'Condor'
GT2_JOB_MANAGERS['jobmanager-lsf'] = 'LSF'

def setup_logger():
	handler = logging.StreamHandler()
	handler.setFormatter(logging.Formatter('%(asctime)s %(msecs)dms: %(levelname)s: %(message)s', '%a %b %d %T %Y'))
	logging.getLogger().addHandler(handler)
	handler = None
	del handler
	logging.getLogger().setLevel(logging.DEBUG)
setup_logger()
del setup_logger


ESCAPED_QUOTE = ''.join(["'", '"', "'", '"', "'"])
def escape_quote(str):
	return ''.join(["'", str.replace("'", ESCAPED_QUOTE), "'"])

def expand_args(args):
	return ' '.join([escape_quote(arg) for arg in args])


class Sender(object):
	fd = None

	def send(cls, str):
		sender = '<dummy>'
		if cls == Responder:
			sender = 'Responder'
		elif cls == Notifier:
			sender = 'Notifier'
		else:
			sender = repr(cls)
		logging.getLogger().debug('%s: %s' % (sender, repr(str)))
		if cls.fd != None:
			os.write(cls.fd, str)
	send = classmethod(send)


class Responder(Sender):
	pass


class Notifier(Sender):
	pass


class Dispatcher(object):
	"""static holder of dispatcher"""
	dispatcher = None

	def enter(cls, fd, handler):
		if cls.dispatcher:
			cls.dispatcher.enter(fd, handler)
	enter = classmethod(enter)

	def poll_dispatch(cls, timeout):
		if cls.dispatcher:
			cls.dispatcher.poll_dispatch(timeout)
	poll_dispatch = classmethod(poll_dispatch)


def format_xml(input, out):
	"""This procedure is ported from Ruby code in http://namazu.org/~satoru/diary/20040824.html ."""
	if isinstance(input, list):
		offset = 1
		element = input[0]
		attributes = ''
		if len(input) > 1 and isinstance(input[1], dict):
			attr = input[1]
			attributes = ' ' + ' '.join(['%s="%s"' % (str(key), attr[key]) for key in attr])
			offset += 1
		if len(input) > offset and (input[offset] == '' or input[offset]):
			out.write('<%s%s\n>' % (str(element), attributes))
			for i in xrange(offset, len(input)):
				format_xml(input[i], out)
			out.write('</%s\n>' % (str(element)))
		else:
			out.write('<%s%s /\n>' % (str(element), attributes))
	else:
		out.write(str(input))


class Delegation(object):

	def __init__(self, attrs, cont):
		self.failed = True
		self.attrs = attrs
		epr_fd, self.filename = tempfile.mkstemp('', 'invoke_server_delegated_credential_epr.')
		os.close(epr_fd)
		del epr_fd
		prog = gt4invokeserverconfig.GT4InvokeServerConfig.GLOBUS_LOCATION + '/bin/globus-credential-delegate'
		args = []
		args.append(prog)
		args.append('-h')
		args.append(self.attrs['hostname'])
		port = self.attrs.get('port')
		if port and port != '0':
			args.append('-p')
			args.append(port)
		if self.attrs.get('protocol') == 'http':
			args.append('-m')
			args.append('msg')
		self.subject = None
		if self.attrs.get('subject') != None:
			self.subject = self.attrs['subject']
			args.append('-a')
			args.append(self.subject)
		args.append(self.filename)

		# exec callback (called in child process of fork)
		def ex():
			fd = os.open('/dev/null', os.O_WRONLY)
			os.dup2(fd, 1)
			os.close(fd)
			logging.getLogger().debug('prog: %s' % prog)
			logging.getLogger().debug('args: %s' % args)
			os.execv(prog, args)

		# process termination callback
		def cb(stat):
			if stat == 0:
				self.failed = False
			cont()

		ngutils.ProcManager.start_proc(ex, cb)

	def destroy(self):
		prog = '/bin/sh'
		args = []
		args.append(prog)
		args.append('-c')
		prog2 = gt4invokeserverconfig.GT4InvokeServerConfig.GLOBUS_LOCATION + '/bin/wsrf-destroy'
		prog3 = gt4invokeserverconfig.GT4InvokeServerConfig.RMCOM
		subargs1 = []
		subargs1.append(prog2)
		subargs1.append('-e')
		subargs1.append(self.filename)
		if self.attrs.get('protocol') == 'http':
			subargs1.append('-m')
			subargs1.append('msg')
		if self.subject:
			subargs1.append('-z')
			subargs1.append(self.subject)
		subargs2 = []
		subargs2.append(prog3)
		subargs2.append(self.filename)
		args.append('; '.join([expand_args(subargs1), expand_args(subargs2)]))
		logging.getLogger().debug('Delegation#destroy args: %s' % args)

		# exec callback (called in child process of fork)
		def ex():
			fd = os.open('/dev/null', os.O_WRONLY)
			os.dup2(fd, 1)
			os.close(fd)
			os.execv(prog, args)

		# process termination callback
		def cb(stat):
			pass

		ngutils.ProcManager.start_proc(ex, cb)


class Job(object):
	"""The class that represent a job."""

	def setup_job(cls, req_id, attrs):
		"""Factory method that returns an instance of Job."""
		if not req_id:
			return None
		if not attrs:
			return None
		if not attrs.get('executable_path'):
			return None
		return cls(req_id, attrs)
	setup_job = classmethod(setup_job)

	def __init__(self, req_id, attrs):
		self.req_id = req_id
		self.attrs = attrs
		self.job_id = None
		self.epr_file_name = None
		self.rsl_file_name = None
		self.status = None
		self.observation_cont = False
		self.delegation = None

	def start_job(self, job_id):
		"""Fork and exec globusrun."""
		# set instance variable
		self.job_id = job_id

		self.status = 'PENDING'
		Notifier.send('CREATE_NOTIFY %s S %s\r\n' % (self.req_id, self.job_id))
		self.notify_status()

		do_delegation = False
		if self.attrs.get('delegate_full_proxy') == 'true':
			do_delegation = True

		if do_delegation :
			self.delegation = Delegation(self.attrs, self._start_job)
		else:
			self._start_job()

	def _start_job(self):
		if self.delegation and self.delegation.failed:
			self.status_handler('FAILED')
			return
		# make XML RSL file
		rsl = self.build_xmlrsl()
		logging.getLogger().debug('rsl: %s' % (repr(rsl),))
		xmlrsl_pathstr = Job.writeout_xmlrsl(rsl);

		# build commandline
		prog = gt4invokeserverconfig.GT4InvokeServerConfig.GLOBUS_LOCATION + '/bin/globusrun-ws'
		args = []
		args.append(prog)
		args.append('-submit')
		args.append('-batch')
		if self.attrs.get('redirect_enable') == 'true':
			args.append('-streaming')
		subject = self.attrs.get('subject')
		if subject:
			args.append('-subject-authz')
			args.append(subject)
		status_polling = self.attrs.get('status_polling')
		if status_polling:
			status_polling = int(status_polling)
			if status_polling:
				args.append('-http-timeout')
				args.append(str(status_polling) + '000')
		hostname = self.attrs.get('hostname')
		if hostname:
			factory = ''
			protocol = self.attrs.get('protocol')
			if protocol:
				factory += protocol
				factory += '://'
			factory += hostname
			port = self.attrs.get('port')
			if port != '0':
				factory += ':'
				factory += port
			args.append('-factory')
			args.append(factory)
		jobmanager = self.attrs.get('jobmanager')
		if jobmanager:
			args.append('-factory-type')
			if GT2_JOB_MANAGERS.has_key(jobmanager):
				args.append(GT2_JOB_MANAGERS[jobmanager])
			else:
				args.append(jobmanager)
		args.append('-submission-id')
		args.append(self.job_id)
		args.append('-job-delegate')
		if self.delegation:
			args.append('-staging-credential-file')
			args.append(self.delegation.filename)
			args.append('-transfer-credential-file')
			args.append(self.delegation.filename)
			if self.attrs.get('delegate_full_proxy') == 'true':
				args.append('-job-credential-file')
				args.append(self.delegation.filename)
		elif self.attrs.get('staging') == 'true':
			args.append('-staging-delegate')
		args.append('-job-description-file')
		args.append(xmlrsl_pathstr)
		logging.getLogger().debug('submit args: %s' % (repr(args),))

		fd, pathstr = tempfile.mkstemp('', 'invoke_server_epr.')
		fd2r, fd2w = os.pipe();

		# exec callback (called in child process of fork)
		def ex():
			os.close(fd2r)
			os.dup2(fd, 1)
			os.close(fd)
			os.dup2(fd2w, 2)
			os.close(fd2w)
			os.execv(prog, args)

		# process termination callback
		def cb(stat):
			if stat == 0:
				self.monitoring_on()
			else:
				self.status_handler('FAILED')

		try:
			def log_handler(line):
				logging.getLogger().debug('submitting: %s' % (repr(line),))
			Dispatcher.enter(fd2r, log_handler)
			del log_handler
			ngutils.ProcManager.start_proc(ex, cb)
			os.close(fd2w)
			self.epr_file_name = pathstr
			self.rsl_file_name = xmlrsl_pathstr
		finally:
			os.close(fd)

	def build_xmlrsl(self):
		staging = self.attrs.get('staging')
		rsl = ['job']

		def append_attr(key, element_name):
			val = self.attrs.get(key)
			if val:
				rsl.append([element_name, val])

		if staging == 'true':
			rsl.append(['executable', '${GLOBUS_SCRATCH_DIR}/invoke_server_executable.' + self.job_id[5:]])
		else:
			append_attr('executable_path', 'executable')
		append_attr('work_directory', 'directory')
		arguments = self.attrs.get('arguments')
		if arguments:
			for arg in arguments:
				rsl.append(['argument', arg])
		environments = self.attrs.get('environments')
		if environments:
			for env in environments:
				splitted = env.split('=', 1)
				env_name = splitted[0]
				if len(splitted) > 1 and splitted[1]:
					env_value = splitted[1]
				else:
					env_value = ''
				rsl.append(['environment', ['name', env_name], ['value', env_value]])
		append_attr('count', 'count')
		append_attr('host_count', 'hostCount')
		append_attr('project', 'project')
		append_attr('queue_name', 'queue')
		append_attr('max_time', 'maxTime')
		append_attr('max_wall_time', 'maxWallTime')
		append_attr('max_cpu_time', 'maxCpuTime')
		append_attr('max_memory', 'maxMemory')
		append_attr('min_memory', 'minMemory')
		del append_attr
		backend = self.attrs.get('backend')
		if backend in ['MPI', 'BLACS']:
			rsl.append(['jobType', 'mpi'])
		rftOptions = None
		deleteOptions = None
		staging_destination_subject = self.attrs.get('staging_destination_subject')
		staging_source_subject = self.attrs.get('staging_source_subject')
		deletion_subject = self.attrs.get('deletion_subject')
		if staging_destination_subject or staging_source_subject:
			rftOptions = ['rftOptions']
			if staging_destination_subject:
				rftOptions.append(['destinationSubjectName', staging_destination_subject])
			if staging_source_subject:
				rftOptions.append(['sourceSubjectName', staging_source_subject])
		if deletion_subject:
			deleteOptions = ['deleteOptions']
			deleteOptions.append(['subjectName', deletion_subject])
		gsiftp_port = self.attrs.get('gsiftp_port')
		if not gsiftp_port:
			gsiftp_port = '2811'
		if staging == 'true':
			fileStageIn = ['fileStageIn']
			fileStageIn.append(['transfer', ['sourceUrl', 'gsiftp://' + self.attrs['client_name'] + ':' + gsiftp_port + self.attrs['executable_path']],
					['destinationUrl', 'file:///${GLOBUS_SCRATCH_DIR}/invoke_server_executable.' + self.job_id[5:]] ])
			if rftOptions:
				fileStageIn.append(rftOptions)
			rsl.append(fileStageIn)
		if staging == 'true' or deleteOptions:
			fileCleanUp = ['fileCleanUp']
			if staging == 'true':
				fileCleanUp.append(['deletion', ['file', 'file:///${GLOBUS_SCRATCH_DIR}/invoke_server_executable.' + self.job_id[5:]]])
			if deleteOptions:
				fileCleanUp.append(deleteOptions)
			rsl.append(fileCleanUp)
		rsl_extensions = self.attrs.get('rsl_extensions')
		if rsl_extensions:
			extensions = ['extensions']
			for element in rsl_extensions:
				element = element + '\n'
				extensions.append(element)
			rsl.append(extensions)

		return rsl

	def writeout_xmlrsl(rsl):
		xmlrsl_fd, xmlrsl_pathstr = tempfile.mkstemp('', 'invoke_server_rsl.')
		xmlrsl_file = os.fdopen(xmlrsl_fd, 'w')
		del xmlrsl_fd
		format_xml(rsl, xmlrsl_file)
		xmlrsl_file.write('\n')
		xmlrsl_file.close()
		return xmlrsl_pathstr
	writeout_xmlrsl = staticmethod(writeout_xmlrsl)

	STATUS_RE = re.compile('^Current job state\: (.*)')
	STATUS_TABLE = {'Active' : 'ACTIVE',
		'Done' : 'DONE',
		'Failed' : 'FAILED'}

	def monitoring_on(self):
		self.observation_cont = True
		status_polling = self.attrs.get('status_polling')
		if status_polling:
			status_polling = int(status_polling)
			if status_polling:
				self.polling_on(status_polling)
		prog = gt4invokeserverconfig.GT4InvokeServerConfig.GLOBUS_LOCATION + '/bin/globusrun-ws'
		args = []
		args.append(prog)
		args.append('-monitor')
		if self.attrs.get('redirect_enable') == 'true':
			args.append('-streaming')
			stdout_file = None
			stdout_file = self.attrs.get('stdout_file')
			if stdout_file:
				args.append('-stdout-file')
				args.append(stdout_file)
			stderr_file = None
			stderr_file = self.attrs.get('stderr_file')
			if stderr_file:
				args.append('-stderr-file')
				args.append(stderr_file)
		args.append('-job-epr-file')
		args.append(self.epr_file_name)
		subject = self.attrs.get('subject')
		if subject:
			args.append('-subject-authz')
			args.append(subject)
		logging.getLogger().debug('monitor args: %s' % (repr(args),))
		fd = ngutils.ppopen2(prog, args)
		if fd == None:
			return
		def _handler(line):
			logging.getLogger().debug('monitoring: %s' % (repr(line),))
			if line:
				match = Job.STATUS_RE.match(line)
				if match:
					self.status_handler(Job.STATUS_TABLE.get(match.group(1)))
		Dispatcher.enter(fd, _handler)

	def polling_on(self, status_polling):
		prog = gt4invokeserverconfig.GT4InvokeServerConfig.GLOBUS_LOCATION + '/bin/globusrun-ws'
		args = []
		args.append(prog)
		args.append('-status')
		args.append('-job-epr-file')
		args.append(self.epr_file_name)
		subject = self.attrs.get('subject')
		if subject:
			args.append('-subject-authz')
			args.append(subject)

		def _polling():
			logging.getLogger().debug('_polling: %s' % (repr(self.req_id),))
			if self.status != 'ACTIVE':
				return
			fd = ngutils.ppopen(prog, args)
			if fd == None:
				return
			def _handler(line):
				if line:
					logging.getLogger().debug('polling: %s' % (repr(line),))
					match = Job.STATUS_RE.match(line)
					if match:
						self.status_handler(Job.STATUS_TABLE.get(match.group(1)))
			Dispatcher.enter(fd, _handler)
			ngutils.EventTimer.enter(status_polling, _polling)

		ngutils.EventTimer.enter(status_polling, _polling)

	def refresh_credential_on(self, interval):

		def _refresh_credential():
			if self.status == 'ACTIVE':
				prog = gt4invokeserverconfig.GT4InvokeServerConfig.GLOBUS_LOCATION + '/bin/globus-credential-refresh'
				args = []
				args.append(prog)
				subject = self.attrs.get('subject')
				if subject:
					args.append('-a')
					args.append(subject)
				args.append('-e')
				args.append(self.delegation.filename)
				if self.attrs.get('protocol') == 'http':
					args.append('-m')
					args.append('msg')

				# exec callback (called in child process of fork)
				def ex():
					fd = os.open('/dev/null', os.O_WRONLY)
					os.dup2(fd, 1)
					os.close(fd)
					os.execv(prog, args)

				# process termination callback
				def cb(stat):
					ngutils.EventTimer.enter(interval, _refresh_credential)

				ngutils.ProcManager.start_proc(ex, cb)

		ngutils.EventTimer.enter(interval, _refresh_credential)

	def destroy(self):
		if self.delegation:
			self.delegation.destroy()
			self.delegation = None
		if self.status in ['DONE', 'FAILED']:
			try:
				os.remove(self.epr_file_name)
				os.remove(self.rsl_file_name)
			except:
				pass
			return True
		# it needs to kill job
		# build commandline
		prog = '/bin/sh'
		args = []
		args.append(prog)
		args.append('-c')
		prog2 = gt4invokeserverconfig.GT4InvokeServerConfig.GLOBUS_LOCATION + '/bin/globusrun-ws'
		prog3 = gt4invokeserverconfig.GT4InvokeServerConfig.RMCOM
		subargs1 = []
		subargs1.append(prog2)
		subargs1.append('-kill')
		subargs1.append('-job-epr-file')
		subargs1.append(self.epr_file_name)
		subject = self.attrs.get('subject')
		if subject:
			subargs1.append('-subject-authz')
			subargs1.append(subject)
		subargs2 = []
		subargs2.append(prog3)
		subargs2.append(self.epr_file_name)
		subargs2.append(self.rsl_file_name)
		args.append('; '.join([expand_args(subargs1), expand_args(subargs2)]))
		logging.getLogger().debug('Job#destroy args: %s' % args)

		# exec callback (called in child process of fork)
		def ex():
			os.execv(prog, args)

		# process termination callback
		def cb(stat):
			pass

		return ngutils.ProcManager.start_proc(ex, cb)

	def status_handler(self, status):
		"""Receive outputs of command, if find status change, kick the notifier."""
		logging.getLogger().debug('status_handler: %s' % (repr(status),))
		if self.status == 'PENDING':
			if status == 'ACTIVE' or status == 'DONE' or status == 'FAILED':
				logging.getLogger().debug('status_handler: %s ==> %s' % (repr(self.status), repr(status)))
				self.status = status
				self.notify_status()
		elif self.status == 'ACTIVE':
			if not status:
				logging.getLogger().debug('status_handler: %s ==> %s' % (repr(self.status), repr('POST_ACTIVE')))
				self.status = 'POST_ACTIVE'
			elif status == 'DONE' or status == 'FAILED':
				logging.getLogger().debug('status_handler: %s ==> %s' % (repr(self.status), repr(status)))
				self.status = status
				self.notify_status()
		elif self.status == 'POST_ACTIVE':
			if status == 'DONE' or status == 'FAILED':
				logging.getLogger().debug('status_handler: %s ==> %s' % (repr(self.status), repr(status)))
				self.status = status
				self.notify_status()
		elif self.status == 'DONE' or self.status == 'FAILED':
			pass

	def notify_status(self):
		Notifier.send('STATUS_NOTIFY %s %s\r\n' % (self.job_id, self.status))


class GT4InvokeServer(object):
	"""NGV4 Invoke Server for GT ver. 4, Server"""

	TOKEN_RE = re.compile('^([^ ]+) ?(.*)')

	def take_token(cls, line):
		"""Split first token and rest."""
		match = cls.TOKEN_RE.match(line)
		if match:
			return match.group(1), match.group(2)
		else:
			return None, None
	take_token = classmethod(take_token)

	def __init__(self, cmd_fd, resp_fd, notif_fd):
		self.cmd_fd = cmd_fd
		Responder.fd = resp_fd
		Notifier.fd = notif_fd
		Dispatcher.dispatcher = ngutils.Dispatcher()
		self.handler = self.default_handler
		self.cont = True
		self.jobid2job_map = {}
		self.uuidGen = uuid.UUIDGen()

	def start(self):
		"""Enter callback handler to global dispatcher, and polling loop."""
		Dispatcher.enter(self.cmd_fd, self.input_handler)
		while self.cont:
			Dispatcher.poll_dispatch(1000)
			ngutils.ProcManager.wait_any()
			ngutils.EventTimer.action()

	def input_handler(self, line):
		"""Callback that called by global input dispatcher."""
		if not line:
			return
		stripped_line = line.rstrip()
		if not stripped_line:
			return
		self.handler(stripped_line)

	def default_handler(self, line):
		"""Interpret (headline of) command."""
		if not line:
			return
		logging.getLogger().debug('IN: %s' % (repr(line),))
		token, id = GT4InvokeServer.take_token(line)
		if token == 'JOB_CREATE':
			self.cmd_job_create(id)
		elif token == 'JOB_STATUS':
			self.cmd_job_status(id)
		elif token == 'JOB_DESTROY':
			self.cmd_job_destroy(id)
		elif token == 'EXIT':
			self.cmd_exit()
		else:
			Responder.send('F "%s %s" Cannot interpret\r\n' % (token, id))

	def cmd_job_create(self, req_id):
		if not req_id:
			Responder.send('F req_id is invalid\r\n')
			return
		req_attrs = {}

		def _handler(line):
			"""Handler for JOB_CREATE multiline command."""
			if not line:
				Responder.send('F parse error in JOB_CREATE, null line\r\n')
				self.handler = self.default_handler
				return
			if line != 'JOB_CREATE_END':
				logging.getLogger().debug('JOB_CREATE: "%s"' % (line,))
				key, val = GT4InvokeServer.take_token(line)
				if not key:
					Responder.send('F parse error in JOB_CREATE, line == "%s"\r\n' % (line,))
					self.handler = self.default_handler
					return
				if key == 'argument':
					if req_attrs.get('arguments') is None:
						req_attrs['arguments'] = []
					req_attrs['arguments'].append(val)
				elif key == 'environment':
					if req_attrs.get('environments') is None:
						req_attrs['environments'] = []
					req_attrs['environments'].append(val)
				elif key == 'rsl_extensions':
					if req_attrs.get('rsl_extensions') is None:
						req_attrs['rsl_extensions'] = []
					req_attrs['rsl_extensions'].append(val)
				else:
					req_attrs[key] = val
				return
			# line == 'JOB_CREATE_END':
			job = Job.setup_job(req_id, req_attrs)
			if not job:
				Responder.send('F failed to setup job\r\n')
				self.handler = self.default_handler
				return
			# send success response
			Responder.send('S\r\n')

			# start job
			job_id = 'uuid:' + self.uuidGen.next()
			self.jobid2job_map[job_id] = job
			job.start_job(job_id)
			self.handler = self.default_handler
			# end _handler

		# change handler
		self.handler = _handler

	def cmd_job_status(self, job_id):
		if not job_id:
			Responder.send('F unknown job_id (None)\r\n')
			return
		job = self.jobid2job_map.get(job_id)
		if not job:
			Responder.send('F unknown job_id "%s"\r\n' % (job_id,))
			return
		status = job.status
		if not status:
			Responder.send('F job_id "%s", status unknown\r\n' % (job_id,))
			return
		if status == 'POST_ACTIVE':
			status = 'ACTIVE'
		Responder.send('S %s\r\n' % (status,))

	def cmd_job_destroy(self, job_id):
		if not job_id:
			Responder.send('F unknown job_id (None)\r\n')
			return
		job = self.jobid2job_map.get(job_id)
		if not job:
			Responder.send('F unknown job_id "%s"\r\n' % (job_id,))
			return
		del self.jobid2job_map[job_id]
		if job.destroy():
			Responder.send('S\r\n')
		else:
			Responder.send('F failed to invoke cancel command\r\n')

	def cmd_exit(self):
		Responder.send('S\r\n')
		self.cont = False

