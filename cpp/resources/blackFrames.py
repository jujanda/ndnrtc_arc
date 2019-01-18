# coding: utf-8
from shutil import copyfile
import os
import numpy as np
import sys


fileName1 = ""
fileName2 = ""
resolution = sys.argv[1] 
fps = sys.argv[2] 
frame_src = "../loopback/frames"
frame_dst = "../loopback/frames_padded"
blk_frame_src = "../tests/black_frame_" + resolution + ".png"
missingFrames = []
srcFrameCounter = 1
dstFrameCounter = 1
blkFrameCounter = 0

# Get missing frame numbers from file
with open("../loopback/missingFrames.log","r") as input:
   	file = input.readlines()
	for line in file:
		missingFrames.append(line.strip())

# go through all the frames
numberOfFrames = len(os.listdir(frame_src))
while srcFrameCounter <= numberOfFrames:

	# if frame is missing
	if str(dstFrameCounter) in missingFrames:

		# check if frames are missing at the start...
		if srcFrameCounter <= 1:

			# copy black frame
			fileName1 = ""
			fileName2 = str(dstFrameCounter) + ".png"
			copyfile(blk_frame_src, frame_dst + "/" + fileName2)
			srcFrameCounter += 0
			dstFrameCounter += 1
			blkFrameCounter += 1

		# ... or somewhere in the middle
		else:

			# copy last frame	
			fileName1 = str(srcFrameCounter - 1) + ".png"
			fileName2 = str(dstFrameCounter) + ".png"
			copyfile(frame_src + "/" + fileName1, frame_dst + "/" + fileName2)
			srcFrameCounter += 0
			dstFrameCounter += 1

	# if frame is present
	else:
		# copy actual frame
		fileName1 = str(srcFrameCounter) + ".png"
		fileName2 = str(dstFrameCounter) + ".png"
		copyfile(frame_src + "/" + fileName1, frame_dst + "/" + fileName2)
		srcFrameCounter += 1
		dstFrameCounter += 1

# passing the calculated offset to the executing script
offset = float(blkFrameCounter) / float(fps)
print offset