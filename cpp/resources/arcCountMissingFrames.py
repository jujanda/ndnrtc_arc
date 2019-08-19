import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import sys


def file_len(fname):
    with open(fname) as f:
    	i = -1
        for i, l in enumerate(f):
            pass
    return i + 1

def read_file(filename):
	file = open(filename, "r")
	sum = 0

	# go through every line in the file
	for line in file.readlines():
		sum += 1
	
	file.close()
	return sum


# ===================================    

# save provided file arguments
path = sys.argv[1]
setting = sys.argv[2]
run = sys.argv[3]

# reconstruct file name from arguments
filename = path + "setting_" + setting + "/results/" + run + "/missingFrames.log"

# scan file
missingFrames = read_file(filename)

# print result
print "Total frames missing: " + str(missingFrames)




