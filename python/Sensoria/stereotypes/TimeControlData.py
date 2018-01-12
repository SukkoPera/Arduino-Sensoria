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

PMO:000000000000000021100000000000000003322222111110 PTU:000000000000000021100000000000000003322222111110 PWE:000000000000000021100000000000000003322222111110 PTH:000000000000000021100000000000000003322222111110 PFR:000000000000000021100000000000000003322222221111 PSA:000000000000000000033222222222222222222222221111 PSU:000000000000000000033222222222222222222222221110
"""
class TimeControlData (StereoType):
	OFF = 0
	NDAYS = 7
	NHOURS = 24
	NSLOTS = 1		# 2 slots -> Half-hour granularity
	DAY_ABBR = ["MO", "TU", "WE", "TH", "FR", "SA", "SU"]
	MON, TUE, WED, THU, FRI, SAT, SUN = xrange (0, 7)

	_IDSTR = "TC"

	def __init__ (self):
		self.schedule = [[[TimeControlData.OFF for i in range (TimeControlData.NSLOTS)] for i in range (TimeControlData.NHOURS)] for i in range (TimeControlData.NDAYS)]

	def __eq__ (self, other):
		return isinstance (other, self.__class__) and \
			self.schedule == other.schedule

	def unmarshal (self, string):
		ret = True
		d = dict ([p.split (":") for p in string.split (" ")])

		for k, v in d.iteritems ():
			if k[0].upper () == "P" and len (k) == 3:
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
			else:
				print "Bad key: %s" % k
		return ret

	def marshal (self):
		val = ""

		for i in xrange (0, TimeControlData.NDAYS):
			p = "".join (["".join (str (y) for y in x) for x in self.schedule[i]])
			val += "P%s:%s " % (TimeControlData.DAY_ABBR[i], p)

		return val.rstrip ()

	# Nice :)
	def __repr__ (self):
		return self.marshal ()

if __name__ == "__main__":
	s = """
PMO:000000001000000003222110
PTU:000000001000000003222110
PWE:000000001000000003222110
PTH:000000001000000003222110
PFR:000000001000000003222111
PSA:000000000322222222222211
PSU:000000000322222222222210
""".replace ('\n', ' ').strip ()


	tc = TimeControlData ()
	tc.unmarshal (s)
	#~ for day in xrange (0, TimeControlData.NDAYS):
		#~ print tc.schedule[day]
	#~ print str (tc)
	s2 = tc.marshal ()
	assert s == s2, "Marshal/Unmarshal mismatch:\n%s\n%s" % (s, s2)
