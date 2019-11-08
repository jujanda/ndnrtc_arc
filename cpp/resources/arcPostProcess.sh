#!/bin/bash

# Create variables
echo '> Creating variables'
PATH_IN="/home/nfd/ndnrtc_arc/cpp/tests"
PATH_EVALUATION=$1
SETTING=$2
RUN=$3
PATH_EXECUTE=$(pwd)
LOW_RES_W=107
LOW_RES_H=60
MED_RES_W=213
MED_RES_H=120
HIGH_RES_W=426
HIGH_RES_H=240
FPS=24
OFFSET=0

# Reconstruct file name from arguments
PATH_RESULTS=$PATH_EVALUATION'setting_'$SETTING/results/$RUN/

# Delete empty results
find $PATH_RESULTS -name 'producer-camera_*' -type 'f' -size 0k -delete

# Run analytics (single-use operation)
echo '> Running analytics'
cd $PATH_RESULTS
git clone https://github.com/peetonn/ndnrtc-tools &>> arc_PostProcess.log && export PATH=$PATH:$(pwd)/ndnrtc-tools &>> arc_PostProcess.log
prep-logs.sh &>> arc_PostProcess.log
killall gnuplot_qt
$PATH_EXECUTE/resources/report-loopback.sh &>> arc_PostProcess.log
cd $PATH_EXECUTE
echo '==================================\n' &>> $PATH_RESULTS/arc_PostProcess.log

# Determine missing frames
echo '> Determining missing frames'
python resources/arcMissingFrames.py $PATH_RESULTS &>> $PATH_RESULTS/arc_PostProcess.log
echo '==================================\n' &>> $PATH_RESULTS/arc_PostProcess.log

# Calculate average throughput
echo '> Calculating average throughput'
python resources/arcThroughput.py $PATH_EVALUATION $SETTING $RUN &>> $PATH_RESULTS/arc_PostProcess.log
echo '==================================\n' &>> $PATH_RESULTS/arc_PostProcess.log

# Extracting frames of output videos as images and combine them
echo '> Extracting and combining frames'
python resources/arcCombineResolutions.py $PATH_RESULTS $LOW_RES_W'x'$LOW_RES_H $MED_RES_W'x'$MED_RES_H $HIGH_RES_W'x'$HIGH_RES_H $FPS &>> $PATH_RESULTS/arc_PostProcess.log
echo '==================================\n' &>> $PATH_RESULTS/arc_PostProcess.log

# FFMPEG, PSNR calculation
echo '> Calculating PSNR'
# ffmpeg -i /media/julian/extHD/setting_87/results/1/combined.avi -f rawvideo -vcodec rawvideo -s 1280x720 -r 24 -pix_fmt argb -i /home/nfd/ndnrtc_arc/cpp/tests/in_1280x720.raw -lavfi  psnr=/media/julian/extHD/setting_87/results/1/arc_psnr.log -f null -
ffmpeg -i $PATH_RESULTS/combined.avi -f rawvideo -vcodec rawvideo -s $HIGH_RES_W'x'$HIGH_RES_H -r $FPS -pix_fmt argb -i $PATH_IN/in_$HIGH_RES_W'x'$HIGH_RES_H.raw -lavfi  psnr=$PATH_RESULTS/arc_psnr.log -f null - &>> $PATH_RESULTS/arc_PostProcess.log
echo '==================================\n' &>> $PATH_RESULTS/arc_PostProcess.log

# FFMPEG, SSIM calculation
echo '> Calculating SSIM'
# ffmpeg -i /media/julian/extHD/setting_87/results/1/combined.avi -f rawvideo -vcodec rawvideo -s 1280x720 -r 24 -pix_fmt argb -i /home/nfd/ndnrtc_arc/cpp/tests/in_1280x720.raw -lavfi  ssim=/media/julian/extHD/setting_87/results/1/arc_ssim.log -f null -
ffmpeg -i $PATH_RESULTS/combined.avi -f rawvideo -vcodec rawvideo -s $HIGH_RES_W'x'$HIGH_RES_H -r $FPS -pix_fmt argb -i $PATH_IN/in_$HIGH_RES_W'x'$HIGH_RES_H.raw -lavfi  ssim=$PATH_RESULTS/arc_ssim.log -f null - &>> $PATH_RESULTS/arc_PostProcess.log
echo '==================================\n' &>> $PATH_RESULTS/arc_PostProcess.log

# FFMPEG, VMAF calculation
echo '> Calculating VMAF'
# ./run_vmaf format width height reference_path distorted_path [--out-fmt output_format]
# ./resources/vmaf/run_vmaf yuv420p 1280 720 /home/nfd/ndnrtc_arc/cpp/tests/in_1280x720.raw /media/julian/extHD/setting_87/results/1/combined.avi --out-fmt json
./resources/vmaf/run_vmaf yuv420p $HIGH_RES_W $HIGH_RES_H $PATH_IN/in_$HIGH_RES_W'x'$HIGH_RES_H.raw $PATH_RESULTS/combined.avi --out-fmt json &>> $PATH_RESULTS/arc_PostProcess.log
echo '==================================\n' &>> $PATH_RESULTS/arc_PostProcess.log

# echo '> Deleting raw files'
rm -f $PATH_RESULTS/producer-camera_*

echo '>> Postprocessing run '$SETTING' '$RUN' finished! <<'

# Shortcut for quick results
# echo '> Quick results:'
# python resources/arcCountMissingFrames.py $PATH_EVALUATION $SETTING $RUN
# python resources/arcThroughput.py $PATH_EVALUATION $SETTING $RUN
# ls -ahl $PATH_RESULTS | grep producer-camera.*
