# Router Demo

This is a AV router for several of my tech demos. It receives metadata related to a job and a job
with a unique job ID. This contains the filename, unique IDs of the streams and some other assorted
metadata (See my client demo.)

This receives with Zmq PUB/SUB and distributes the work in the same way. Any of my demos that are
running will receive the video from the client. The router will also set up the work directory and
store the streams sent from the user.
