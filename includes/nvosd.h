/* Copyright (c) 2016-2018, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


/**
 * @file
 * <b>NVIDIA Multimedia Utilities: On-Screen Display Utility</b>
 *
 * This file defines the NvOSD library to be used to draw rectangles and text over the frame
 * for given parameters.
 */

/**
 * @defgroup nvosd_group On-Screen Display Utility
 * Defines the NvOSD library to be used to draw rectangles and text over the frame.
 * @ingroup gstreamer_utilities_group
 * @{
 */

#ifndef __NVOSD_DEFS__
#define __NVOSD_DEFS__

#ifdef __cplusplus
extern "C"
{
#endif

#define NVOSD_MAX_NUM_RECTS 128

/**
 * Holds the color parameters of the box or text to be overlayed.
 */
typedef struct _NvOSD_ColorParams {
  double red;                 /**< Holds red component of color.
                                   Value must be in the range 0-1. */

  double green;               /**< Holds green component of color.
                                   Value must be in the range 0-1.*/

  double blue;                /**< Holds blue component of color.
                                   Value must be in the range 0-1.*/

  double alpha;               /**< Holds alpha component of color.
                                   Value must be in the range 0-1.*/
} NvOSD_ColorParams;

/**
 * Holds the font parameters of the text to be overlayed.
 */
typedef struct _NvOSD_FontParams {
  const char * font_name;         /**< Holds pointer to the string containing
                                      font name. */

  unsigned int font_size;         /**< Holds size of the font. */

  NvOSD_ColorParams font_color;   /**< Holds font color. */
} NvOSD_FontParams;


/**
 * Holds the text parameters of the text to be overlayed.
 */

typedef struct _NvOSD_TextParams {
  char * display_text; /**< Holds the text to be overlayed. */

  unsigned int x_offset; /**< Holds horizontal offset w.r.t top left pixel of
                             the frame. */
  unsigned int y_offset; /**< Holds vertical offset w.r.t top left pixel of
                             the frame. */

  NvOSD_FontParams font_params;/**< font_params. */

  int set_bg_clr; /**< Boolean to indicate text has background color. */

  NvOSD_ColorParams text_bg_clr; /**< Background color for text. */

} NvOSD_TextParams;

/**
 * Holds the box parameters of the box to be overlayed.
 */
typedef struct _NvOSD_RectParams {
  unsigned int left;   /**< Holds left coordinate of the box in pixels. */

  unsigned int top;    /**< Holds top coordinate of the box in pixels. */

  unsigned int width;  /**< Holds width of the box in pixels. */

  unsigned int height; /**< Holds height of the box in pixels. */

  unsigned int border_width; /**< Holds border_width of the box in pixels. */

  NvOSD_ColorParams border_color; /**< Holds color params of the border
                                      of the box. */

  unsigned int has_bg_color;  /**< Holds boolean value indicating whether box
                                    has background color. */

  unsigned int reserved; /**< Reserved field for future usage.
                             For internal purpose only */

  NvOSD_ColorParams bg_color; /**< Holds background color of the box. */
} NvOSD_RectParams;

/**
 * Holds the box parameters of the line to be overlayed.
 */
typedef struct _NvOSD_LineParams {
  unsigned int x1;   /**< Holds left coordinate of the box in pixels. */

  unsigned int y1;    /**< Holds top coordinate of the box in pixels. */

  unsigned int x2;  /**< Holds width of the box in pixels. */

  unsigned int y2; /**< Holds height of the box in pixels. */

  unsigned int line_width; /**< Holds border_width of the box in pixels. */

  NvOSD_ColorParams line_color; /**< Holds color params of the border
                                        of the box. */
} NvOSD_LineParams;

/**
 * List modes used to overlay boxes and text
 */
typedef enum {
    NV_OSD_MODE_CPU, /**< Selects CPU for OSD processing.
                Works with RGBA data only */
    NV_OSD_MODE_GPU, /**< Selects GPU for OSD processing.
                Works with RGBA data only */
} NvOSD_Mode;

/** @} */

#ifdef __cplusplus
}
#endif

#endif
