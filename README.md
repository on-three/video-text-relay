#VideoTextOverlay
Simple Gstreamer based video text overlay. Driven by JSON RPC calls.


#Overview
This is a series of simple c++ executables that recreate some work i had done using the Gstreamer python binding. But since the python binding is very unstable (0.1 version does not support .nsv stream container, 1.0 version does not yet support Cairo) I'm reimplimenting in straight C/C++ which appears more mature.

None of this is very involved, and it can be considered pretty 'typical' GStreamer code.

#Status
This is all still fairly primitive, mostly as I'm reimplimenting previous work in python. Still, at this point, the following is accomplished:
* **relay_stream:** relays an existing HTTP video+audio stream to a (local) TCP connection, visible to VLC (for example).
* **relay_stream_with_overlay:** As relay_stream above, but with a demo text crawl superimposed on the stream.
* **json_rpc_client_test:** A test application to send JSON Remote Procedure Calls to corresponding JSON RPC servers. This makes a single remote procedure call as an example
* **json_rpc_server_test:** A test application to receive JSON Remote Procedure calls. Works to test the _client_test application data above.
* **json_rpc_relay:** The above **relay_stream_with_overlay** executable with the addition of an embedded (simple) RPC server. This can receive a single remote procedure call that chages the displayed scrolling text on the screen.

#building
I've tried to make it as simple to compile as possible, but this still relies upon having development packages for Gtk and GStreamer being available. I've only built it in LinuxMint16, where the following command _should_ cover all the Package Dependencies:

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
~/code/VideoTextOverlay/src $ ./relay_stream http://<stream URL>:<PORT>/;stream.nsv
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
./relay_stream_with_overlay http://<stream URL>:<PORT>/;stream.nsv
[MSG]	Received new pad 'src_0' from 'demux':
[OK]	Pipeline of type 'video/x-raw' is now online.
[MSG]	Received new pad 'src_1' from 'demux':
[OK]	Pipeline of type 'audio/x-raw' is now online.

```
You can then connect VLC (and other media players?) to 'TCP://127.0.0.1:10000' and you should see something like the image below.

![Overlay Demo in VLC](https://github.com/on-three/VideoTextOverlay/blob/master/img/Screenshot%20from%202014-04-28%2018:49:00.png?raw=true)


## json_rpc_server_test and json_rpc_client_test
These are two simple applications derived off the example code for the JSON RPC library above. They demonstrate calling a remote procedure (via TCP connection) on the server from the client.
I'm not going to describe them further here now, but as the json_rpc_client_test exe is used below, i'll describe it there.

## json_rpc_relay
Okay, this is the basic relay with text overlay demo above, but with the addition of a JSON RPC server. If we use the corect JSON RPC client (which here is C++, but could in theory be in any language, like python or Perl) we can invoke a remote procedure on the server to alter the displayed text.
This is just a proof of concept, but it does demonstrate where this could be carried. I'll address future work plans below.

### Running
To actually run this, you've got to run two executables in two shells. One is the realy (as above, providing a single URL as a command line parameter):

```
 ~/code/VideoTextOverlay/src $ ./json_rpc_relay http://green-oval.net<STREAM URL>:<PORT>/;stream.nsv
[MSG]	Received new pad 'src_0' from 'demux':
[OK]	Pipeline of type 'video/x-raw' is now online.
[MSG]	Received new pad 'src_1' from 'demux':
[OK]	Pipeline of type 'audio/x-raw' is now online.

```
Then, attach VLC to this relay to view the stream contents.

Lastly, you can now change the screen text by using the **json_rpc_client_test** executable. This takes at least one command line argument, the text you'd like to see superimposed on the screen.
```
./json_rpc_client_test "whut. a five second wait???" 100 200 "one"
```

If we run the above command, we can see the RPC is correctly picked up by our server/relay as additonal debug info is currently dumped to the screen:
```
~/code/VideoTextOverlay/src $ ./json_rpc_relay http://<stream URL>:<PORT>/;stream.nsv
[MSG]	Received new pad 'src_0' from 'demux':
[OK]	Pipeline of type 'video/x-raw' is now online.
[MSG]	Received new pad 'src_1' from 'demux':
[OK]	Pipeline of type 'audio/x-raw' is now online.
Showing message'whut. a five second wait???' at 100,200 named one
```
The results are shown below:

![Overlay Demo in VLC](https://github.com/on-three/VideoTextOverlay/blob/master/img/Screenshot%20from%202014-04-28%2019:57:16.png?raw=true)

#What Needs to Be Done:
As stated above, this is fairly primitive, but I believe I've confronted all major hurdles to showing text superimposed upon a relayed video stream.
The primary outstanding areas where work needs to be done are:
* Currently these executables relay to local TCP clients (VLC via TCP connection provided by the GStreamer 'tcpsink' element). But a better model to stream to numerous remote clients may be needed. Perhaps 'tcpsink' need be replaced with the GStreamer shoutcast backend? Or their HTTP sink backend? This needs looking into.
* I've only demonstrated a single RPC call that changes the text on the screen. I'd now have to (re) build code that allows the on-screen text to be manipulated in any number of ways: list boxes, time displays, text displays that use Pango Markup for text colors and weights.
* The current code, though it works as a demo, is very poor quality. Needs a lot of work.
* The JSON RPC mechanism cleanly separates relay servers from any possible client. But clients would have to be coded in some language to provide things like live IRC chats and other data.

