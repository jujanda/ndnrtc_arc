import numpy as np
import sys

# save provided file arguments
path = sys.argv[1]
setting = sys.argv[2]
run = sys.argv[3]

# reconstruct file name from arguments
filename = path + "setting_" + setting + "/results/" + run + "/arc_PostProcess.log"

# # open file
# file = open(filename, "r")

# # go through every line in the file except the first two
# for line in file.readlines():
# 	if "Total frames missing:" in line:
# 		try:
# 		    file.remove(line)
# 		except:
# 			print "line not found"
# 		break

# file.close()

# from: https://stackoverflow.com/questions/4710067/using-python-for-deleting-a-specific-line-in-a-file
with open(filename,"r+") as f:
	new_f = f.readlines()
	f.seek(0)
	keepNextLine = True
	for line in new_f:
		if "Total frames missing:" not in line:
			if keepNextLine:
				f.write(line)
			else: 
				keepNextLine = True
		else:
			keepNextLine = False
	f.truncate()
