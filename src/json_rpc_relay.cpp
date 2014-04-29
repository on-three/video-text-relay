/*
  name: relay_stream_with_overlay.c
  author: John O'Neil (somewhat. it's all cut and paste)
  date: Saturday, April 27th 2014.
  desc:
    I've done all this before in python, but as Gstreamer 1.0 python support is
  still _very_ buggy (no change from 0.1 support) i'm taking a step back to
  re-implement in straight C and/or C++.

  This module can relay an HTML stream to a client via TCP. In other words, we
  provide 1 command line parameter (URL of the stream source) and this program
  will connect to that HTML stream and transcode it.
  We can then connect clients up to this program via TCP (only) on port 10000.
  I haven't yet made the port number configurable.

  Additionally, this program just overlays some Cairo graphics on the stream
  while we have access to the raw video buffers (before we re-encode the video).

  Don't know quite where this is going, but this appears to work for both
  .nsv video and .ts video streams.
  
  Output of this relay is always a TS(H264/MP3).

  Building:
  In LinuxMint 16, i believe the following packages are required to build.
  There could be some missing:
  libgstreamer1.0-dev
  libgstreamer-plugins-base1.0-dev
  libcairo2-dev
  libglib2.0-dev
*/

#include <gst/gst.h>
#include <cairo.h>
#include <gst/video/video-info.h>
#include <stdio.h>
#include <jsonrpc/rpc.h>
#include <iostream>

#include "colors.h"
#include "abstractrpcstubserver.h"
#include "threadsafe_queue.hpp"

using namespace jsonrpc;
using namespace std;

class MyStubServer : public AbstractRPCStubServer
{
    public:
        MyStubServer(Queue<std::string>* queue);

        std::string ShowMessage(const std::string& friendlyName, const std::string& msg, const int& x, const int& y);
	private:
		Queue<std::string>* m_queue;
};

MyStubServer::MyStubServer(Queue<std::string>* queue)
	:AbstractRPCStubServer(new HttpServer(8080))
	,m_queue(queue)
{
}

std::string MyStubServer::ShowMessage(const std::string& friendlyName, 
	const std::string& msg, const int& x, const int& y)
{
	cout<<"Showing message\'"<<msg<<"\' at "<<x<<","<<y<<" named "<<friendlyName<<endl;
	m_queue->push(msg);
	return friendlyName;
}

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GstElement *demux;
  GstElement *videoconvert;
  GstElement *overlay;
  GstElement *videoconvert2;
  GstElement *h264enc;
  GstElement *tsmux;
	GstElement *tcpsink;
	GstElement *audioconvert;
	GstElement *lamemp3enc;
} CustomData;

typedef struct {
  gboolean valid;
  int width;
  int height;
  int current_x_coord;
  guint64 previous_timestamp;
	Queue< std::string >* queue;
	std::string blurb = "This is a test. This is ONLY a test...";
  
} CairoOverlayState;

/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);

/******************************************************************************
function: prepare_overlay
desc: Callback that is invoked when video buffers change dimensions.
We wait until this arrives before drawing in the raw video buffer.
******************************************************************************/
static void prepare_overlay (GstElement * overlay, GstCaps * caps, gpointer user_data)
{
  CairoOverlayState *state = (CairoOverlayState *)user_data;

   /*gst_video_format_parse_caps (caps, NULL, &state->width, &state->height);*/
  GstVideoInfo info;
  gst_video_info_from_caps(&info, caps);
  state->width = info.width;
  state->height = info.height;
  state->current_x_coord = info.width;
  //TODO: Set this to actual current timestamp value(if possible)
  state->previous_timestamp = -1;
  state->valid = TRUE;
 }

