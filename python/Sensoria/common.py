class Error (Exception):
	pass

class SensorError (Error):
	pass

# Actuator genres
SENSOR, ACTUATOR = xrange (0, 2)
