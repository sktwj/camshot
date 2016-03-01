# Welcome to the home of camshot #

**camshot** is a very small tool for capturing images from a v4l2 compatibile camera or webcam in a linux console. It can be useful in many areas. For instance:

  * Linux powered security systems
  * Linux powered robotic systems
  * Linux powered embedded systems
  * Linux scripts
  * Testing camera and getting info about it
  * ...

## News ##
**camshot** now supports shared memory & semaphores for interprocess communication.

## Installing ##
  1. Get the sources from downloads section or from the svn. You could also get the binary from the downloads section (the binary is not guaranteed to be the latest version all the time).
  1. Go to the source directory and execute: `make && make install`
  1. If you don't get an error you can start using camshot. **Note:** You need to be root (have write access to /usr/bin) for the `make install` to work properly!

## Usage examples ##
### Example 1: ###
Capturing 160x120 bmp images using standard input / output. The files will be in the default destination: /tmp/camshot\_TIMESTAMP.bmp where TIMESTAMP is your system current timestamp.
```
~ $ camshot -W 160 -H 120 -o ./
Letting the camera automaticaly adjust the picture:..........Done.
Command (h for help): h

Commands:
	x	Capture a picture from camera.
	h	Prints this help.
	q	Quits the program.

Command (h for help): x
Command (h for help): q
~ $
```

### Example 2: ###
Capturing 320x240 bmp images using named pipe.
In one console execute:
```
~ $ camshot -W 320 -H 240 -p ./campipe
Letting the camera automaticaly adjust the picture:..........Done.
```
In a different console execute:
```
~ $ cat ./campipe > /tmp/tst.bmp
```
And as you can see below you can find the captured image in the desired destination /tmp/tst.bmp.
```
~ $ ls /tmp/ | grep tst
/tmp/tst.bmp
~ $ 
```
Once done capturing the images simply kill the camshot process:
```
^CCaught CTRL+C, camshot ending
~ $ 
```

## Current features ##
Current features can be seen in the help of camshot:
```
$ camshot -h

		./camshot - Linux console webcam screenshot utility

Usage: ./camshot [OPTIONS]

Options:
	--help		-h	Prints this help.
	--device dev	-d dev	Works with the dev device (default /dev/video0)
	--outdir dir	-o dir	Saves all output in dir (default /tmp)
	--format fmt	-f fmt	Saves images in the fmt format (see below)
	--info		-i	Prints info about the video device and stops.
	--width num	-W num	Sets the desired width (if the camera supports it)
	--height num	-H num	Sets the desired height (if the camera supports it)
	--pipe file	-p file	Uses named pipe file as output for images
	--shm id	-s id	Uses shared memory as output for images. See below for more info

Image formats (fmt):
	rgb - raw rqb binary file
	bmp - bmp image (default)

Option --shm id / -s id:
	If you choose this option camshot will store the images from the camera to a
region of memory which it will share with a key id so a users process can use this region
for its input. The writes to this region are protected  by a semaphore with the same key 
as the memory region. It is important that the user process uses this semaphore while 
reading the buffer!
$
```
## Coming soon ##
  * jpg and png support
  * virtual memory file mapping support for inter process communication.

## Requesting features ##
Request features by writing to my email below or by writing to the mailing list.

## Contributing ##
Contributors, testers etc are more than welcome to join. Send patches/suggestions/requests to the mailing list.



&lt;hr&gt;



**Copyright (c) 2010 Gabriel Zabusek <gabriel.zabusek@gmail.com>**

**This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version
3 of the License, or (at your option) any later version.**