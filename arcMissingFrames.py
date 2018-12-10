# coding: utf-8
import matplotlib
import matplotlib.pyplot as plt
import numpy as np


# ===================================

# Define Variables
requestedFrames = []
playedFrames = []
missingFrames = []

# ???
with open("./all.log","r") as input:
	with open("./requestedFrames.log","wb") as output:
		file = input.readlines()
   		for line in file:
   			linePosition = line.find("▷▷▷")
   			if linePosition >= 0:
   				record = line.split(" ")
   				for item in record:
   					itemPosition = item.find("/")
   					if itemPosition >= 0:
   						requestedFrames.append(item + "\n")


with open("./all.log","r") as input:
	with open("./playedFrames.log","wb") as output:
		file = input.readlines()
   		for line in file:
   			linePosition = line.find("●--")
   			if linePosition >= 0:
   				record = line.split(" ")
   				for item in record:
   					itemPosition = item.find("/")
   					if itemPosition >= 0:
   						playedFrames.append(item + "\n")

# Delete duplicates from list
requestedFrames = list(set(requestedFrames))

# Print key frames
# for item in playedFrames:
# 	if "k" in item:
# 		print(item)

# Find missing frames
for item in requestedFrames:
	if item not in playedFrames:
		missingFrames.append(item)
		# missingFrames.append(item.split("/")[3] + "\n")

# Write to files
# with open("./requestedFrames.log","wb") as output:
# 	for item in requestedFrames:
# 		output.write(item)

# with open("./playedFrames.log","wb") as output:
# 	for item in playedFrames:
# 		output.write(item)

with open("./missingFrames.log","wb") as output:
	for item in missingFrames:
		output.write(item)