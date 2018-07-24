/*
 * Copyright (c) 2015-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 *
 */

/**
 * @file
 * <b>NVIDIA GStreamer DeepStream: Metadata Extension</b>
 *
 * @b Description: This file specifies the NVIDIA DeepStream Metadata structures used to
 * describe metadata objects.
 */

/**
 * @defgroup gstreamer_metagroup_api DeepStream Metadata Extension
 *
 * Defines an API for managing GStreamer DeepStream metadata.
 * @ingroup gstreamer_metadata_group
 * @{
 *
 * DeepStream Metadata is attached to a buffer with gst_buffer_add_nvds_meta().
 *
 * Multiple metadatas may be attached by different elements.
 * gst_buffer_get_nvds_meta() gets the last added @ref NvDsMeta.
 * To iterate through each NvDsMeta, following snippet can be used.
 *
 * @code
 *  gpointer state = NULL;
 *  GstMeta *gst_meta;
 *  NvDsMeta *nvdsmeta;
 *  static GQuark _nvdsmeta_quark = 0;
 *
 *  if (!_nvdsmeta_quark)
 *    _nvdsmeta_quark = g_quark_from_static_string(NVDS_META_STRING);
 *
 *  while ((gst_meta = gst_buffer_iterate_meta (buf, &state))) {
 *    if (gst_meta_api_type_has_tag(gst_meta->info->api, _nvdsmeta_quark)) {
 *      nvdsmeta = (NvDsMeta *) gst_meta;
 *      // Do something with this nvdsmeta
 *    }
 *  }
 * @endcode
 *
 *  The meta_data member of the NvDsMeta structure must be cast to a meaningful structure pointer
 *  based on the meta_type. For example, for `meta_type = NVDS_META_FRAME_INFO`,
 *  meta_data must be cast as `(NvDsFrameMeta *)`.
 */

#ifndef GST_NVDS_META_API_H
#define GST_NVDS_META_API_H

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/base/gstbasetransform.h>

#include "nvosd.h"

