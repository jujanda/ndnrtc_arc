#!/bin/bash

# Input files
# 	in_$RES.avi
# 	in_$RES.raw
#	black_frame.png

# Output files
# 	producer-camera.$RES
# 	producer-camera.mp4

# Create variables
echo 'Create variables'
PATH_IN="../tests"
PATH_OUT="../loopback"
PATH_SRC=$(pwd)
RES=256x144

# Create folders
echo 'Create folders'
mkdir $PATH_OUT/frames
mkdir $PATH_OUT/frames_padded

# Run analytics
echo 'Run analytics'
cd $PATH_OUT
git clone https://github.com/peetonn/ndnrtc-tools && export PATH=$PATH:$(pwd)/ndnrtc-tools
prep-logs.sh
../resources/report-loopback.sh
cd $PATH_SRC

# Determine missing frames
echo 'Determine missing frames'
python arcMissingFrames.py 

# Transforming .raw file into viewable format
echo 'Transforming .raw file into viewable format'
ffmpeg -f rawvideo -vcodec rawvideo -s $RES -r 30 -pix_fmt argb -i $PATH_OUT/producer-camera.$RES -c:v libx264 -preset ultrafast -qp 0 $PATH_OUT/producer-camera.avi

# Extracting frames of output video as images
echo 'Extracting frames of output video as images'
ffmpeg -i $PATH_OUT/producer-camera.avi $PATH_OUT/frames/%01d.png

# Fill in missing frames
echo 'Fill in missing frames'
# TODO create black frames fo all used resolutions and use $RES to take right one here
python blackFrames.py $RES

# Reconstruct video from list of images
echo 'Reconstruct video from list of images'
ffmpeg -framerate 30 -i $PATH_OUT/frames_padded/%01d.png -c:v libx264 -crf 0 -r 30 -preset fast -pix_fmt yuv420p $PATH_OUT/producer-camera_padded.avi

# FFMPEG, PSNR calculation
echo 'FFMPEG, PSNR calculation'
ffmpeg -s $RES -r 30 -i $PATH_IN/in_$RES.avi -s $RES -r 30 -i $PATH_OUT/producer-camera_padded.avi -lavfi "[0:v][1:v]psnr" -f null -