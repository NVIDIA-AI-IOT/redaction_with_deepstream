/*
 * Copyright (c) 2018 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

#include <gst/gst.h>
#include <glib.h>

#include "gstnvdsmeta.h"

#define MAX_DISPLAY_LEN 64

#define PGIE_CLASS_ID_VEHICLE 0
#define PGIE_CLASS_ID_PERSON 2

gint frame_number = 0;
// gchar pgie_classes_str[4][32] = { "Vehicle", "TwoWheeler", "Person",
//   "Roadsign"
// };
gchar pgie_classes_str[5][32] = { "license_plate", "make", "model","face","person"};

/* osd_sink_pad_buffer_probe  will extract metadata received on OSD sink pad
 * and update params for drawing rectangle, object information etc. */

static GstPadProbeReturn
osd_sink_pad_buffer_probe (GstPad * pad, GstPadProbeInfo * info,
    gpointer u_data)
{

  GstMeta *gst_meta = NULL;
  NvDsMeta *nvdsmeta = NULL;
  gpointer state = NULL;
  static GQuark _nvdsmeta_quark = 0;
  GstBuffer *buf = (GstBuffer *) info->data;
  NvDsFrameMeta *frame_meta = NULL;
  guint num_rects = 0, rect_index = 0, l_index = 0;
  NvDsObjectParams *obj_meta = NULL;
  guint i = 0;
  NvOSD_TextParams *txt_params = NULL;
  guint vehicle_count = 0;
  guint person_count = 0;

  if (!_nvdsmeta_quark)
    _nvdsmeta_quark = g_quark_from_static_string (NVDS_META_STRING);

  while ((gst_meta = gst_buffer_iterate_meta (buf, &state))) {
    if (gst_meta_api_type_has_tag (gst_meta->info->api, _nvdsmeta_quark)) {

      nvdsmeta = (NvDsMeta *) gst_meta;

      /* We are interested only in intercepting Meta of type
       * "NVDS_META_FRAME_INFO" as they are from our infer elements. */
      if (nvdsmeta->meta_type == NVDS_META_FRAME_INFO) {
        frame_meta = (NvDsFrameMeta *) nvdsmeta->meta_data;
        if (frame_meta == NULL) {
          g_print ("NvDS Meta contained NULL meta \n");
          return GST_PAD_PROBE_OK;
        }

        /* We reset the num_strings here as we plan to iterate through the
         *  the detected objects and form our own strings.
         *  The pipeline generated strings shall be discarded.
         */
        frame_meta->num_strings = 0;

        num_rects = frame_meta->num_rects;

        /* This means we have num_rects in frame_meta->obj_params,
         * now lets iterate through them */

        for (rect_index = 0; rect_index < num_rects; rect_index++) {
          /* Now using above information we need to form a text that should
           * be displayed on top of the bounding box, so lets form it here. */

          obj_meta = (NvDsObjectParams *) & frame_meta->obj_params[rect_index];

          txt_params = &(obj_meta->text_params);
          if (txt_params->display_text)
            g_free (txt_params->display_text);

          txt_params->display_text = g_malloc0 (MAX_DISPLAY_LEN);

          g_snprintf (txt_params->display_text, MAX_DISPLAY_LEN, "%s ",
              pgie_classes_str[obj_meta->class_id]);

          if (obj_meta->class_id == PGIE_CLASS_ID_VEHICLE)
            vehicle_count++;
          if (obj_meta->class_id == PGIE_CLASS_ID_PERSON)
            person_count++;

          /* Now set the offsets where the string should appear */
          txt_params->x_offset = obj_meta->rect_params.left;
          txt_params->y_offset = obj_meta->rect_params.top - 25;

          /* Font , font-color and font-size */
          txt_params->font_params.font_name = "Arial";
          txt_params->font_params.font_size = 10;
          txt_params->font_params.font_color.red = 1.0;
          txt_params->font_params.font_color.green = 1.0;
          txt_params->font_params.font_color.blue = 1.0;
          txt_params->font_params.font_color.alpha = 1.0;

          /* Text background color */
          txt_params->set_bg_clr = 1;
          txt_params->text_bg_clr.red = 0.0;
          txt_params->text_bg_clr.green = 0.0;
          txt_params->text_bg_clr.blue = 0.0;
          txt_params->text_bg_clr.alpha = 1.0;

          frame_meta->num_strings++;
        }
      }
    }
  }
  // g_print ("Frame Number = %d Number of objects = %d "
  //     "Vehicle Count = %d Person Count = %d\n",
  //     frame_number, num_rects, vehicle_count, person_count);
  frame_number++;

  return GST_PAD_PROBE_OK;
}

