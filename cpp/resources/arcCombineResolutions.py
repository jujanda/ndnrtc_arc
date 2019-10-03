# coding: utf-8
import sys
import os
import glob
import subprocess



# Define parameters
PATH = sys.argv[1]
LOW_RES = sys.argv[2]
MED_RES = sys.argv[3]
HIGH_RES = sys.argv[4]
FPS = sys.argv[5]

# Define variables
# startingThread = ""
# listOfSwitches = []
listOfVids = glob.glob(PATH + "producer-camera_*")

# Define methods:
def file_len(fname):
    with open(fname) as f:
    	i = -1
        for i, l in enumerate(f):
            pass
    return i + 1

# Clear directories (DEBUG ONLY)
os.system("rm -r "+ PATH + "/frames/")

# # Get starting thread 
# # thread_to_fetch = "high";
# with open(PATH + "/tests/evaluation-consumer.cfg") as file:
# 	for line in file.readlines():
# 		pos = line.find("[thread_to_fetch]")
# 		if pos >= 0:
# 			listOfSwitches.append(line.split("\t")[-1].strip())	


# # Get thread switches
# with open(PATH + "arcLog_threadswitches.csv") as file:
# 	for line in file.readlines():
# 		pos = line.find("[switchingThread]")
# 		if pos >= 0:
# 			listOfSwitches.append(line.split("\t")[-1].strip())	

# print listOfSwitches
# print "length = " + str(len(listOfSwitches))

# Create directories
os.system("mkdir " + PATH + "frames/")
os.system("mkdir " + PATH + "frames/combined/")
for x in xrange(0,len(listOfVids)):
	os.system("mkdir " + PATH + "frames/" + str(x))

# Convert raw videos into frames
for vid in listOfVids:
	# /media/julian/extHD/setting_86/results/0/producer-camera_0.1280x720
	tmp = vid.split("_")[-1] # 0.1280x720
	num = tmp.split(".")[0] # 0
	res = tmp.split(".")[1] # 1280x720
	os.system("ffmpeg -f rawvideo -vcodec rawvideo -s " + res + " -r " + FPS + " -vsync 0 -pix_fmt argb -i " + vid + " " + PATH + "frames/" + num + "/%01d.png")


# Convert raw videos to avi
# ffmpeg -fflags +genpts -r 24 -t 00:00:11.25 -i producer-camera_854x480 -vcodec copy producer-camera_854x480.mp4
# ffmpeg -i producer-camera.854x480 -t 00:00:11.25 -c:v libx264 -preset ultrafast -qp 0 producer-camera.854x480.mp4
# ffmpeg -f rawvideo -vcodec rawvideo -s 854x480 -r 24 -pix_fmt argb -i producer-camera.854x480 -t 00:00:11.25 -vsync 0 -c:v libx264 -preset ultrafast -qp 0 producer-camera_854x480.avi
# os.system("ffmpeg -f rawvideo -vcodec rawvideo -s " + LOW_RES + " -r " + FPS + " -pix_fmt argb -i producer-camera." + LOW_RES + " -c:v libx264 -preset ultrafast -qp 0 producer-camera_" + LOW_RES + ".avi"

# Convert raw videos into frames
# ffmpeg -f rawvideo -vcodec rawvideo -s 854x480 -r 24 -pix_fmt argb -i producer-camera.854x480 ./frames/med/%01d.png
# ffmpeg -f rawvideo -vcodec rawvideo -s 854x480 -r 24 -pix_fmt argb -c:v libx264 -preset ultrafast -qp 0 -i producer-camera.854x480 ./frames/med/%01d.png
# ffmpeg -i producer-camera.854x480 -c:v copy -f mp4 producer-camera.854x480.mp4
# os.system("ffmpeg -f rawvideo -vcodec rawvideo -s " + LOW_RES + " -r " + FPS + " -vsync 0 -pix_fmt argb -i " + PATH + "producer-camera." + LOW_RES + " " + PATH + "frames/low/%01d.png")
# os.system("ffmpeg -f rawvideo -vcodec rawvideo -s " + MED_RES + " -r " + FPS + " -vsync 0 -pix_fmt argb -i " + PATH + "producer-camera." + MED_RES + " " + PATH + "frames/med/%01d.png")
# os.system("ffmpeg -f rawvideo -vcodec rawvideo -s " + HIGH_RES + " -r " + FPS + " -vsync 0 -pix_fmt argb -i " + PATH + "producer-camera." + HIGH_RES + " " + PATH + "frames/high/%01d.png")

# Initialize counters
absolute = 0;
lineCounter = 0;
# lowCounter = 1;
# medCounter = 1;
# highCounter = 1;
lastQuality = ""
folderCounter = -1
inFolderCounter = 1

# Get total number of frames
numberOfFrames = file_len(PATH + "publishedFrames.log")
print "numberOfFrames = " +  str(numberOfFrames)
numberOfPlayedFrames = file_len(PATH + "playedFrames.log")
print "numberOfPlayedFrames = " +  str(numberOfPlayedFrames)

# go through played frames, check for absolute numbers (last element split by "\t")
with open(PATH + "playedFrames.log","r") as input:
	file = input.readlines()
	
	while absolute < numberOfFrames: # TODO check if < or <= is right here
		
		# Extract information
		line = file[lineCounter].split("\t") # ["69d_high", "73"]
		tmp = line[0].split("_") # ["69d", "high"]
		# relative = tmp[0][:-1] # 69
		quality = tmp[1] # high

		# Check for quality switch
		if quality != lastQuality:
			folderCounter += 1
			inFolderCounter = 1
			print "Quality switch (" + lastQuality + " --> " + quality + ")"

		# # prepare according to quality
		# if quality == "low":
		# 	relative = lowCounter	
		# elif quality == "med":
		# 	relative = medCounter
		# elif quality == "high":
		# 	relative = highCounter


		if absolute != int(line[-1]): # 73
			if absolute > 0:
				# Copy last frame
				# cp /frames/combined/72.png /frames/combined/73.png
				os.system("cp " + PATH + "frames/combined/" + str((absolute - 1)) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")
			else:
				# Copy first frame that got actually played 
				# cp frames/0/69.png frames/combined/73.png
				os.system("cp " + PATH + "frames/0/" + str(inFolderCounter) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")
		else:
			if os.path.isfile(PATH + "frames/" + str(folderCounter) + "/" + str(inFolderCounter) + ".png"):
				# Fetch appropriate frame # mv frames/0/69.png frames/combined/73.png
				subprocess.call("mv " + PATH + "frames/" + str(folderCounter) + "/" + str(inFolderCounter) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png", shell=True)
			else:
				# Copy last frame
				# cp /frames/combined/72.png /frames/combined/73.png
				os.system("cp " + PATH + "frames/combined/" + str((absolute - 1)) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")


			# Increment counters
			lineCounter += 1
			inFolderCounter += 1
			# if quality == "low":	
			# 	lowCounter += 1
			# elif quality == "med":
			# 	medCounter += 1
			# elif quality == "high":
			# 	highCounter += 1

		absolute += 1
		lastQuality = quality
		
		if lineCounter >= numberOfPlayedFrames:
			break;

print "lineCounter = " + str(lineCounter)

# Combine frames into video
os.system("ffmpeg -framerate 24 -i " + PATH + "frames/combined/%0d.png -c:v libx264 -crf 0 -r 24 -preset fast -pix_fmt yuv420p " + PATH + "combined.avi")