/******************************************************************************
function: draw_overlay
desc: Primary callback to draw Cairo elements onto the raw video buffer.
******************************************************************************/
static void draw_overlay (GstElement * overlay, cairo_t * cr, guint64 timestamp, 
   guint64 duration, gpointer user_data)
 {
   CairoOverlayState *s = (CairoOverlayState *)user_data;
   double scale;

   if (!s->valid)
    return;

  //TODO:In current initial state, the previous_timestamp value is invalid.
  if(s->previous_timestamp<0) {
    s->previous_timestamp = timestamp;
  }

	//Are there commands to take out of our RPC queue?
	if(!s->queue->empty())
	{
		s->blurb = s->queue->front();
		s->queue->pop();
	}
	
  cairo_text_extents_t te;
  cairo_set_source_rgb (cr, 1.0, 1.0, 0.0);
  cairo_select_font_face (cr, "Georgia", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size (cr, 35.0);
  cairo_text_extents (cr, s->blurb.c_str(), &te);
  double dt = ((timestamp-s->previous_timestamp)/(double)1e9);//time format is in nanoseconds, so we convert to seconds
  //double dt = ((duration)/(double)1e9);//time format is in nanoseconds, so we convert to seconds
  s->previous_timestamp = timestamp;
  //printf("elapsed time %f", elapsed_time);
  s->current_x_coord -= ((s->width+te.width)/12.0)*dt;//full scroll in 12 seconds.
  if(s->current_x_coord<(-1.0*te.width))//wraparound.
  {
    s->current_x_coord = s->width;
  }
  //cairo_move_to (cr, 0.5 - te.width / 2 - te.x_bearing, 0.5 - te.height / 2 - te.y_bearing);
  cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
  cairo_move_to(cr, s->current_x_coord+3, (2*s->height/3)+3);
  cairo_show_text (cr, s->blurb.c_str());
  cairo_set_source_rgb (cr, 1.0, 1.0, 0.0);
  cairo_move_to(cr, s->current_x_coord, 2*s->height/3);
  cairo_show_text (cr, s->blurb.c_str());
}

//
// Dummy task function for RPC TX
//
void RPCRXFunc(void *data) {
	//TODO: sleep?
	cout<<"."<<endl;
}

/******************************************************************************
function: main()
currently takes one argument: url of stream to relay
Can connect to this realy via TCP on port 1000
(e.g) tcp://127.0.0.1:10000
******************************************************************************/
int main(int argc, char *argv[]) {
  GstElement *pipeline, *source, *sink;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;
	GstTask * rpc_task = 0;

	CairoOverlayState overlay_state;
	Queue< std::string > queue;
	overlay_state.queue = &queue;
	MyStubServer s(&queue);

	CustomData data;

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
	
  
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
   
  /* Create the elements */
  //source = gst_element_factory_make ("videotestsrc", "source");
  //sink = gst_element_factory_make ("autovideosink", "sink");

	data.demux = gst_element_factory_make ("uridecodebin", "demux");
	data.videoconvert = gst_element_factory_make ("videoconvert", "videoconv");
  data.overlay = gst_element_factory_make("cairooverlay", "overlay");
  data.videoconvert2 = gst_element_factory_make("videoconvert", "videoconv2");
	data.h264enc = gst_element_factory_make ("x264enc", "h264");
	data.tsmux = gst_element_factory_make("mpegtsmux", "tsmux");
  data.tcpsink = gst_element_factory_make ("tcpserversink", "sink");
	data.audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
	data.lamemp3enc = gst_element_factory_make("lamemp3enc", "lamemp3enc");
   
  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
  
  //TODO: Test all pipeline elements to ensure they're created correctly.
  if (!pipeline) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }
  
  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), data.demux, data.videoconvert, data.overlay, data.videoconvert2, data.h264enc, data.tsmux, data.tcpsink, data.audioconvert, data.lamemp3enc, NULL);
  if (gst_element_link (data.videoconvert, data.overlay) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (data.overlay, data.videoconvert2) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (data.videoconvert2, data.h264enc) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (data.h264enc, data.tsmux) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  if (gst_element_link (data.tsmux, data.tcpsink) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
	if (gst_element_link (data.audioconvert, data.lamemp3enc) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }
	if (gst_element_link(data.lamemp3enc, data.tsmux) != TRUE) {
    g_printerr ("Elements could not be linked.\n");
    gst_object_unref (pipeline);
    return -1;
  }

	  /* Set the URI to play */
  g_object_set (data.demux, "uri", argv[1], NULL);
	g_object_set (data.tcpsink, "port", 10000, NULL);
  
  /* Connect to the pad-added signal */
  g_signal_connect (data.demux, "pad-added", G_CALLBACK (pad_added_handler), &data);

  /* connect 'on draw' and video image size change handlers to cairo overlay*/
  g_signal_connect(data.overlay,"draw", G_CALLBACK (draw_overlay), &overlay_state);
  g_signal_connect(data.overlay, "caps-changed",G_CALLBACK (prepare_overlay), &overlay_state);
 /* 
  rpc_task = gst_task_create(RPCRXFunc, &s);
	if(!rpc_task)
	{
    g_printerr ("Unable to create json rpc RX task.\n");
    gst_object_unref (pipeline);
    return -1;
	}
	GStaticRecMutex *rpc_mutex = 0;
     
	rpc_mutex = g_new (GStaticRecMutex, 1);
	if (task_mutex && input_task)
	{
		g_static_rec_mutex_init(rpc_mutex);
		gst_task_set_lock (rpc_task, rpc_mutex);
		gst_task_start (rpc_task);
		}

	gst_task_set_lock(rpc_task, rpc_lock);
	gst_task_start(rpc_task);
*/  
  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }

	s.StartListening();
  
  /* Wait until error or EOS */
  bus = gst_element_get_bus (pipeline);
  msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
  
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
  
	s.StopListening();
	//gst_task_join(rpc_task);
  
  /* Free resources */
  gst_object_unref (bus);
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);
  return 0;
}

/* This function will be called by the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data) {
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