static gboolean
bus_call (GstBus * bus, GstMessage * msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;
  switch (GST_MESSAGE_TYPE (msg)) {
    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;
    case GST_MESSAGE_ERROR:{
      gchar *debug;
      GError *error;
      gst_message_parse_error (msg, &error, &debug);
      g_printerr ("ERROR from element %s: %s\n",
          GST_OBJECT_NAME (msg->src), error->message);
      g_free (debug);
      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);
      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }
  return TRUE;
}

GstElement *pipeline, *video, *sinkbin;

static void
cb_new_pad (GstElement *element,GstPad *pad, gpointer data)
{
  gchar* name;
  name = gst_pad_get_name (pad);
  g_print ("A new pad %s was created\n", name);
  GstPad* sinkpad;
  GstElement* h264parser = (GstElement *) data;
  sinkpad = gst_element_get_static_pad (h264parser, "sink");
  gst_pad_link (pad, sinkpad);
  gst_object_unref (sinkpad);
  g_free (name);
}

static void
cb_newpad (GstElement *decodebin, GstPad *pad, gpointer data)
  {
  GstCaps *caps;
  GstStructure *str;
  GstPad *videopad;
  /*only link once*/
  videopad = gst_element_get_static_pad (video, "sink");
  if (GST_PAD_IS_LINKED (videopad)) {
    g_object_unref (videopad);
    return;
  }
  /* check media type */
  caps = gst_pad_query_caps (pad, NULL);
  str = gst_caps_get_structure (caps, 0);
  if (!g_strrstr (gst_structure_get_name (str), "video")){
    gst_caps_unref (caps);
    gst_object_unref (videopad);
    return;
  }
  gst_caps_unref (caps);
  /* link’n’play */
  gst_pad_link (pad, videopad);
  g_object_unref (videopad);
  GST_DEBUG_BIN_TO_DOT_FILE (GST_BIN (pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "ds-app-playing");
}

int
main (int argc, char *argv[])
{
  GMainLoop *loop = NULL;
  GstElement *source = NULL, *h264parser =
      NULL, *demux = NULL, *pgie = NULL, *encoder = NULL, *mux = NULL, *videoconv = NULL,
      *decoder = NULL, *sink = NULL, *nvvidconv = NULL, *nvvidconv1 = NULL, 
      *nvosd = NULL, *filter1 = NULL, *filter2 = NULL, *filter3 = NULL;
  GstBus *bus = NULL;
  guint bus_watch_id;
  GstCaps *caps1 = NULL, *caps2 = NULL, *caps3 = NULL;
  gulong osd_probe_id = 0;
  GstPad *osd_sink_pad = NULL, *videopad = NULL;

  /* Check input arguments */
  // if (argc != 3) {
  //   g_printerr ("Usage: %s <video filename> <pgie config filename>\n", argv[0]);
  //   return -1;
  // }

  /* Standard GStreamer initialization */
  gst_init (&argc, &argv);
  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  /* Create Pipeline element that will form a connection of other elements */
  pipeline = gst_pipeline_new ("ds-redaction-pipeline");

  /* Source element for reading from the file */
  source = gst_element_factory_make ("filesrc", "file-source");
  // source = gst_element_factory_make ("uridecodebin", "file-source");
  decoder = gst_element_factory_make ("decodebin", "decoder");
  g_signal_connect (decoder, "pad-added", G_CALLBACK (cb_newpad), NULL);
  gst_bin_add_many (GST_BIN (pipeline), source, decoder, NULL);
  gst_element_link (source, decoder);


  demux = gst_element_factory_make ("qtdemux", "demuxer");
  /* Since the data format in the input file is elementary h264 stream,
   * we need a h264parser */
  h264parser = gst_element_factory_make ("h264parse", "h264-parser");

  /* Use nvdec_h264 for hardware accelerated decode on GPU */
  // decoder = gst_element_factory_make ("nvdec_h264", "nvh264-decoder");

  /* Use nvinfer to run inferencing on decoder's output,
   * behaviour of inferencing is set through config file */
  video = gst_bin_new ("video-process-bin");
  pgie = gst_element_factory_make ("nvinfer", "primary-nvinference-engine");
  videopad = gst_element_get_static_pad (pgie, "sink");
  /* Use convertor to convert from NV12 to RGBA as required by nvosd */
  nvvidconv = gst_element_factory_make ("nvvidconv", "nvvideo-converter");

  /* Create OSD to draw on the converted RGBA buffer */
  nvosd = gst_element_factory_make ("nvosd", "nv-onscreendisplay");


  nvvidconv1 = gst_element_factory_make ("nvvidconv", "nvvideo-converter1");

  videoconv = gst_element_factory_make ("videoconvert", "video-converter");

  encoder = gst_element_factory_make ("avenc_mpeg4", "mp4-encoder");

  // g_object_set (G_OBJECT (encoder), "bitrate", 2000000, NULL);

  mux = gst_element_factory_make ("qtmux", "muxer");
  /* Finally render the osd output */
  sink = gst_element_factory_make ("nveglglessink", "nvvideo-renderer");
  // sink = gst_element_factory_make ("fakesink", "nvvideo-renderer");
  // sink = gst_element_factory_make ("filesink", "nvvideo-renderer");
  // g_object_set (G_OBJECT (sink), "location", "/home/shuo/out.mp4","sync", FALSE, "async", FALSE, NULL);

  /* caps filter for nvvidconv to convert NV12 to RGBA as nvosd expects input
   * in RGBA format */
  filter1 = gst_element_factory_make ("capsfilter", "filter1");
  filter2 = gst_element_factory_make ("capsfilter", "filter2");
  filter3 = gst_element_factory_make ("capsfilter", "filter3");
  if (!pipeline || !source || !h264parser || !decoder || !pgie
      || !filter1 || !nvvidconv || !filter2 || !nvosd || !sink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* we set the input filename to the source element */
  // g_object_set (G_OBJECT (source), "location", argv[1], NULL);
  g_object_set (G_OBJECT (source), "location", "sample_videos/1.mp4", NULL);
  // g_object_set (G_OBJECT (source), "location", "/home/shuo/DeepStream_Release/samples/streams/sample_720p.mp4", NULL);

  /* Set all the necessary properties of the nvinfer element,
   * the necessary ones are : */
  // g_object_set (G_OBJECT (pgie),"config-file-path", argv[2], NULL);
  g_object_set (G_OBJECT (pgie),"config-file-path", "configs/pgie_config_fd_lpd.txt", NULL);

  /* we set the osd properties here */
  g_object_set (G_OBJECT (nvosd), "font-size", 15, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* Set up the pipeline */
  /* we add all elements into the pipeline */
  // gst_bin_add_many (GST_BIN (video), pgie, filter1, nvvidconv, filter2, nvosd, nvvidconv1, filter3, videoconv, encoder, mux, sink, NULL);
  gst_bin_add_many (GST_BIN (video), pgie, filter1, nvvidconv, filter2, nvosd, sink, NULL);

  caps1 = gst_caps_from_string ("video/x-raw(memory:NVMM), format=NV12");
  g_object_set (G_OBJECT (filter1), "caps", caps1, NULL);
  gst_caps_unref (caps1);
  caps2 = gst_caps_from_string ("video/x-raw(memory:NVMM), format=RGBA");
  g_object_set (G_OBJECT (filter2), "caps", caps2, NULL);
  gst_caps_unref (caps2);
  caps3 = gst_caps_new_simple ("video/x-raw", "format", G_TYPE_STRING, "RGBA", NULL);
  g_object_set (G_OBJECT (filter3), "caps", caps3, NULL);
  gst_caps_unref (caps3);

  /* we link the elements together */
  /* file-source -> h264-parser -> nvh264-decoder ->
   * nvinfer -> filter1 -> nvvidconv -> filter2 -> nvosd -> video-renderer */
  // gst_element_link_many (pgie, filter1, nvvidconv, filter2, nvosd, nvvidconv1, filter3, videoconv, encoder, mux, sink, NULL);
  gst_element_link_many (pgie, filter1, nvvidconv, filter2, nvosd, sink, NULL);

  gst_element_add_pad (video, gst_ghost_pad_new ("sink", videopad));
  gst_object_unref (videopad);
  gst_bin_add (GST_BIN (pipeline), video);
  // gst_bin_add_many (GST_BIN (pipeline), source, demux,h264parser, NULL);
  // gst_element_link_pads (source, "src", demux, "sink");
  // gst_element_link_pads (demux, "src", h264parser, "sink");
  // g_signal_connect (demux, "pad-added", G_CALLBACK (cb_new_pad), NULL);


  /* Lets add probe to get informed of the meta data generated, we add probe to
   * the sink pad of the osd element, since by that time, the buffer would have
   * had got all the metadata. */
  // osd_sink_pad = gst_element_get_static_pad (nvosd, "sink");
  // if (!osd_sink_pad)
  //   g_print ("Unable to get sink pad\n");
  // else
  //   osd_probe_id = gst_pad_add_probe (osd_sink_pad, GST_PAD_PROBE_TYPE_BUFFER,
  //       osd_sink_pad_buffer_probe, NULL, NULL);

  /* Set the pipeline to "playing" state */
  g_print ("Now playing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* Wait till pipeline encounters an error or EOS */
  g_print ("Running...\n");
  // GST_DEBUG_BIN_TO_DOT_FILE (GST_BIN (pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "ds-app-playing");
  GST_DEBUG_BIN_TO_DOT_FILE (GST_BIN (pipeline), GST_DEBUG_GRAPH_SHOW_ALL, "ds-app-playing-loop");
  g_main_loop_run (loop);

  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);
  return 0;


}
