# Media2 Storage Demo Service

Subscribes to the outbound segment stream for the router and stores the video
segments it produces. This is just pass-through storage, no transcoding takes
place in this service.

It also doesn't stich the segments back together. They'll be stored by their
segment timestamp, which is the timestamp if the iframe / first packet in the
segment. Reassembling them later should therefore be pretty easy, just read the
segments back in, sort them on their timestamp and start popping the packets out.

