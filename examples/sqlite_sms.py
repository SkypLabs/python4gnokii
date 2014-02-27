#!/usr/bin/python
# -*- coding: utf-8 -*-

from gnokii import *
from sqlite3 import *

conn = connect('sms.db')

c = conn.cursor()
c.execute('''CREATE TABLE IF NOT EXISTS sms (
	id INTEGER PRIMARY KEY NOT NULL,
	time TIMESTAMP,
	num TEXT,
	msg TEXT);'''
)

print('[*] Start SMS reception ...')

try:
	while True:
		messages = getsms('SM', 1, 5)
		i=0

		for msg in messages:
			i += 1

			if msg:
				print("From : {0}".format(msg[2]))
				print("Message : {0}".format(msg[4]))
				print("----------")

				c.execute("INSERT INTO sms(time, num, msg) VALUES (DateTime('now'), '{0}', '{1}');".format(msg[2], msg[4]))
				conn.commit()

				deletesms('SM', i)
except KeyboardInterrupt as e:
	conn.commit()
	conn.close()
	print('[*] Bye !')
	exit(0)
