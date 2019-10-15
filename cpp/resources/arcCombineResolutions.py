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
listOfVids = glob.glob(PATH + "producer-camera_*")

# Define methods:
def file_len(fname):
    with open(fname) as f:
    	i = -1
        for i, l in enumerate(f):
            pass
    return i + 1

# Clear directories (DEBUG ONLY)
os.system("rm -r "+ PATH + "frames/")

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
	
	# DEBUG message
	DIR = PATH + "frames/" + num
	print "Frames in folder " + str(num) + ": " + str(len([name for name in os.listdir(DIR) if os.path.isfile(os.path.join(DIR, name))]))


# Initialize counters
absolute = 0;
lineCounter = 0;
lastQuality = ""
folderCounter = 0
inFolderCounter = 1
correctEntry = True
firstEntry = True

# Get total number of frames and played frames
numberOfFrames = file_len(PATH + "publishedFrames.log")
numberOfPlayedFrames = file_len(PATH + "playedFrames.log")
# print "numberOfFrames = " +  str(numberOfFrames)
# print "numberOfPlayedFrames = " +  str(numberOfPlayedFrames)

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
		if (quality != lastQuality):

			# Make sure current folder is empty (workaround for losing big chunks of frames in a representation)
			# From https://stackoverflow.com/questions/2632205/how-to-count-the-number-of-files-in-a-directory-using-python
			DIR = PATH + "frames/" + str(folderCounter)
			if len([name for name in os.listdir(DIR) if os.path.isfile(os.path.join(DIR, name))]) == 0: 

				# Switch folders
				folderCounter += 1
				inFolderCounter = 1

			elif not firstEntry:
				# Reset quality (avoid switching to false one)
				quality = lastQuality
				correctEntry = False
				# print "false entry detected: " + str(line) + "\tfolder = " + str(folderCounter) + "\tquality = " + quality


		# Check if frame at current absolute number was played
		if absolute != int(line[-1]): # 73
			# Frame not played (missing)

			# Check if special handling for first frame is needed
			if absolute > 0:
				# Copy last frame
				# cp /frames/combined/72.png /frames/combined/73.png
				os.system("cp " + PATH + "frames/combined/" + str((absolute - 1)) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")
				# print("cp " + PATH + "frames/combined/" + str((absolute - 1)) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png" + " (frame not played)")
			else:
				# Copy first frame that got actually played 
				# cp frames/0/69.png frames/combined/73.png
				os.system("cp " + PATH + "frames/0/" + str(inFolderCounter) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")
				# print("cp " + PATH + "frames/0/" + str(inFolderCounter) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")
		else:
			# Frame was played

			# Check if frame is available as file
			if os.path.isfile(PATH + "frames/" + str(folderCounter) + "/" + str(inFolderCounter) + ".png") and correctEntry:
				# Fetch appropriate frame 
				# mv frames/0/69.png frames/combined/73.png
				subprocess.call("mv " + PATH + "frames/" + str(folderCounter) + "/" + str(inFolderCounter) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png", shell=True)
				# print("mv " + PATH + "frames/" + str(folderCounter) + "/" + str(inFolderCounter) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")

				# Increment counter
				inFolderCounter += 1			
			else:
				# Copy last frame
				# cp /frames/combined/72.png /frames/combined/73.png
				os.system("cp " + PATH + "frames/combined/" + str((absolute - 1)) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png")
				# print("cp " + PATH + "frames/combined/" + str((absolute - 1)) + ".png " + PATH + "frames/combined/" + str(absolute) + ".png" + " (frame played but missing)")

			# Increment counter
			lineCounter += 1
			
		# Clean up
		absolute += 1
		lastQuality = quality
		firstEntry = False
		correctEntry = True
		
		# Fail safe break
		if lineCounter >= numberOfPlayedFrames:
			break;

# Combine frames into viewable video
# ffmpeg -framerate 24 -i frames/combined/%0d.png -c:v libx264 -crf 0 -s 1280x720 -r 24 -preset fast -pix_fmt yuv420p combined.avi
os.system("ffmpeg -framerate " + FPS + " -i " + PATH + "frames/combined/%0d.png -c:v libx264 -crf 0 -s " + HIGH_RES + " -r " + FPS + " -preset fast -pix_fmt yuv420p " + PATH + "combined.avi")

print "---------------------------"
# Save snapshot of directory file numbers (for debugging)
for vid in listOfVids:
	# /media/julian/extHD/setting_86/results/0/producer-camera_0.1280x720
	tmp = vid.split("_")[-1] # 0.1280x720
	num = tmp.split(".")[0] # 0
	res = tmp.split(".")[1] # 1280x

	DIR = PATH + "frames/" + num
	print "Frames in folder " + str(num) + ": " + str(len([name for name in os.listdir(DIR) if os.path.isfile(os.path.join(DIR, name))]))

# Remove frames to save disk space
os.system("rm -r "+ PATH + "/frames")


