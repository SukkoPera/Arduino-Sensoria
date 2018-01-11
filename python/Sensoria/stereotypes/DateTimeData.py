import datetime
import time

from Sensoria.StereoType import StereoType

class DateTimeData (StereoType):
	"""
		Stereotype: DT (Date/Time Data)

		Output can contain one or more of the following:
		* Date: D:<date:string:DDMMYYYY>
		* Time: T:<time:string:HHMMSS>
		* UnixTime: U:<unixtime:unsigned int:seconds since epoch>

		If more than one data item is present, they MUST all refer to the same date/time.
	"""

	_IDSTR = "DT"

	def __init__ (self):
		self.date = None
		self.time = None
		self.unixtime = None

	# Commodity
	@staticmethod
	def fromDateTime (dt):
		dtd = DateTimeData ()
		dtd.date = dt.date ()
		dtd.time = dt.time ()
		dtd.unixtime = time.mktime (dt.timetuple())
		return dtd

	# Commodity
	@staticmethod
	def fromNow ():
		return DateTimeData.fromDateTime (datetime.datetime.now ())

	# Commodity
	@property
	def datetime (self):
		if self.date is not None and self.time is not None:
			return datetime.datetime.combine (self.date, self.time)

	def __eq__ (self, other):
		return self.date == other.date and \
			self.time == other.time and \
			self.unixtime == other.unixtime

	def unmarshal (self, string):
		d = dict ([p.split (":") for p in string.split (" ")])
		if 'D' in d:
			self.date = datetime.datetime.strptime (d['D'], "%d%m%y").date ()
		if 'T' in d:
			self.time = datetime.datetime.strptime (d['T'], "%H%M%S").time ()
		if 'U' in d:
			self.unixtime = int (d['U'])
		return True

	def marshal (self):
		ret = ""
		if self.date is not None:
			ret += "D:%s " % self.date.strftime ("%d%m%y")
		if self.time is not None:
			ret += "T:%s " % self.time.strftime ("%H%M%S")
		if self.unixtime is not None:
			ret += "U:%u " % self.unixtime
		return ret.rstrip ()

	def __repr__ (self):
		ret = {}
		for attr in filter (lambda x: not x.startswith ("_") and x != "datetime", dir (self)):
			a = getattr (self, attr)
			if a is not None and not callable (a):
				ret[attr] = a
		return " ".join ("%s:%s" % (k, v) for k, v in ret.iteritems ())
