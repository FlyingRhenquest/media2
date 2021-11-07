# Client Demo

This is a demo client for the router. It can upload media to my router.
It'll print the main job ID and the job IDs of your streams. At the
moment it won't tell you which stream is which but I'll probably add
that functionality at some point.

My intention is to later reassemble the media files by Job ID.

Call it with ./media2_client filename.mp4

filename can be anything ffmpeg can handle. Files. Rtmp streams. Files
stored on the web. Yadda, yadda.

The demo is designed to run in a docker container, so it may complain
to you about some undefined environment variables. I'll try to keep
those the same as the media2 router ones.