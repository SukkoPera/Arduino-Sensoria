#!/usr/bin/env python

# Derived from
# https://www.metachris.com/2016/04/python-threadpool/

from threading import Thread

# ~ import myqueue as Queue
from classfulqueue import ClassfulQueue, Full

__all__ = ['ThreadPool']


class Worker(Thread):
	NTHREADS = 0

	""" Thread executing tasks from a given tasks queue """
	def __init__(self, tasks):
		Thread.__init__(self)
		self.n = Worker.NTHREADS
		Worker.NTHREADS += 1
		self.tasks = tasks
		self.daemon = True
		self.start()

	def run(self):
		while True:
			# ~ print "Thread %d waiting" % self.n
			task, n = self.tasks.get()
			func, args = task
			# ~ print "Thread %d running %s" % (self.n, str (func))
			assert callable (func)
			try:
				# ~ func(*args, **kargs)
				func(*args)
			except Exception as e:
				# An exception happened in this thread
				print "Thread %d raised an exception: %s" % (self.n, str (e))
			finally:
				# Mark this task as done, whether an exception happened or not
				self.tasks.task_done(n)
				# ~ print "Thread %d done" % self.n

class ThreadPool:
	""" Pool of threads consuming tasks from a queue """
	def __init__(self, num_threads, qlen):
		self.tasks = ClassfulQueue(qlen)
		for _ in range(num_threads):
			Worker(self.tasks)

	def add_task(self, func, cls, *args, **kargs):
		""" Add a task to the queue """
		try:
			self.tasks.put((func, args), cls)
		except Full:
			raise

	def wait_completion(self):
		""" Wait for completion of all the tasks in the queue """
		self.tasks.join()


if __name__ == "__main__":
	from random import randrange
	from time import sleep

	# Function to be executed in a thread
	def wait_delay(d):
		print("sleeping for (%d)sec" % d)
		sleep(d)

	# Instantiate a thread pool with 5 worker threads
	pool = ThreadPool(5, qlen = 25)

	# Generate random delays
	for i in range(5000):
		cls = str (randrange(0, 5))
		delay = randrange(0, 10) / 10
		pool.add_task(wait_delay, cls, delay)

	# The code will block here, which
	# makes it possible to cancel the thread pool with an exception when
	# the currently running batch of workers is finished.
	pool.wait_completion()