#ifdef __cplusplus
extern "C"
{
#endif
GType nvds_meta_api_get_type (void);
#define NVDS_META_API_TYPE (nvds_meta_api_get_type())

const GstMetaInfo *nvds_meta_get_info (void);
#define NVDS_META_INFO (nvds_meta_get_info())

#define NVDS_MAX_ATTRIBUTES 16

#define NVDS_META_STRING "nvdsmeta"

/** Defines DeepStream meta data types. */
typedef enum
{
    NVDS_META_INVALID=-1,
    /** Indicates that the meta data contains objects information (NvDsFrameMeta) */
    NVDS_META_FRAME_INFO = 0x01,
} NvDsMetaType;

/**
 * Holds information about one secondary label attribute
 */
typedef struct _NvDsAttr {
  /** Attribute id */
  gshort attr_id;
  /** Attribute value */
  gshort attr_val;
} NvDsAttr;

/** Holds data that secondary classifiers / custom elements update with
 *  secondary label information, such as car type, car color, etc.
 *  The structure may contain a string label. It may also contain a set of
 *  N(num_attrs) pairs of `<attr_id,attr_val>`.
 *
 *  For single label classifiers, `attr_id` will always be 0, and N=1.
 *  For multi-label classifiers, `attr_id` will be the index of the attribute
 *  type (e.g. 0 for Car type, 1 for Car make, ...).
 *  In both cases, `attr_val` will be the value of the attribute
 *  (e.g. 0 for Sedan, 1 for Coupe, ...)
 */
typedef struct _NvDsAttrInfo {
  /** Boolean indicating whether @a attr_label is valid. */
  gboolean is_attr_label;
  /** String label */
  gchar attr_label[64];
  /** Number of valid elements in the @a attrs array. */
  gint num_attrs;
  /** An array of attribute id and value, only @a num_attrs elements are valid. */
  NvDsAttr attrs[64];
} NvDsAttrInfo;

/** Holds parameters that describe meta data for one object. */
typedef struct _NvDsObjectParams {
  /** Structure containing the positional parameters of the object in the frame.
   *  Can also be used to overlay borders / semi-transparent boxes on top of objects
   *  Refer NvOSD_RectParams from nvosd.h
   */
  NvOSD_RectParams rect_params;
  /** Text describing the object can be overlayed using this structure.
   *  @see NvOSD_TextParams from nvosd.h. */
  NvOSD_TextParams text_params;
  /** Index of the object class infered by the primary detector/classifier */
  gint class_id;
  /** Unique ID for tracking the object. This -1 indicates the object has not been
   * tracked. Custom elements adding new NvDsObjectParams should set this to
   * -1. */
  gint tracking_id;
  /** Secondary classifiers / custom elements update this structure with
   *  secondary classification labels. Each element will only update the
   *  attr_info structure at index specified by the element's unique id.
   */
  NvDsAttrInfo attr_info[NVDS_MAX_ATTRIBUTES];
  /** Boolean indicating whether attr_info contains new information. */
  gboolean has_new_info;
  /** Boolean indicating whether secondary classifiers should run inference on the object again. Used internally by components. */
  gboolean force_reinference;
  /** Used internally by components. */
  gint parent_class_id;
  /** Used internally by components. */
  struct _NvDsObjectParams *parent_roi;
  /** Used internally by components */
  NvOSD_LineParams line_params[4];
  /** Used internally by components */
  gint lines_present;
  /** Used internally by components */
  gint obj_status;
} NvDsObjectParams;

/** Holds data that describes metadata objects in the current frame.
    `meta_type` member of @ref NvDsMeta must be set to `NV_BBOX_INFO`. */
typedef struct _NvDsFrameMeta {
  /** Array of NvDsObjectParams structure describing each object. */
  NvDsObjectParams *obj_params;
  /** Number of rectangles/objects i.e. length of @ref NvDsObjectParams */
  guint num_rects;
  /** Number of valid strings in @ref NvDsObjectParams. */
  guint num_strings;
  /** Index of the frame in the batched buffer to which this meta belongs to. */
  guint batch_id;
  /** NvOSD mode to be used. @see NvOSD_Mode in `nvosd.h`. */
  NvOSD_Mode nvosd_mode;
  /** 1 = Primary GIE, 2 = Secondary GIE, 3 = Custom Elements */
  gint gie_type;
  /** Batch size of the primary detector. */
  gint gie_batch_size;
  /** Unique ID of the primary detector that attached this metadata. */
  gint gie_unique_id;
  /** Frame number. */
  gint frame_num;
  /** Index of the stream this params structure belongs to. */
  guint stream_id;
  /** Boolean indicating if these params are no longer valid. */
  gint is_invalid;
} NvDsFrameMeta;


/** Holds DeepSteam meta data. */
 typedef struct _NvDsMeta {
  GstMeta       meta;
  /** Must be cast to another structure based on @a meta_type. */
  gpointer meta_data;
  /** Type of metadata, from the @ref meta_type enum. */
  gint meta_type;
  /** Function called with meta_data pointer as argument when the meta is going to be destroyed.
   * Can be used to clear/free meta_data.
   * Refer to https://developer.gnome.org/glib/unstable/glib-Datasets.html#GDestroyNotify */
  GDestroyNotify destroy;
} NvDsMeta;

/**
 * Adds GstMeta of type @ref NvDsMeta to the GstBuffer and sets the `meta_data` member of @ref NvDsMeta.
 *
 * @param[in] buffer GstBuffer to which the function adds metadata.
 * @param[in] meta_data The pointer to which the function sets the meta_data member of @ref NvDsMeta.
 * @param[in] destroy The GDestroyNotify function to be called when NvDsMeta is to be destroyed.
 *            The function is called with meta_data as a parameter. Refer to
 *            https://developer.gnome.org/glib/unstable/glib-Datasets.html#GDestroyNotify.
 *
 * @return A pointer to the attached @ref NvDsMeta structure; or NULL in case of failure.
 */
NvDsMeta* gst_buffer_add_nvds_meta (GstBuffer *buffer, gpointer meta_data,
        GDestroyNotify destroy);

/** Gets the @ref NvDsMeta last added to the GstBuffer.
 *
 * @param[in] buffer GstBuffer
 *
 * @return A pointer to the last added NvDsMeta structure; or NULL if no NvDsMeta was attached.
 */
NvDsMeta* gst_buffer_get_nvds_meta (GstBuffer *buffer);

/** Creates a deep copy of the input NvDsFrameMeta structure.
 *
 * @param[in] src Pointer to the input NvDsFrameMeta structure to be copied.
 *
 * @return A pointer to the copied NvDsFrameMeta structure.
 */
NvDsFrameMeta* nvds_copy_frame_meta (NvDsFrameMeta *src);
/**
 * Free a heap allocated NvDsFrameMeta structure including members
 * which have been heap allocated.
 *
 * @param[in] params Pointer to the heap allocated NvDsFrameMeta structure
 */
void nvds_free_frame_meta (NvDsFrameMeta *params);
/** @} */
#ifdef __cplusplus
}
#endif
#endif
