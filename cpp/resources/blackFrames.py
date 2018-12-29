# coding: utf-8
from shutil import copyfile
import os
import numpy as np
import sys


fileName1 = ""
fileName2 = ""
resolution = sys.argv[1] 
frame_src = "../loopback/frames"
frame_dst = "../loopback/frames_padded"
blk_frame_src = "../tests/black_frame_" + resolution + ".png"
missingFrames = []
srcFrameCounter = 1
dstFrameCounter = 1

# Get missing frame numbers from file
with open("../loopback/missingFrames.log","r") as input:
   	file = input.readlines()
	for line in file:
		missingFrames.append(line.strip())

for file in os.listdir(frame_src):
	if str(dstFrameCounter) in missingFrames:
		# copy black frame
		fileName1 = ""
		fileName2 = str(dstFrameCounter) + ".png"
		copyfile(blk_frame_src, frame_dst + "/" + fileName2)
		srcFrameCounter += 0
		dstFrameCounter += 1
		
	else:
		# copy actual frame
		# print("copy actual frame at " + str(dstFrameCounter))
		fileName1 = str(srcFrameCounter) + ".png"
		fileName2 = str(dstFrameCounter) + ".png"
		copyfile(frame_src + "/" + fileName1, frame_dst + "/" + fileName2)
		srcFrameCounter += 1
		dstFrameCounter += 1
