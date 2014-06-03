/*
Relay.hpp
John O'Neil
Sunday, June 1st 2014

Refactor of existing C gstreamer pipeline to C++

Relays HTTP served video stream to TCP/IP


*/
#ifndef __RELAY_HPP__
#define __RELAY_HPP__

#include "video_overlay_rpc_server.hpp"
#include <string>

class Relay
{
public:
  Relay(const std::string& name, const std::string& uri);
  ~Relay();

public:
  bool Initialize(void);
  bool Run(void);
  bool Stop(void);

public:
  //callback method invoked when pad added to pipeline
  static void pad_added_handler (GstElement *src, GstPad *pad, Relay *data);

private:
  const std::string m_name;
  const std::string m_uri;

  //Pipeline elements
  GstBus *bus;
  GstElement *pipeline;
  GstElement *demux;
  GstElement *videoconvert;
  GstElement *overlay;
  GstElement *videoconvert2;
  GstElement *h264enc;
  GstElement *tsmux;
  GstElement *tcpsink;
  GstElement *audioconvert;
  GstElement *lamemp3enc;

  //scrolling message
  //TODO: put this into plugin class(es)
  gboolean valid;
  int width;
  int height;
  int current_x_coord;
  guint64 previous_timestamp;
  Queue< std::string > queue;
  VideoOverlayRPCServer* m_rpc_server;
  std::string blurb;// = "This is a test. This is ONLY a test...";

public:
  static void prepare_overlay (GstElement * overlay, GstCaps * caps, gpointer user_data);
  static void draw_overlay (GstElement * overlay, cairo_t * cr, guint64 timestamp, 
   guint64 duration, gpointer user_data);

private:
  Relay();
  Relay(const Relay&);
  Relay operator = (const Relay&);

};


#endif