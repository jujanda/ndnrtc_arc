# coding: utf-8
from shutil import copyfile
import numpy as np
import sys


fileName1 = ""
fileName2 = ""
frm_src = sys.argv[1]
frm_dst = sys.argv[2]
blk_frm_src = sys.argv[3]
missingFrames = []
srcFrameCounter = 0
dstFrameCounter = 0

# Get missing frame numbers from file
with open(frm_src + "/../missingFrames.log","r") as input:
   	file = input.readlines()
	for line in file:
		missingFrames.append(line)


for file in os.listdir(directory):
	if dstFrameCounter in missingFrames:
		# copy black frame
		fileName1 = blk_frm_src
		fileName2 = dstFrameCounter + ".png"
		dstFrameCounter += 1
	else:
		# copy actual frame
		fileName1 = srcFrameCounter + ".png"
		fileName2 = dstFrameCounter + ".png"
		srcFrameCounter += 1
		dstFrameCounter += 1

	copyfile(frm_src + fileName1, frm_dst + fileName2)
