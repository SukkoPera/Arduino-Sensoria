from Sensoria.StereoType import StereoType

"""
EXAMPLE:

R:somebody@gmail.com S:Subject\ with\ spaces B:Body\ with\ spaces

Subject is not mandatory, other fields are

TODO: Multiple recipients
"""
class InstantMessageData (StereoType):
	_IDSTR = "IM"

	# By titan (Slightly modified)
	# https://stackoverflow.com/questions/18092354/python-split-string-without-splitting-escaped-character#21107911
	@staticmethod
	def escape_split (s, delim = " "):
		i, res, buf = 0, [], ''
		while True:
			j, e = s.find(delim, i), 0
			if j < 0:  # end reached
				return res + [buf + s[i:]]  # add remainder
			while j - e and s[j - e - 1] == '\\':
				e += 1  # number of escapes
			d = e // 2  # number of double escapes
			if e != d * 2:  # odd number of escapes
				buf += s[i:j - d - 1] + s[j]  # add the escaped char
				i = j + 1  # and skip it
				continue  # add more to buf
			res.append(buf + s[i:j])
			i, buf = j + len(delim), ''  # start after delim

	def __init__ (self):
		self.recipient = None
		self.subject = None
		self.body = None

	def __eq__ (self, other):
		return isinstance (other, self.__class__) and \
			self.recipient == other.recipient and \
			self.subject == other.subject and \
			self.body == other.body

	def unmarshal (self, string):
		ret = True
		parts = InstantMessageData.escape_split (string, " ")
		d = dict ([p.split (":") for p in parts])
		for k, v in d.iteritems ():
			k = k.upper ()
			v = v.decode ("string_escape")		# Escape backslashes
			if k == 'R':
				self.recipient = v
			elif k == 'S':
				self.subject = v
			elif k == 'B':
				self.body = v
			else:
				print "Bad key: %s" % k
				ret = False
		return ret

	@staticmethod
	def _sanitize (s):
		v = str (s)							# No Unicode on the line!
		v = v.encode ("string_escape")		# Backslash-escape characters that need it
		v = v.replace (" ", r"\ ")			# Additionally escape spaces
		return v

	def marshal (self):
		ret = None
		if self.recipient is not None and self.body is not None:
			vals = []
			vals.append ("R:%s" % InstantMessageData._sanitize (self.recipient))
			if self.subject is not None:	# I like subject to be here inbetween
				vals.append ("S:%s" %  InstantMessageData._sanitize (self.subject))
			vals.append ("B:%s" % InstantMessageData._sanitize (self.body))
			ret = " ".join (vals)
		return ret

	# Nice :)
	def __repr__ (self):
		return self.marshal ()

if __name__ == "__main__":
	s = r"R:somebody@gmail.com S:Subject\ with\ spaces B:Body\ with\ spaces\ and\ \\\ backslash\ \\"

	vs = InstantMessageData ()
	vs.unmarshal (s)
	print vs.recipient
	print vs.subject
	print vs.body
	print str (vs)
	s2 = vs.marshal ()
	assert s == s2, "Marshal/Unmarshal mismatch:\n'%s'\nvs.:\n'%s'" % (s, s2)
