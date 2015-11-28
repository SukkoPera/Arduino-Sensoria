from stereotype import StereoType

class WeatherData (StereoType):
	"""
		Stereotype: WD (Weather Data)
		
		Output can contain one or more of the following:
		* Temperature: T:<temperature:float:degrees celsius>
		* Humidity: H:<humidity:float:percentage>
		* Pressure (Local or adjusted to Sea-level): (LP|SP):<pressure:float:hectopascal> (NOTE: 1 hectopascal == 1 mbar)
		* Altitude: A:<altitude:float:meters>
		* Light: LX:<light_level:float:lux> | LU:<light_level:float:0-1023 scale>

		#cfg mode alt 123456
		#cfg mode rel 123"""
	
	_IDSTR = "WD"
	
	def __init__ (self):
		self.temperature = None
		self.humidity = None
		self.localPressure = None
		self.seaPressure = None
		self.altitude = None
		self.luxLight = None
		self.scaledLight = None

	@staticmethod
	def parse (output):
		wd = WeatherData ()
		d = dict ([p.split (":") for p in output.split (" ")])
		if 'T' in d:
			wd.temperature = float (d['T'])
		if 'H' in d:
			wd.humidity = float (d['H'])
		if 'LP' in d:
			wd.localPressure = float (d['LP'])
		if 'SP' in d:
			wd.seaPressure = float (d['SP'])
		if 'A' in d:
			wd.altitude = float (d['A'])
		if 'LX' in d:
			wd.luxLight = float (d['LX'])
		if 'LU' in d:
			wd.scaledLight = float (d['LU'])
		return wd
