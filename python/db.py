#!/usr/bin/env python

import datetime
import sqlite3

class DB (object):
	def __init__ (self):
		self._conn = sqlite3.connect ('weather.db', detect_types = sqlite3.PARSE_DECLTYPES)
		self._conn.row_factory = sqlite3.Row
		
	def create (self):
		q = """CREATE TABLE weather_data (
			date timestamp,
			temp_dht real,
			temp_dallas real,
			temp_bmp real,
			hum real,
			local_px real,
			sea_px real,
			light_lux real,
			light_v real
		)"""
		self._conn.execute (q)
		self._conn.commit ()
		
	def insert (self, dt, tdht, tdallas, tbmp, hum, loc_px, sea_px, lux, lv, commit = False):
		self._conn.execute ("INSERT INTO weather_data VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)", (dt, tdht, tdallas, tbmp, hum, loc_px, sea_px, lux, lv))
		if commit:
			self._conn.commit ()

	def close (self):
		self._conn.commit ()
		self._conn.close ()
		
	def get_data_since (self, start):
		now = datetime.datetime.now ()
		c = self._conn.cursor ()
		c.execute ("SELECT * FROM weather_data WHERE date BETWEEN ? AND ? ORDER BY date ASC", (start, now))
		for row in c.fetchall ():
			yield dict (zip (row.keys (), row))  
		
	def get_data_between (self, start, end):
		c = self._conn.cursor ()
		c.execute ("SELECT * FROM weather_data WHERE date BETWEEN ? AND ? ORDER BY date ASC", (start, end))
		row = c.fetchone ()
		if row is not None:
			yield dict (zip (row.keys (), row))
		else:
			yield
		
	def count (self):
		c = self._conn.cursor ()
		c.execute ("SELECT COUNT(*) FROM weather_data")
		return c.fetchone ()[0]

if __name__ == "__main__":
	db = DB ()
	print db.count ()
	now = datetime.datetime.now ()
	#~ print list (db.get_data_between ("2015-11-10", "2015-11-15"))
	print list (db.get_data_since ("2015-11-10"))
