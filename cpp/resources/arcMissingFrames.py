# coding: utf-8
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
from collections import OrderedDict

# Define Variables
publishedFrames = []
playedFrames = []
missingFrames = []

# Parse log files to lists
with open("../loopback/logs-original/producer-producer-camera.log","r") as input:
   file = input.readlines()
   for line in file:
      linePosition = line.find("▻")
      if linePosition >= 0:
         record = line.split(" ")
         publishedFrames.append(record[21] + "\t" + record [22])

with open("../loopback/all.log","r") as input:
		file = input.readlines()
		for line in file:
			linePosition = line.find("●--")
			if linePosition >= 0:
				record = line.split(" ")
				for item in record:
					itemPosition = item.find("/")
					if itemPosition >= 0:
						playedFrames.append(item.split("/")[3])

# Delete duplicates from list
publishedFrames = list(OrderedDict.fromkeys(publishedFrames))

# Find missing frames
for item in publishedFrames:
	absoluteFrameNumber = item.split("\t")[1][:-1]
	relativeFrameNumber = item.split("\t")[0][:-1]
	if absoluteFrameNumber not in playedFrames:
		if relativeFrameNumber not in playedFrames:
			missingFrames.append(absoluteFrameNumber)

# Write to files
with open("../loopback/publishedFrames.log","wb") as output:
   for item in publishedFrames:
      output.write(item + "\n")

with open("../loopback/playedFrames.log","wb") as output:
	for item in playedFrames:
		output.write(item + "\n")

with open("../loopback/missingFrames.log","wb") as output:
	for item in missingFrames:
		output.write(item + "\n")