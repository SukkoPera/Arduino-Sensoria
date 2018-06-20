import datetime
import time

from Sensoria.StereoType import StereoType

class DateTimeData (StereoType):
	"""
		Stereotype: DT (Date/Time Data)

		Output can contain one or more of the following:
		* UTC: U:<unixtime:unsigned int:seconds since epoch>
		* Local Time: L:<unixtime:unsigned int:seconds since epoch>

		Time is expressed unixtime-style, i.e. as the number of seconds since
		January 1st, 1970 00:00:00 UTC.

		If more than one data item is present, they MUST all refer to the same
		instant. Thus, L - U gives the timezone offset in seconds (including
		DST, in case).
	"""

	_IDSTR = "DT"

	def __init__ (self):
		self.utctime = None
		self.localtime = None

	# Commodity, 'ts' here is a strictly-defined Unix timestamp (i.e.: UTC)
	@staticmethod
	def fromTimestamp (ts):
		# This certinly is more complex than I'd like it to be
		dtd = DateTimeData ()
		udt = datetime.datetime.utcfromtimestamp (ts)
		dtd.utctime = time.mktime (udt.timetuple ())
		ldt = datetime.datetime.fromtimestamp (ts)
		dtd.localtime = time.mktime (ldt.timetuple ())
		return dtd

	# Commodity
	@staticmethod
	def fromNow ():
		return DateTimeData.fromTimestamp (time.time ())

	def __eq__ (self, other):
		# Since all data items MUST refer to the same instant, comparing one is
		# sufficient
		return isinstance (other, self.__class__) and \
			self.utctime == other.utctime

	def unmarshal (self, string):
		d = dict ([p.split (":") for p in string.split (" ")])
		if 'U' in d:
			self.utctime = int (d['U'])
		if 'L' in d:
			self.localtime = int (d['L'])
		return True

	def marshal (self):
		ret = ""
		if self.utctime is not None:
			ret += "U:%u " % self.utctime
		if self.localtime is not None:
			ret += "L:%u " % self.localtime
		return ret.rstrip ()

	def __repr__ (self):
		ret = {}
		for attr in filter (lambda x: not x.startswith ("_") and x != "datetime", dir (self)):
			a = getattr (self, attr)
			if a is not None and not callable (a):
				ret[attr] = a
		return " ".join ("%s:%s" % (k, v) for k, v in ret.iteritems ())
