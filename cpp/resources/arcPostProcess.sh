#!/bin/bash

# Create variables
echo '> Creating variables'
PATH_IN="../tests"
PATH_OUT="../loopback"
PATH_SRC=$(pwd)
RES_H=256
RES_W=144

# Create folders
echo '> Creating folders'
mkdir $PATH_OUT/frames
mkdir $PATH_OUT/frames_padded

# Run analytics
echo '> Running analytics'
cd $PATH_OUT
git clone https://github.com/peetonn/ndnrtc-tools &>> $PATH_OUT/arc_PostProcess.log && export PATH=$PATH:$(pwd)/ndnrtc-tools &>> $PATH_OUT/arc_PostProcess.log
prep-logs.sh &>> $PATH_OUT/arc_PostProcess.log
../resources/report-loopback.sh &>> $PATH_OUT/arc_PostProcess.log
cd $PATH_SRC

# Determine missing frames
echo '> Determining missing frames'
python arcMissingFrames.py &>> $PATH_OUT/arc_PostProcess.log

# Transforming .raw file into viewable format
echo '> Transforming .raw file into viewable format'
ffmpeg -f rawvideo -vcodec rawvideo -s $RES_H'x'$RES_W -r 30 -pix_fmt argb -i $PATH_OUT/producer-camera.$RES_H'x'$RES_W -c:v libx264 -preset ultrafast -qp 0 $PATH_OUT/producer-camera.avi &>> $PATH_OUT/arc_PostProcess.log

# Extracting frames of output video as images
echo '> Extracting frames of output video as images'
ffmpeg -i $PATH_OUT/producer-camera.avi $PATH_OUT/frames/%01d.png &>> $PATH_OUT/arc_PostProcess.log

# Fill in missing frames
echo '> Filling in missing frames'
python blackFrames.py $RES_H'x'$RES_W &>> $PATH_OUT/arc_PostProcess.log

# Reconstruct video from list of images
echo '> Reconstructing video from list of images'
ffmpeg -framerate 30 -i $PATH_OUT/frames_padded/%01d.png -c:v libx264 -crf 0 -r 30 -preset ultrafast -pix_fmt argb $PATH_OUT/producer-camera_padded.avi &>> $PATH_OUT/arc_PostProcess.log

# FFMPEG, PSNR calculation
echo '> Calculating PSNR'
ffmpeg -i $PATH_OUT/producer-camera_padded.avi -i $PATH_IN/in_$RES_H'x'$RES_W.avi -lavfi  psnr=$PATH_OUT/arc_psnr.log -f null - &>> $PATH_OUT/arc_PostProcess.log

# FFMPEG, SSIM calculation
echo '> Calculating SSIM'
ffmpeg -i $PATH_OUT/producer-camera_padded.avi -i $PATH_IN/in_$RES_H'x'$RES_W.avi -lavfi  ssim=$PATH_OUT/arc_ssim.log -f null - &>> $PATH_OUT/arc_PostProcess.log

# FFMPEG, VMAF calculation
echo '> Calculating VMAF'
# ./run_vmaf format width height reference_path distorted_path [--out-fmt output_format]
./vmaf/run_vmaf yuv420p $RES_W $RES_H $PATH_IN/in_$RES_H'x'$RES_W.avi $PATH_OUT/producer-camera_padded.avi --out-fmt json &>> $PATH_OUT/arc_PostProcess.log

