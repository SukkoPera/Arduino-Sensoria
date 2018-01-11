#!/usr/bin/env python

import datetime
import sqlite3

import stereotypes

class DB (object):
	TABLE = "sensoria"

	def __init__ (self, dbfile = 'sensoria.db'):
		self._conn = sqlite3.connect (dbfile, detect_types = sqlite3.PARSE_DECLTYPES)
		self._conn.row_factory = sqlite3.Row
		if not self.has_column ("date"):
			print "Creating DB"
			self.create ()
		self._load_stereotypes ()

	def _load_stereotypes (self):
		# Load stereotypes (Hmmm... A bit of a hack?)
		self.stereotypes = {}
		for clsname in stereotypes.__all__:
			exec ("import stereotypes.%s" % clsname)
			cls = eval ("stereotypes.%s.%s" % (clsname, clsname))
			id_ = cls.getIdString ()
			print "Registering stereotype: %s (%s)" % (clsname, id_)
			if id_ in self.stereotypes:
				print "ERROR: Duplicate stereotype: %s" % id_
			else:
				self.stereotypes[cls.getIdString ()] = cls

	def create (self):
		q = """CREATE TABLE %s (
			date timestamp PRIMARY KEY
		)""" % self.TABLE
		self._conn.execute (q)
		self._conn.commit ()

	def has_column (self, col):
		c = self._conn.cursor ()
		c.execute ("PRAGMA table_info (%s)" % self.TABLE)
		return any (x[1] == col for x in c.fetchall ())

	def add_column (self, col):
		c = self._conn.cursor ()
		c.execute ("ALTER TABLE %s ADD COLUMN '%s' TYPE TEXT" % (self.TABLE, col))
		#~ self._conn.commit ()

	def _trans2col (self, t):
		"""Makes up the name of the column where data for a transducer belong"""
		return "%s:%s" % (t.name, t.stereotype)

	def insert (self, date, dct, commit = False):
		"""dct is a dictionary of sensor_name => sensor_data type"""
		for t in dct.iterkeys ():
			cname = self._trans2col (t)
			if not self.has_column (cname):
				print "Adding column: %s" % cname
				self.add_column (cname)

		# If items(), keys(), values(),  iteritems(), iterkeys(), and
		# itervalues() are called with no intervening modifications to
		# the dictionary, the lists will directly correspond
		self._conn.execute ("INSERT INTO %s (date, %s) VALUES ('%s', %s)" % (self.TABLE, ", ".join ("'%s'" % self._trans2col (x) for x in dct.keys ()), date, ", ".join ("'%s'" % x for x in dct.values ())))
		if commit:
			self._conn.commit ()

	def close (self):
		self._conn.commit ()
		self._conn.close ()

	def get_data_since (self, start):
		now = datetime.datetime.now ()
		c = self._conn.cursor ()
		c.execute ("SELECT * FROM %s WHERE date BETWEEN ? AND ? ORDER BY date ASC" % self.TABLE, (start, now))
		for row in c:
			dt = row["date"]
			vals = {}
			for t, r in zip (row.keys (), row):
				if t != "date" and r is not None:
					parts = t.split (":")
					if len (parts) == 2:
						tname, st = parts
						stereoclass = self.stereotypes[st]
						vals[tname] = stereoclass.unmarshalStatic (r)
					else:
						print "ERROR: Unrecognized column in DB: %s" % t
			yield dt, vals

	def get_data_between (self, start, end):
		c = self._conn.cursor ()
		c.execute ("SELECT * FROM %s WHERE date BETWEEN ? AND ? ORDER BY date ASC" % self.TABLE, (start, end))
		for row in c:
			dt = row["date"]
			vals = {}
			for t, r in zip (row.keys (), row):
				if t != "date" and r is not None:
					parts = t.split (":")
					if len (parts) == 2:
						tname, st = parts
						stereoclass = self.stereotypes[st]
						vals[tname] = stereoclass.unmarshalStatic (r)
			yield dt, vals

	def count (self):
		c = self._conn.cursor ()
		c.execute ("SELECT COUNT(*) FROM %s" % self.TABLE)
		return c.fetchone ()[0]

if __name__ == "__main__":
	db = DB ()
	print "DB currently has %d lines" % db.count ()
	#~ print list (db.get_data_between ("2016-02-10", "2016-02-29"))
	print list (db.get_data_since ("2016-01-15"))
