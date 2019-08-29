# coding: utf-8
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from collections import OrderedDict
import sys


# Define parameters
PATH = sys.argv[1]

# Define variables
publishedFrames = []
playedFrames = []
missingFrames = []
falsePositives = []

# Define methods
def removeAllOccurencesFromList(frameList, delFrame):
   return [frame for frame in frameList if frame != delFrame]


# Parse log files to lists
with open(PATH + "/logs-original/producer-producer-camera.log","r") as input:
	file = input.readlines()
	for line in file:
		linePosition = line.find("spawned publish task for")
		if linePosition >= 0:
			record = line.split(" ")
			# 1566302748363	[TRACE][      vstream-camera]-             publish: spawned publish task for 3d 4p (/high/d/%FE%03)
			publishedFrames.append(record[24] + "_" + record[26].split("/")[1] + "\t" + record[25][:-1])

with open(PATH + "/all.log","r") as input:
		file = input.readlines()
		for line in file:
			linePosition = line.find("●--")
			if linePosition >= 0:
				record = line.split(" ")
				for item in record:
					itemPosition = item.find("/")
					if itemPosition >= 0:
						# 1272.0	[DEBUG][            vplayout]-       extractSample: ●-- play frame [     5☆, C, 116.667%   0, 24277 /low/k/5/%00%00 CACH dgen -1 rtt -1 9874257]31ms
						playedFrames.append(item.split("/")[3] + item.split("/")[2] + "_" + item.split("/")[1])

# Delete duplicates from list
publishedFrames = list(OrderedDict.fromkeys(publishedFrames))

# Find missing frames
for item in publishedFrames:
	absoluteFrameNumber = item.split("\t")[1]
	relativeFrameNumber = item.split("\t")[0]

	# Look what frames got played and add their absolute numbers to corresponding playedFrames records
	if relativeFrameNumber in playedFrames:
		position = playedFrames.index(relativeFrameNumber)
		playedFrames[position] += "\t" + absoluteFrameNumber
	
	# Gather all missing frames' absolute numbers in seperate list
	else:
		missingFrames.append(absoluteFrameNumber)

# Delete false positives from missingFrames (happens due to duplicates from multiple representations)
for item in playedFrames:
	missingFrames = removeAllOccurencesFromList(missingFrames, item.split("\t")[-1])

# Delete duplicates from list (only count lost frames once, instead of per representation)
missingFrames = list(OrderedDict.fromkeys(missingFrames))

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

print "Total frames published: " + str(len(publishedFrames))
print "Total frames played: " + str(len(playedFrames))
print "Total frames missing: " + str(len(missingFrames))
