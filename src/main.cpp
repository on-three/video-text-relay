#include "colors.h"
#include <string>
#include <gst/gst.h>
#include "relay.hpp"


/******************************************************************************
function: main()
currently takes one argument: url of stream to relay
Can connect to this realy via TCP on port 1000
(e.g) tcp://127.0.0.1:10000
******************************************************************************/
int main(int argc, char *argv[]) {

  /* Initialize GStreamer */
  gst_init (&argc, &argv);

  //MyStubServer s(&queue);

  //draw url to relay out of our command line args
  if(argc<2) {
    g_print ("********relay-stream-with-overlay********"
    BOLDWHITE "\nusage: relay-stream-with-overlay [URL of http stream to relay]" RESET
    "\nIf successful, program should report correct stream online status."
    "\nClients (i.e. vlc) can then connect to the relay via TCP on port 10000"
    BOLDRED "\nPort value 10000 " RESET "is currently hard coded."
    "\nThe URL i'm using for VLC is " BOLDWHITE "\"tcp://127.0.0.1:10000\"" RESET
    "\nfor a vlc instance on the same host."
    "\nSome work needs to be done to make a more scalable solution.");
    return -1;
  }

  const std::string uri = argv[1];

  try {
    Relay relay(std::string("my_relay"), uri);

    relay.Initialize();
    relay.Run();

  }catch(...){
    g_printerr("Exception thrown. exiting.");
  }

  return 0;
}