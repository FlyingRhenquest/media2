# Transcoder Demo

This is designed to subscribe to the router on the outgoing segment address.
When segments arrive, the transcoder will pick them up. If they're 4K, they'll
be transcoded to 1080 and 720. If they're 1080, they'll be transcoded to 720.
All transcoded segments will be stored in uuid_1080 and uuid_720 in the same
directory as the original packets.

The transcoder is composed of two parts; the front end subscribes to the router
and buffers segments passed to it. The back end can be run on multiple machines
and connects to the transcoder front end to request segments to process
whenever they need some.