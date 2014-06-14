#include "colors.h"
#include <string>
#include <gst/gst.h>
#include "relay.hpp"
#include <tclap/CmdLine.h>
#include <iostream>

using std::cout;
using std::endl;

/******************************************************************************
function: main()
currently takes one argument: url of stream to relay
Can connect to this realy via TCP on port 1000
(e.g) tcp://127.0.0.1:10000
******************************************************************************/
int main(int argc, char *argv[]) {

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  std::string url = "";
  int relay_port = 10000;
  int jsonrpc_port = 8080;
  try {
    TCLAP::CmdLine cmd("********relay-stream-with-overlay********"
    BOLDWHITE "Video stream realy with json RPC text overlay server." RESET
    "If successful, program should report correct stream online status."
    "Clients (i.e. vlc) can then connect to the relay via TCP on port 10000.\n"
    BOLDRED "Port value 10000 " RESET "is currently hard coded."
    "The URL i'm using for VLC is " BOLDWHITE "\"tcp://127.0.0.1:10000\"" RESET
    "for a vlc instance on the same host."
    "Some work needs to be done to make a more scalable solution.",
     ' ', //delimiter between command line args
     "0.1a"); //version of program

    //primary positional arg is the URL of the video stream.
    TCLAP::UnlabeledValueArg< std::string > stream_url(std::string("url"),
      std::string("URL of video stream to relay."),
      true,//required
      std::string(""),
      std::string("URL of video stream to relay"));
    cmd.add(stream_url);

    //TCP port we'll relay on (i.e. clients can connect to)
    TCLAP::ValueArg<int> relay_port_option(std::string("p"),
      std::string("realy_port"),
      std::string("Port TCP clients can connect to to receive relayed, transcoded, decorated video stream."),
      false,//not required
      10000,//default value
      std::string("port number"));
    cmd.add(relay_port_option);

  //TCP port we'll accept json-rpc commands on
    TCLAP::ValueArg<int> jsonrpc_port_option(std::string("j"),
      std::string("jsonrpc_port"),
      std::string("Port TCP clients can pass JSON RPC commands to display text atop stream."),
      false,//not required
      8080,//default value
      std::string("port number"));
    cmd.add(jsonrpc_port_option);

    cmd.parse(argc, argv);
    url = stream_url.getValue();
    jsonrpc_port = jsonrpc_port_option.getValue();
    relay_port = relay_port_option.getValue();

  } catch (TCLAP::ArgException &e)  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << endl;
    return -1;
  }

  try {
    Relay relay(std::string("my_relay"), url, relay_port, jsonrpc_port);

    relay.Initialize();
    relay.Run();

  }catch(...){
    g_printerr("Exception thrown. exiting.");
  }

  return 0;
}