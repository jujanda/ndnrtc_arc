import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import sys


def read_file(filename):
	avgTP = 0.0
	sumTP = 0.0
	numOfTP = 0.0
	file = open(filename, "r")

	# go through every line in the file except the first two
	for line in file.readlines()[2:]:

		# split line at spaces
		record = line.split(" ")

		# take second to last value
		tmpTP = record[-2]

		# add value and keep count
		sumTP += float(tmpTP)
		numOfTP += 1

	# calulate average throughput
	avgTP = sumTP / numOfTP
	
	file.close()
	return avgTP


# ===================================    

# save provided file arguments
path = sys.argv[1]
setting = sys.argv[2]
run = sys.argv[3]

# reconstruct file name from arguments
filename = path + "setting_" + setting + "/results/" + run + "/arcLog_networkMeasurements.csv"

# scan file
avgTP = read_file(filename)

# print result
print "Average throughput: " + str(avgTP) + " kBit/s"




