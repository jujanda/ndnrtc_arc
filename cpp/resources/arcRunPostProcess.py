import sys
import fileinput
import os

# Define parameters
RESULT_PATH = sys.argv[1]
SETTING = sys.argv[2]
RUNS = sys.argv[3]

for run_counter in xrange(0,int(RUNS)):
	# ./resources/arcPostProcess.sh /media/julian/extHD/ 0 2
	os.system("./resources/arcPostProcess.sh " + RESULT_PATH + " " + SETTING + " " + str(run_counter))
