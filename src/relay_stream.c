#include <gst/gst.h>
/*
	Okay since:
	1. Gstreamer 0.1 doesn't handle .nsv contaned streams well
	2. Gstreamer 1.0 python bindings don't support Cairo yet
	
	I've got no choice but to reimpliment some prevous basic work
	in python via the C api.

	This is all probably going to take more time than I want to
	put into this, so it ain't going far.

	Now: This module relays an existing stream to
	a local TCP server, transcoding to TS(h264/mp3)

*/

/* Structure to contain all our information, so we can pass it to callbacks */
typedef struct _CustomData {
  GstElement *demux;
  GstElement *videoconvert;
  GstElement *h264enc;
  GstElement *tsmux;
	GstElement *tcpsink;
	GstElement *audioconvert;
	GstElement *lamemp3enc;
} CustomData;

/* Handler for the pad-added signal */
static void pad_added_handler (GstElement *src, GstPad *pad, CustomData *data);
  
int main(int argc, char *argv[]) {
  GstElement *pipeline, *source, *sink;
  GstBus *bus;
  GstMessage *msg;
  GstStateChangeReturn ret;

	CustomData data;

	//draw url to relay out of our command line args
	if(argc<2) {
		g_print ("Please provide a valid URL to relay.\n");
		return -1;
	}
	
  
  /* Initialize GStreamer */
  gst_init (&argc, &argv);
   
  /* Create the elements */
  //source = gst_element_factory_make ("videotestsrc", "source");
  //sink = gst_element_factory_make ("autovideosink", "sink");

	data.demux = gst_element_factory_make ("uridecodebin", "demux");
	data.videoconvert = gst_element_factory_make ("videoconvert", "videoconv");
	data.h264enc = gst_element_factory_make ("x264enc", "h264");
	data.tsmux = gst_element_factory_make("mpegtsmux", "tsmux");
  data.tcpsink = gst_element_factory_make ("tcpserversink", "sink");
	data.audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
	data.lamemp3enc = gst_element_factory_make("lamemp3enc", "lamemp3enc");
   
  /* Create the empty pipeline */
  pipeline = gst_pipeline_new ("test-pipeline");
   
  if (!pipeline || !source || !sink) {
    g_printerr ("Not all elements could be created.\n");
    return -1;
  }
  
  /* Build the pipeline */
  gst_bin_add_many (GST_BIN (pipeline), data.demux, data.videoconvert, data.h264enc, data.tsmux, data.tcpsink, data.audioconvert, data.lamemp3enc, NULL);
  if (gst_element_link (data.videoconvert, data.h264enc) != TRUE) {
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
  
  /* Modify the source's properties */
 // g_object_set (source, "pattern", 0, NULL);
  
  /* Start playing */
  ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_printerr ("Unable to set the pipeline to the playing state.\n");
    gst_object_unref (pipeline);
    return -1;
  }
  
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
  
  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (new_pad), GST_ELEMENT_NAME (src));

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
	  g_print ("  Link succeeded (type '%s').\n", new_pad_type);
	}
  
exit:
  /* Unreference the new pad's caps, if we got them */
  if (new_pad_caps != NULL)
    gst_caps_unref (new_pad_caps);
  
  /* Unreference the sink pad */
  gst_object_unref (sink_pad);
}
