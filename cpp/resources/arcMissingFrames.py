# coding: utf-8
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from collections import OrderedDict
import sys


# Define Parameters
PATH = sys.argv[1]

# Define Variables
publishedFrames = []
playedFrames = []
missingFrames = []

# Parse log files to lists
with open(PATH + "/logs-original/producer-producer-camera.log","r") as input:
   file = input.readlines()
   for line in file:
      linePosition = line.find("▻")
      if linePosition >= 0:
         record = line.split(" ")
         publishedFrames.append(record[21] + "\t" + record [22])

# 1272.0	[DEBUG][            vplayout]-       extractSample: ●-- play frame [     5☆, C, 116.667%   0, 24277 /low/k/5/%00%00 CACH dgen -1 rtt -1 9874257]31ms
with open(PATH + "/all.log","r") as input:
		file = input.readlines()
		for line in file:
			linePosition = line.find("●--")
			if linePosition >= 0:
				record = line.split(" ")
				for item in record:
					itemPosition = item.find("/")
					if itemPosition >= 0:
						playedFrames.append(item.split("/")[3] + item.split("/")[2])

# Delete duplicates from list
publishedFrames = list(OrderedDict.fromkeys(publishedFrames))

# Find missing frames
for item in publishedFrames:
	absoluteFrameNumber = item.split("\t")[1][:-1]
	relativeFrameNumber = item.split("\t")[0]

	if relativeFrameNumber not in playedFrames:
		missingFrames.append(absoluteFrameNumber)

# Write to files
with open(PATH + "/publishedFrames.log","wb") as output:
   for item in publishedFrames:
      output.write(item + "\n")

with open(PATH + "/playedFrames.log","wb") as output:
	for item in playedFrames:
		output.write(item + "\n")

with open(PATH + "/missingFrames.log","wb") as output:
	for item in missingFrames:
		output.write(item + "\n")

print "Total frames missing: " + str(len(missingFrames))
