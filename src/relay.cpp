#include <gst/gst.h>
#include <cairo.h>
#include <gst/video/video-info.h>
#include <stdio.h>
#include <jsonrpc/rpc.h>
#include <iostream>
using std::cout;
using std::endl;

#include "relay.hpp"

#include "colors.h"


Relay::Relay(const std::string& name, const std::string& uri
  ,const int relay_port, const int json_rpc_port)
  :m_name(name)
  ,m_uri(uri)
  ,m_relay_port(relay_port)
  ,m_jsonrpc_port(json_rpc_port)
  ,m_rpc_server(0)
  ,pipeline(0)
  ,bus(0)
  ,demux(0)
  ,videoconvert(0)
  ,overlay(0)
  ,videoconvert2(0)
  ,h264enc(0)
  ,tsmux(0)
  ,tcpsink(0)
  ,audioconvert(0)
  ,lamemp3enc(0)
  ,valid(false)
  ,width(0)
  ,height(0)
  ,previous_timestamp(0)
  ,m_t1(std::chrono::high_resolution_clock::now())
{
  //TODO: Move to RAII
  m_rpc_server = new VideoOverlayRPCServer(m_jsonrpc_port);
}

Relay::~Relay() {
  if(bus) {
    gst_object_unref(bus);
  }
  if(pipeline) {
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
  }
  if(m_rpc_server)
  {
    delete m_rpc_server;
  }

}

bool Relay::Initialize(void) {

  demux = gst_element_factory_make ("uridecodebin", "demux");
  videoconvert = gst_element_factory_make ("videoconvert", "videoconv");
  overlay = gst_element_factory_make("cairooverlay", "overlay");
  videoconvert2 = gst_element_factory_make("videoconvert", "videoconv2");
  h264enc = gst_element_factory_make ("x264enc", "h264");
  tsmux = gst_element_factory_make("mpegtsmux", "tsmux");
  tcpsink = gst_element_factory_make ("tcpserversink", "sink");
  audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
  lamemp3enc = gst_element_factory_make("lamemp3enc", "lamemp3enc");
   
  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
  
  //TODO: Test all pipeline elements to ensure they're created correctly.
  if (!pipeline) {
    g_printerr ("Not all elements could be created.\n");
    return false;
  }
  
  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), demux, videoconvert, overlay, videoconvert2, h264enc,
      tsmux, tcpsink, audioconvert, lamemp3enc, NULL);
  if (gst_element_link(videoconvert, overlay) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return false;
  }
  if (gst_element_link(overlay, videoconvert2) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref(pipeline);
    return false;
  }
  if (gst_element_link(videoconvert2, h264enc) != TRUE) {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return false;
  }
  if (gst_element_link(h264enc, tsmux) != TRUE) {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return false;
  }
  if (gst_element_link(tsmux, tcpsink) != TRUE) {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref(pipeline);
    return false;
  }
  if (gst_element_link (audioconvert, lamemp3enc) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref(pipeline);
    return false;
  }
  if (gst_element_link(lamemp3enc, tsmux) != TRUE) {
    g_printerr("Elements could not be linked.\n");
    gst_object_unref(pipeline);
    return false;
  }

  g_object_set(demux, "uri", m_uri.c_str(), NULL);
  g_object_set(tcpsink, "port", m_relay_port, NULL);
  
  /* Connect to the pad-added signal */
  g_signal_connect(demux, "pad-added", G_CALLBACK (Relay::pad_added_handler), this);

  /* connect 'on draw' and video image size change handlers to cairo overlay*/
  g_signal_connect(overlay,"draw", G_CALLBACK (Relay::draw_overlay), this);
  g_signal_connect(overlay, "caps-changed",G_CALLBACK (Relay::prepare_overlay), this);

  //start jsonrpc server
  m_rpc_server->Initialize();

  return true;

}
bool Relay::Run(void) {
  GstMessage *msg;
  GstStateChangeReturn ret;
  /* Set the URI to play */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  
  /* Wait until error or EOS */
  bus = gst_element_get_bus(pipeline);
  msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
  
  /* Parse message */
  if (msg != NULL) {
    GError *err;
    gchar *debug_info;
    
    switch (GST_MESSAGE_TYPE (msg)) {
      case GST_MESSAGE_ERROR:
        gst_message_parse_error (msg, &err, &debug_info);
        g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
        g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
        g_clear_error (&err);
        g_free (debug_info);
        break;
      case GST_MESSAGE_EOS:
        g_print ("End-Of-Stream reached.\n");
        break;
      default:
        /* We should not reach here because we only asked for ERRORs and EOS */
        g_printerr ("Unexpected message received.\n");
        break;
    }
    gst_message_unref (msg);
  }
}
bool Relay::Stop(void) {

}

/* Handler for the pad-added signal */
void Relay::pad_added_handler (GstElement *src, GstPad *new_pad, Relay *data) {
  GstPad *sink_pad = NULL;
  GstPadLinkReturn ret;
  GstCaps *new_pad_caps = NULL;
  GstStructure *new_pad_struct = NULL;
  const gchar *new_pad_type = NULL;
  
  g_print (YELLOW "[MSG]" RESET "\tReceived new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

  /* Check the new pad's type */
  new_pad_caps = gst_pad_query_caps(new_pad, 0);
  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
  new_pad_type = gst_structure_get_name (new_pad_struct);

  if(g_str_has_prefix(new_pad_type, "video/x-raw")) {
     sink_pad = gst_element_get_static_pad (data->videoconvert, "sink");
  }else if(g_str_has_prefix(new_pad_type, "audio/x-raw")) {
    sink_pad = gst_element_get_static_pad (data->audioconvert, "sink");
  }else{
    goto exit;
  }
  
  /* If our converter is already linked, we have nothing to do here */
  if (gst_pad_is_linked (sink_pad)) {
    g_print ("  We are already linked. Ignoring.\n");
    goto exit;
  }
  
  /* Attempt the link */
  ret = gst_pad_link (new_pad, sink_pad);
  if (GST_PAD_LINK_FAILED (ret)) {
    g_print ("  Type is '%s' but link failed.\n", new_pad_type);
  } else {
    g_print ( GREEN "[OK]" RESET "\tPipeline of type '%s' is now online.\n", new_pad_type);
  }
  
exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);
  
  /* Unreference the sink pad */
  gst_object_unref (sink_pad);
}


void Relay::prepare_overlay(GstElement * overlay, GstCaps * caps, gpointer user_data)
{
  Relay *s = static_cast<Relay*>(user_data);

  GstVideoInfo info;
  gst_video_info_from_caps(&info, caps);
  s->width = info.width;
  s->height = info.height;
  s->m_rpc_server->Resize(info.width, info.height);
  s->previous_timestamp = -1;
  s->valid = TRUE;
 }


void Relay::draw_overlay(GstElement * overlay, cairo_t * cr, guint64 timestamp, 
   guint64 duration, gpointer user_data)
 {
   Relay *s = static_cast<Relay*>(user_data);
   double scale;

   if (!s->valid)
    return;

  //TODO:In current initial state, the previous_timestamp value is invalid.
  if(s->previous_timestamp<0) {
    s->previous_timestamp = timestamp;
  }

  //time format is in nanoseconds, so we convert to seconds
  const float dt = (timestamp - s->previous_timestamp)/1e9f;
  s->previous_timestamp = timestamp;

  //TODO: abstract these controllers into some generic plugin type.
  s->m_rpc_server->Update(dt);
  s->m_rpc_server->Draw(cr);

}

