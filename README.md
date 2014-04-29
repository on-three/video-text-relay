#VideoTextOverlay
Simple Gstreamer based video text overlay. Driven by JSON RPC calls.


#Overview
This is a series of simple c++ executables that recreate some work i had done using the Gstreamer python binding. But since the python binding is very unstable (0.1 version does not support .nsv stream container, 1.0 version does not yet support Cairo) I'm reimplimenting in straight C/C++ which appears more mature.

None of this is very involved, and it can be considered pretty 'typical' GStreamer code.

#Status
This is all still fairly primitive, mostly as i'm reimplimenting previous work in python. Still, at this point, the following is accomplished:
* **relay_stream:** relays an existing HTTP video+audio stream to a (local) TCP connection, visible to VLC (for example).
* **relay_stream_with_overlay:** As relay_stream above, but with a demo text crawl superimposed on the stream.
* **json_rpc_client_test:** A test application to send JSON Remote Procedure Calls to corresponding JSON RPC servers. This makes a single remote procedure call as an example
* **json_rpc_server_test:** A test application to receive JSON Remote Procedure calls. Works to test the _client_test application data above.
* **json_rpc_relay:** The above **relay_stream_with_overlay** executable with the addition of an embedded (simple) RPC server. This can receive a single remote procedure call that chages the displayed scrolling text on the screen.

#building
I've tried to make it as simple to compile as possible, but this still relies upon having development packages for Gtk and GStreamer being available. I've only built it in LinuxMint16, where the following command _should_ cover all the ##Package Dependencies:

```
sudo apt-get install g++ libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libcairo2-dev libglib2.0-dev libgstreamer-plugins-good1.0-dev
```

##libjson-rpc-cpp
The only dependency that isn't widely available via package manager is this library support for JSON Remote Procedure Calls. I really want to use this, so i'm not removing this dependency. It must be locally built and insalled to build this code.
To do so, simply follow the instructions on this page: https://github.com/cinemast/libjson-rpc-cpp.

##Make
If those packages are available, you should be able to simply download the repository, move to the src directory and build.

```
git clone https://github.com/on-three/VideoTextOverlay
cd VideoTextOverlay/src
make
```

Haven't tried to build on Windows yet. In theory it ought to build, but I don't think I'll take the time to do so.

#Modules
I'll try to describe each executable that is built in turn, from simplest to most complex.

##relay_stream
This is a basic Gstreamer pipleine that connects to a remote HTTP video+audio stream and relays it to a local TCP server. It also transcodes the stream to TS(h254/mp3), regardless of the original stream container/codecs. An example run is as follows (simply providing the stream URL as the single input argument):


###running
```
~/code/VideoTextOverlay/src $ ./relay_stream http://<stream URL>/;stream.nsv
Received new pad 'src_0' from 'demux':
  Link succeeded (type 'video/x-raw').
Received new pad 'src_1' from 'demux':
  Link succeeded (type 'audio/x-raw').
```
At this point, you can connect to the relay via TCP using VLC. The VLC stream url is "tcp://127.0.0.1:10000"
I've currently hard coded the port to be 10000 as this is just a demo.

##relay_stream_with_overlay
  This is very similar to the previous executable, but with the addition of a simple superimposed scrolling demo text.
###running
Run as below. The stream URL is still the only argument. TCP Server port is still hard coded to 10000.
```
./relay_stream_with_overlay http://<stream URL>t:9613/;stream.nsv
[MSG]	Received new pad 'src_0' from 'demux':
[OK]	Pipeline of type 'video/x-raw' is now online.
[MSG]	Received new pad 'src_1' from 'demux':
[OK]	Pipeline of type 'audio/x-raw' is now online.

```
You can then connect VLC (and other media players?) to 'TCP://127.0.0.1:10000' and you should see something like the image below.

