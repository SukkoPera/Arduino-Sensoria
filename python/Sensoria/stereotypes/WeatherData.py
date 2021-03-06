from Sensoria.StereoType import StereoType

class WeatherData (StereoType):
	"""
		Stereotype: WD (Weather Data)

		Output can contain one or more of the following:
		* Temperature: T:<temperature:float:degrees celsius>
		* Humidity: H:<humidity:float:percentage>
		* Pressure (Local or adjusted to Sea-level): (LP|SP):<pressure:float:hectopascal> (NOTE: 1 hectopascal == 1 mbar)
		* Altitude: A:<altitude:float:meters>
		* Light: LX:<light_level:float:lux> | LU:<light_level:float:0-1023 scale> | LV:<visible_light_level:lux> | LR <infrared_light_level:lux>

		#cfg mode alt 123456
		#cfg mode rel 123"""

	_IDSTR = "WD"

	def __init__ (self):
		self.temperature = None
		self.humidity = None
		self.localPressure = None
		self.seaPressure = None
		self.altitude = None
		self.lightLux = None
		self.light10bit = None

	def __eq__ (self, other):
		return isinstance (other, self.__class__) and \
			self.temperature == other.temperature and \
			self.humidity == other.humidity and \
			self.localPressure == other.localPressure and \
			self.altitude == other.altitude and \
			self.lightLux == other.lightLux

	def unmarshal (self, string):
		d = dict ([p.split (":") for p in string.split (" ")])
		if 'T' in d:
			self.temperature = float (d['T'])
		if 'H' in d:
			self.humidity = float (d['H'])
		if 'LP' in d:
			self.localPressure = float (d['LP'])
		if 'SP' in d:
			self.seaPressure = float (d['SP'])
		if 'A' in d:
			self.altitude = float (d['A'])
		if 'LX' in d:
			self.lightLux = float (d['LX'])
		if 'LU' in d:
			self.light10bit = float (d['LU'])
		return True

	def marshal (self):
		ret = ""
		if self.temperature is not None:
			ret += "T:%.2f " % self.temperature
		if self.humidity is not None:
			ret += "H:%.2f " % self.humidity
		if self.localPressure is not None:
			ret += "LP:%.2f " % self.localPressure
		if self.seaPressure is not None:
			ret += "SP:%.2f " % self.seaPressure
		if self.altitude is not None:
			ret += "A:%.2f " % self.altitude
		if self.lightLux is not None:
			ret += "LX:%.2f " % self.lightLux
		if self.light10bit is not None:
			ret += "LX:%.2f " % self.light10bit
		return ret

	def __repr__ (self):
		ret = {}
		for attr in filter (lambda x: not x.startswith ("_"), dir (self)):
			a = getattr (self, attr)
			if a is not None and not callable (a):
				ret[attr] = a
		return " ".join ("%s:%s" % (k, v) for k, v in ret.iteritems ())
