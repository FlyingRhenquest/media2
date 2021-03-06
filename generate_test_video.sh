#!/bin/bash
ffmpeg \
    -f lavfi -i testsrc=duration=10:size=1280x720:rate=30 \
    -f lavfi -i sine=duration=10 \
    -c:v h264 -speed 6 -pix_fmt yuvj420p -b:v 512k -r 30 \
    -c:a aac -threads 4 -b:a 256K -ac 2 -ar 48000 \
    test_data/testvideo.mp4
