from Sensoria.StereoType import StereoType

"""
EXAMPLES:

On/Off
                        1 1 1 1 1 1 1 1 1 1 2 2 2 2
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
PMO:000000000000000011100000000000000001111111111110
PTU:000000000000000011100000000000000001111111111110
PWE:000000000000000011100000000000000001111111111110
PTH:000000000000000011100000000000000001111111111110
PFR:000000000000000011100000000000000001111111111111
PSA:000000000000000000011111111111111111111111111111
PSU:000000000000000000011111111111111111111111111111

Multilevel
                        1 1 1 1 1 1 1 1 1 1 2 2 2 2
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
PMO:000000000000000021100000000000000003322222111110
PTU:000000000000000021100000000000000003322222111110
PWE:000000000000000021100000000000000003322222111110
PTH:000000000000000021100000000000000003322222111110
PFR:000000000000000021100000000000000003322222221111
PSA:000000000000000000033222222222222222222222221111
PSU:000000000000000000033222222222222222222222221110

TL1:10 TL2:18 TL3:21 PMO:000000000000000021100000000000000003322222111110 PTU:000000000000000021100000000000000003322222111110 PWE:000000000000000021100000000000000003322222111110 PTH:000000000000000021100000000000000003322222111110 PFR:000000000000000021100000000000000003322222221111 PSA:000000000000000000033222222222222222222222221111 PSU:000000000000000000033222222222222222222222221110
"""
class TimeControlData (StereoType):
	OFF = 0
	UNKNOWN = -1
	NLEVELS = 3
	NDAYS = 7
	NHOURS = 24
	NSLOTS = 1		# 2 slots -> Half-hour granularity
	DAY_ABBR = ["MO", "TU", "WE", "TH", "FR", "SA", "SU"]
	MON, TUE, WED, THU, FRI, SAT, SUN = xrange (0, 7)

	_IDSTR = "TC"

	def __init__ (self):
		#~ self.schedule = [[[TimeControlData.OFF] * TimeControlData.NSLOTS] * TimeControlData.NHOURS] * TimeControlData.NDAYS
		self.schedule = [[[TimeControlData.OFF for i in range (TimeControlData.NSLOTS)] for i in range (TimeControlData.NHOURS)] for i in range (TimeControlData.NDAYS)]
		self.levels = [TimeControlData.UNKNOWN for i in range (TimeControlData.NLEVELS + 1)]	# Level 0 is built-in

	#~ def __eq__ (self, other):
		#~ same = other is not None and \
			#~ self.state == other.state and \
			#~ self.controller == other.controller

	def unmarshal (self, string):
		ret = True
		string = string.upper ()
		d = dict ([p.split (":") for p in string.split (" ")])

		for k, v in d.iteritems ():
			if k.startswith ("TL") and len (k) == 3:
				lev = int (k[2])
				if lev >= 1 and lev <= TimeControlData.NLEVELS:
					self.levels[lev] = float (v)
			elif k.startswith ("P") and len (k) == 3:
				day = k[1:]
				if day in TimeControlData.DAY_ABBR:
					n = TimeControlData.DAY_ABBR.index (day)
					if len (v) == TimeControlData.NHOURS * TimeControlData.NSLOTS:
						# Let Python do the magic!
						self.schedule[n] = [[int (x) for x in list (v[i:i + TimeControlData.NSLOTS])] for i in xrange (0, TimeControlData.NHOURS * TimeControlData.NSLOTS, TimeControlData.NSLOTS)]
					else:
						print "Bad schedule of length %d" % len (v)
						ret = False
				else:
					print "Bad Day: '%s'" % day
					ret = False

		return ret

	def marshal (self):
		val = ""

		for i in xrange (1, TimeControlData.NLEVELS + 1):
			if int (self.levels[i]) == self.levels[i]:
				# Avoid decimals if possible
				val += "TL%u:%u " % (i, self.levels[i])
			else:
				val += "TL%u:%f " % (i, self.levels[i])

		for i in xrange (0, TimeControlData.NDAYS):
			p = "".join (["".join (str (y) for y in x) for x in self.schedule[i]])
			val += "P%s:%s " % (TimeControlData.DAY_ABBR[i], p)

		return val.rstrip ()

	# Nice :)
	def __repr__ (self):
		return self.marshal ()

if __name__ == "__main__":
	s = "TL1:10 TL2:18 TL3:21 PMO:000000000000000021100000000000000003322222111110 PTU:000000000000000021100000000000000003322222111110 PWE:000000000000000021100000000000000003322222111110 PTH:000000000000000021100000000000000003322222111110 PFR:000000000000000021100000000000000003322222221111 PSA:000000000000000000033222222222222222222222221111 PSU:000000000000000000033222222222222222222222221110"

	tc = TimeControlData ()
	tc.unmarshal (s)
	#~ for day in xrange (0, TimeControlData.NDAYS):
		#~ print tc.schedule[day]
	#~ for i in xrange (1, TimeControlData.NLEVELS + 1):
		#~ print "- Level %u: %u" % (i, tc.levels[i])
	#~ print str (tc)
	s2 = tc.marshal ()
	assert s == s2, "Marshal/Unmarshal mismatch"
