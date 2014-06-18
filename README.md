#video-text-relay
Simple Gstreamer based video stream relay which can overlay text of various sorts. Text can be set by tcp clients via JSON RPC calls, allowing simply coded daemons to dynamically change text superimposed on video streams.


#Overview
This code builds a single executable, `video-text-relay`, which does the following:
* Connects to an HTTP served viseo stream (of various containers/audio+video encodings)
* Renders text on the decoded video
* Reencodes the video to a standard MPEGTS(h264/mp3) video with text overlays
* Relays the video stream to a local TCP server currently at 127.0.0.1:10000
* Text can be added/removed via JSON RPC calls currently on 127.0.0.1:8080

None of this is very involved, and it can be considered pretty 'typical' GStreamer code.

#Status
I'm moving this forward, but still fairly primitive. Currently static and scrolling text displays are available, which can be added and removed via simple python JSON RPC clients.

#building
I've tried to make it as simple to compile as possible, but this still relies upon having development packages for Gtk and GStreamer being available. I've only built it in LinuxMint16, where the following command _should_ cover all the Package Dependencies:

```
sudo apt-get install g++ libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libcairo2-dev libglib2.0-dev libgstreamer-plugins-good1.0-dev libpangocairo-1.0-0 libtclap-dev libjsoncpp-dev libboost-regex-dev
```

##Make
If those packages are available, you should be able to simply download the repository, move to the src directory and build.

```
git clone https://github.com/on-three/VideoTextOverlay
cd VideoTextOverlay/src
make
```

Haven't tried to build on Windows yet. In theory it ought to build, but I don't think I'll take the time to do so.


##running video-text-realay
Run as below. The stream URL is  currently the only argument. TCP Server port is still hard coded to 10000.
```
./video-text-relay "http://<stream URL>:<PORT>/;stream.nsv"
[MSG]	Received new pad 'src_0' from 'demux':
[OK]	Pipeline of type 'video/x-raw' is now online.
[MSG]	Received new pad 'src_1' from 'demux':
[OK]	Pipeline of type 'audio/x-raw' is now online.

```
You can then connect VLC (and other media players?) to 'TCP://127.0.0.1:10000' and you should pick up the relay (with as yet no text!).

![Overlay Demo in VLC](https://github.com/on-three/VideoTextOverlay/blob/master/img/Screenshot%20from%202014-04-28%2018:49:00.png?raw=true)


##Adding Text
To add text overlays to the relayed video stream, 'video-text-realay' runs a simple http server that can pick up JSON RPC messages on (currently) port 8080. This allows JSON messages to control the text overlaid on the relayed stream. The required code for clients is fairly simple, and cound conceivably be coded in any language, but I'm currently using python.

Python scripts to set text on a running relay are available in the /python directory.

###scrolling_msg
This python script adds scrolling msg text to the video. Simple command line help is available:
```
./python/scrolling_msg -h
usage: scrolling_msg [-h] [-m MESSAGE] [-u URL] [-f FONT] [-t SCROLL_TIME]
                     [-l LOOPS] [-y YPOS] [-d]
                     message_name

Add or remove a scrolling message on a video stream.

positional arguments:
  message_name          Friendly name to remove/change this message later.

optional arguments:
  -h, --help            show this help message and exit
  -m MESSAGE, --message MESSAGE
                        Text to display on video stream scrolling msg
  -u URL, --url URL     URL of json RPC server to invoke commands on in form
                        IP:PORT.
  -f FONT, --font FONT  Pangocairo font family and size.
  -t SCROLL_TIME, --scroll_time SCROLL_TIME
                        Time in seconds to scroll text across screen.
  -l LOOPS, --loops LOOPS
                        Number of times (loops) to scroll the text. Value 0 is
                        show forever.
  -y YPOS, --ypos YPOS  Vertical y pos of scrolling text in pixels.
  -d, --delete          Delte scrolling message via the provided message name.

```
An example of superimposed video text is shown below.

![Overlay Demo in VLC](https://raw.githubusercontent.com/on-three/VideoTextOverlay/e3a66d8a2a544106cd3198091f11d275a18979f8/img/vlcsnap-2014-06-06-16h27m41s56.png)

###static_msg
This python script can apply staic (non moving) text to a specific location (x,y in pixels) over the relay stream. Simple help is avilable as below:
```
./python/static_msg -h
usage: static_msg [-h] [-m MESSAGE] [-u URL] [-f FONT] [-x XPOS] [-y YPOS]
                  [-d]
                  message_name

Add or remove a scrolling message on a video stream.

positional arguments:
  message_name          Friendly name to remove/change this message later.

optional arguments:
  -h, --help            show this help message and exit
  -m MESSAGE, --message MESSAGE
                        Text to display on video stream scrolling msg
  -u URL, --url URL     URL of json RPC server to invoke commands on in form
                        IP:PORT.
  -f FONT, --font FONT  Pangocairo font family and size.
  -x XPOS, --xpos XPOS  Time in seconds to scroll text across screen.
  -y YPOS, --ypos YPOS  Vertical y pos of scrolling text in pixels.
  -d, --delete          Delte scrolling message via the provided message name.

```
An example of some Staic text can be seen below:
![Overlay Demo in VLC](https://raw.githubusercontent.com/on-three/VideoTextOverlay/e3a66d8a2a544106cd3198091f11d275a18979f8/img/Screenshot%20from%202014-04-28%2019:57:16.png)

#What Needs to Be Done:
As stated above, this is fairly primitive, but I believe I've confronted all major hurdles to showing text superimposed upon a relayed video stream.
The primary outstanding areas where work needs to be done are:
* Currently these executables relay to local TCP clients (VLC via TCP connection provided by the GStreamer 'tcpsink' element). But a better model to stream to numerous remote clients may be needed. Perhaps 'tcpsink' need be replaced with the GStreamer shoutcast backend? Or their HTTP sink backend? This needs looking into.
* ~~I've only demonstrated a single RPC call that changes the text on the screen. I'd now have to (re) build code that allows the on-screen text to be manipulated in any number of ways: list boxes, time displays, text displays that use Pango Markup for text colors and weights.~~
* ~~The current code, though it works as a demo, is very poor quality. Needs a lot of work.~~
* ~~The JSON RPC mechanism cleanly separates relay servers from any possible client. But clients would have to be coded in some language to provide things like live IRC chats and other data.~~

