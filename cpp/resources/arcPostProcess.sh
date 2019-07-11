#!/bin/bash

# Create variables
echo '> Creating variables'
PATH_IN="tests"
PATH_OUT="loopback"
PATH_SRC=$(pwd)
RES_W=256
RES_H=144
FPS=30
OFFSET=0

# Create folders
echo '> Creating folders'
mkdir $PATH_OUT/frames
mkdir $PATH_OUT/frames_padded

# Run analytics
echo '> Running analytics'
cd $PATH_OUT
git clone https://github.com/peetonn/ndnrtc-tools &>> arc_PostProcess.log && export PATH=$PATH:$(pwd)/ndnrtc-tools &>> arc_PostProcess.log
prep-logs.sh &>> arc_PostProcess.log
./resources/report-loopback.sh &>> arc_PostProcess.log
cd $PATH_SRC
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# Determine missing frames
echo '> Determining missing frames'
python ./resources/arcMissingFrames.py &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# Transforming .raw file into viewable format
echo '> Transforming .raw file into viewable format'
ffmpeg -f rawvideo -vcodec rawvideo -s $RES_W'x'$RES_H -r 30 -pix_fmt argb -i $PATH_OUT/producer-camera.$RES_W'x'$RES_H -c:v libx264 -preset ultrafast -qp 0 $PATH_OUT/producer-camera.avi &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# Extracting frames of output video as images
echo '> Extracting frames of output video as images'
ffmpeg -i $PATH_OUT/producer-camera.avi $PATH_OUT/frames/%01d.png &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# Fill in missing frames
echo '> Filling in missing frames'
OFFSET=`python ./resources/blackFrames.py $RES_W'x'$RES_H $FPS`
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# Reconstruct video from list of images
echo '> Reconstructing video from list of images'
ffmpeg -framerate 30 -i $PATH_OUT/frames_padded/%01d.png -c:v libx264 -crf 0 -r 30 -preset ultrafast -pix_fmt argb $PATH_OUT/producer-camera_padded.avi &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# Trim original video for fair comparison
echo '> Trimming videos for fair comparison'
ffmpeg -ss $OFFSET -i $PATH_IN/in_$RES_W'x'$RES_H.avi -c:v libx264 $PATH_OUT/in_$RES_W'x'$RES_H'_adjusted'.avi &>> $PATH_OUT/arc_PostProcess.log
ffmpeg -ss $OFFSET -i $PATH_OUT/producer-camera_padded.avi -c:v libx264 $PATH_OUT/producer-camera_padded'_adjusted'.avi &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# FFMPEG, PSNR calculation
echo '> Calculating PSNR'
ffmpeg -i $PATH_OUT/producer-camera_padded'_adjusted'.avi -i $PATH_OUT/in_$RES_W'x'$RES_H'_adjusted'.avi -lavfi  psnr=$PATH_OUT/arc_psnr.log -f null - &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# FFMPEG, SSIM calculation
echo '> Calculating SSIM'
ffmpeg -i $PATH_OUT/producer-camera_padded'_adjusted'.avi -i $PATH_OUT/in_$RES_W'x'$RES_H'_adjusted'.avi -lavfi  ssim=$PATH_OUT/arc_ssim.log -f null - &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log

# FFMPEG, VMAF calculation
echo '> Calculating VMAF'
# ./run_vmaf format width height reference_path distorted_path [--out-fmt output_format]
./vmaf/run_vmaf yuv420p $RES_W $RES_H $PATH_OUT/in_$RES_W'x'$RES_H'_adjusted'.avi $PATH_OUT/producer-camera_padded'_adjusted'.avi --out-fmt json &>> $PATH_OUT/arc_PostProcess.log
echo '==================================' &>> $PATH_OUT/arc_PostProcess.log
