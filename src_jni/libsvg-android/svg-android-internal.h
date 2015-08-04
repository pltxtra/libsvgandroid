/* libsvg-android - Render SVG documents to an Android canvas
 *
 * Copyright © 2002 University of Southern California
 * Copyright © 2010 Anton Persson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy (COPYING.LESSER) of the
 * GNU Lesser General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Original Cairo-version:
 * Author: Carl D. Worth <cworth@isi.edu>
 *
 * Android modification:
 * Author: Anton Persson {don d0t juanton 4t gmail d0t com}
 */

#ifndef SVG_ANDROID_INTERNAL_H
#define SVG_ANDROID_INTERNAL_H

#include "svg-android.h"

#include <jni.h>

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* XXX: What should this actually be? */
#define SVG_ANDROID_FONT_FAMILY_DEFAULT "verdana"

extern svg_render_engine_t SVG_ANDROID_RENDER_ENGINE;

typedef struct svg_android_pt {
	double x;
	double y;
} svg_android_pt_t;

typedef enum svg_android_render_type {
	SVG_ANDROID_RENDER_TYPE_FILL,
	SVG_ANDROID_RENDER_TYPE_STROKE
} svg_android_render_type_t;

typedef struct svg_android_state {	
	svg_android_t *instance;

	// we need to store the last path point, since we cant get it from an android path...
	// this we need for the arc_to implementation...
	double last_x, last_y;
	
	jobject offscreen_bitmap;
	
	svg_bounding_box_t bounding_box;

	jobject saved_canvas; // temporary canvas

	jobject path;       // this can point to either the state_path or a cached object
	jobject state_path; // this is always the same object for each svg_android_state instance
	jobject paint;
	jobject matrix;
	
	svg_color_t color;

	/* save state data for "copy_android_state" function */
	svg_fill_rule_t fill_rule;
	svg_length_t width_len;
	svg_stroke_line_cap_t line_cap;
	svg_stroke_line_join_t line_join;
	double miter_limit;
	
	svg_paint_t fill_paint;
	svg_paint_t stroke_paint;
	double fill_opacity;
	double stroke_opacity;

	char *font_family;
	double font_size;
	svg_font_style_t font_style;
	unsigned int font_weight;
	int font_dirty;

	double *dash;
	int num_dashes;
	double dash_offset;

	double opacity;

	double viewport_width;
	double viewport_height;

	int bbox;

	svg_text_anchor_t text_anchor;

	struct svg_android_state *next;
} svg_android_state_t;

struct svg_android {
	svg_t *svg;

	svg_android_state_t *state;
	
	unsigned int viewport_width;
	unsigned int viewport_height;

	jboolean do_antialias; // set to !0 for "true", 0 for "false"
	
	unsigned int fit_to_area; // set to !0 for "true", 0 for "false"
	unsigned int fit_to_x, fit_to_y, fit_to_w, fit_to_h;
	double fit_to_scale;
	jobject fit_to_MATRIX;
	
//	cairo_t cr;
	JNIEnv *env;
	
	jobject canvas; // android canvas reference
	
	jclass canvas_clazz; // android Canvas class
	jclass raster_clazz; // android SvgRaster class
	jclass bitmap_clazz; // android Bitmap class
	jclass matrix_clazz; // android matrix class
	jclass shader_clazz; // android shader class
	jclass path_clazz;   // android path class	
	jclass paint_clazz; // android paint class
	jclass dashPathEffect_clazz; // android paint dash effect class
	
	/* android canvas method references */
	jmethodID canvas_constructor;
	jmethodID canvas_save;
	jmethodID canvas_restore;
	jmethodID canvas_clip_rect;
	jmethodID canvas_concat;
	jmethodID canvas_draw_bitmap;
	jmethodID canvas_draw_bitmap2;
	jmethodID canvas_draw_path;
	jmethodID canvas_draw_text;
	jmethodID canvas_drawRGB;
	jmethodID canvas_getWidth;
	jmethodID canvas_getHeight;
	
	/* android razter method references */
	jmethodID raster_setTypeface;
	jmethodID raster_getBounds;
	jmethodID raster_matrixCreate;
	jmethodID raster_matrixInit;
	jmethodID raster_createBitmap;
	jmethodID raster_data2bitmap;
	jmethodID raster_setFillRule;
	jmethodID raster_setPaintStyle;
	jmethodID raster_setStrokeCap;
	jmethodID raster_setStrokeJoin;
	jmethodID raster_createBitmapShader;
	jmethodID raster_createLinearGradient;
	jmethodID raster_createRadialGradient;
	jmethodID raster_matrixInvert;
	jmethodID raster_getBoundingBox;
	jmethodID raster_drawEllipse;
	jmethodID raster_drawRect;
	jmethodID raster_debugMatrix;
	
	/* android bitmap method references */
	jmethodID bitmap_erase_color;
	
	/* android matrix method references */
	jmethodID matrix_constructor;
	jmethodID matrix_postTranslate;
	jmethodID matrix_postScale;
	jmethodID matrix_postConcat;
	jmethodID matrix_reset;
	jmethodID matrix_set;

	/* android shader method references */
	jmethodID shader_setLocalMatrix;
	
	/* android path method references */
	jmethodID path_constructor;
	jmethodID path_clone_constructor;
	jmethodID path_transform;
	jmethodID path_moveTo;
 	jmethodID path_lineTo;
 	jmethodID path_cubicTo;
 	jmethodID path_quadTo;
 	jmethodID path_close;
	jmethodID path_reset;

	/* android paint method references */
	jmethodID paint_constructor;
	jmethodID paint_setPathEffect;
	jmethodID paint_setARGB;
	jmethodID paint_setShader;
	jmethodID paint_setStrokeMiter;
	jmethodID paint_setStrokeWidth;
	jmethodID paint_getTextPath;
	jmethodID paint_setAntialias;
	jmethodID paint_set;
	jmethodID paint_reset;

	/* DashPathEffect method/constructors refs */
	jmethodID dashPathEffect_constructor;
};

#define ANDROID_CANVAS_CREATE(a,B) \
	(*(a->env))->NewObject(a->env, a->canvas_clazz,a->canvas_constructor,B)
#define ANDROID_SAVE(a) \
	(*(a->env))->CallIntMethod(a->env, a->canvas, a->canvas_save)
#define ANDROID_RESTORE(a) \
	(*(a->env))->CallVoidMethod(a->env, a->canvas, a->canvas_restore)
#define ANDROID_CANVAS_CLIP_RECT(a,l,t,r,b)				\
	(*(a->env))->CallBooleanMethod(a->env, a->canvas, a->canvas_clip_rect, l, t, r, b)
#define ANDROID_CANVAS_CONCAT_MATRIX(a,m) \
	(*(a->env))->CallVoidMethod(a->env, a->canvas, a->canvas_concat, m)
#define ANDROID_DRAW_BITMAP(a,b,m) \
	(*(a->env))->CallVoidMethod(a->env, a->canvas, a->canvas_draw_bitmap, b, m, NULL)
#define ANDROID_DRAW_BITMAP2(a,b,x,y)					\
	(*(a->env))->CallVoidMethod(a->env, a->canvas, a->canvas_draw_bitmap2, b, x, y, NULL)
#define ANDROID_DRAW_PATH(a,p,P) \
	(*(a->env))->CallVoidMethod(a->env, a->canvas, a->canvas_draw_path, p, P)
#define ANDROID_DRAW_TEXT(e,T,X,Y) \
	(*(e->env))->CallVoidMethod(e->env, e->canvas, e->canvas_draw_text, (*(e->env))->NewStringUTF(e->env, T), X, Y, e->state->paint)
#define ANDROID_DRAW_RGB(a,R,G,B)					\
	(*(a->env))->CallVoidMethod(a->env, a->canvas, a->canvas_drawRGB, R, G, B)
#define ANDROID_GET_WIDTH(a)					\
	(*(a->env))->CallIntMethod(a->env, a->canvas, a->canvas_getWidth)
#define ANDROID_GET_HEIGHT(a)					\
	(*(a->env))->CallIntMethod(a->env, a->canvas, a->canvas_getHeight)

#define ANDROID_SET_TYPEFACE(e,F,W,S,A)				\
	(*(e->env))->CallStaticVoidMethod(e->env, e->raster_clazz, e->raster_setTypeface, e->state->paint, (*(e->env))->NewStringUTF(e->env, F), W, S, A)
#define ANDROID_PATH_GET_BOUNDS(e,P) \
	(*(e->env))->CallStaticObjectMethod(e->env, e->raster_clazz, e->raster_getBounds, P)
#define ANDROID_MATRIX_CREATE(e,A,B,C,D,E,F) \
	(*(e->env))->CallStaticObjectMethod(e->env, e->raster_clazz, e->raster_matrixCreate, A, B, C, D, E, F)
#define ANDROID_MATRIX_INIT(e,m,A,B,C,D,E,F)				\
	(*(e->env))->CallStaticVoidMethod(e->env, e->raster_clazz, e->raster_matrixInit, m, A, B, C, D, E, F)
#define ANDROID_CREATE_BITMAP(a,w,h) \
	(*(a->env))->CallStaticObjectMethod(a->env, a->raster_clazz, a->raster_createBitmap, w, h)
#define ANDROID_DATA_2_BITMAP(a,d,w,h)					\
	(*(a->env))->CallStaticObjectMethod(a->env, a->raster_clazz, a->raster_data2bitmap, w, h, d)

 // e == jbool true ? EVEN_ODD : WINDING
#define ANDROID_SET_FILL_TYPE(a,p,e) \
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_setFillRule, p, e)

 // s = {TRUE = STROKE, FALSE = FILL}
#define ANDROID_SET_PAINT_STYLE(a,p,s) \
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_setPaintStyle,p,s)

 // c[3] = {0 = BUTT, 1 = ROUND, 2/* = SQUARE}
#define ANDROID_SET_STROKE_CAP(a,p,c) \
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_setStrokeCap,p,c)

 // c[3] = {0 = MITER, 1 = ROUND, 2/* = BEVEL}
#define ANDROID_SET_STROKE_JOIN(a,p,c) \
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_setStrokeJoin,p,c)
#define ANDROID_CREATE_BITMAP_SHADER(a,B) \
	(*(a->env))->CallStaticObjectMethod(a->env, a->raster_clazz, a->raster_createBitmapShader,B)
#define ANDROID_CREATE_LINEAR_GRADIENT(a,L,T,R,B,C,O,S) \
	(*(a->env))->CallStaticObjectMethod(a->env, a->raster_clazz, a->raster_createLinearGradient,L,T,R,B,C,O,S)
#define ANDROID_CREATE_RADIAL_GRADIENT(a,X,Y,R,C,O,S) \
	(*(a->env))->CallStaticObjectMethod(a->env, a->raster_clazz, a->raster_createRadialGradient,X,Y,R,C,O,S)
#define ANDROID_MATRIX_INVERT(a,m) \
	(*(a->env))->CallStaticObjectMethod(a->env, a->raster_clazz, a->raster_matrixInvert, m)
#define ANDROID_GET_PATH_BOUNDING_BOX(a,P)				\
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_getBoundingBox, P, a->canvas)
#define ANDROID_DRAW_ELLIPSE(a,A,B,C,D)				\
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_drawEllipse, a->canvas, a->state->paint, A, B, C, D)
#define ANDROID_DRAW_RECT(a,X,Y,W,H,RX,RY)				\
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_drawRect, a->canvas, a->state->paint, X, Y, W, H, RX, RY)
#define ANDROID_DEBUG_MATRIX(a,A)				\
	(*(a->env))->CallStaticVoidMethod(a->env, a->raster_clazz, a->raster_debugMatrix, A)
	
#define ANDROID_FILL_BITMAP(a,b,c) \
	(*(a->env))->CallVoidMethod(a->env, b, a->bitmap_erase_color, c)

#define ANDROID_IDENTITY_MATRIX(a) \
	(*(a->env))->NewObject(a->env, a->matrix_clazz,a->matrix_constructor)
#define ANDROID_MATRIX_TRANSLATE(a,m,x,y)				\
	(*(a->env))->CallBooleanMethod(a->env, m,a->matrix_postTranslate,x,y)
#define ANDROID_MATRIX_SCALE(a,m,x,y)			\
	(*(a->env))->CallBooleanMethod(a->env, m,a->matrix_postScale,x,y)
#define ANDROID_MATRIX_MULTIPLY(a,m,M)			\
	(*(a->env))->CallBooleanMethod(a->env, m,a->matrix_postConcat,M)
#define ANDROID_MATRIX_RESET(a,m)			\
	(*(a->env))->CallVoidMethod(a->env, m,a->matrix_reset)
#define ANDROID_MATRIX_SET(a,m,M)			\
	(*(a->env))->CallVoidMethod(a->env, m,a->matrix_set,M)

#define ANDROID_SHADER_SET_MATRIX(a,s,m) \
	(*(a->env))->CallVoidMethod(a->env, s, a->shader_setLocalMatrix, m) 
	
#define ANDROID_PATH_CREATE(a) \
	(*(a->env))->NewObject(a->env, a->path_clazz, a->path_constructor)
#define ANDROID_PATH_CLONE(a, b)						\
	(*(a->env))->NewObject(a->env, a->path_clazz, a->path_clone_constructor, b)
#define ANDROID_PATH_TRANSFORM(a,m)					\
	(*(a->env))->CallVoidMethod(a->env, a->state->path, a->path_transform, m)
#define ANDROID_PATH_MOVE_TO(a,x,y) \
	(*(a->env))->CallVoidMethod(a->env, a->state->path, a->path_moveTo, x, y)
#define ANDROID_PATH_LINE_TO(a,x,y) \
	(*(a->env))->CallVoidMethod(a->env, a->state->path, a->path_lineTo, x, y)
#define ANDROID_PATH_CURVE_TO(a,x1,y1,x2,y2,x3,y3) \
	(*(a->env))->CallVoidMethod(a->env, a->state->path, a->path_cubicTo, x1, y1, x2, y2, x3, y3)
#define ANDROID_PATH_QUADRATIC_CURVE_TO(a,x1,y1,x2,y2)	\
	(*(a->env))->CallVoidMethod(a->env, a->state->path, a->path_quadTo, x1, y1, x2, y2)
#define ANDROID_PATH_CLOSE(a) \
	(*(a->env))->CallVoidMethod(a->env, a->state->path, a->path_close)
#define ANDROID_PATH_CLEAR(a,b)					\
	(*(a->env))->CallVoidMethod(a->env, b, a->path_reset)

#define ANDROID_PAINT_CREATE(a)	\
	(*(a->env))->NewObject(a->env, a->paint_clazz, a->paint_constructor)
#define ANDROID_PAINT_SET_EFFECT(a,b) \
	(*(a->env))->CallObjectMethod(a->env, a->state->paint, a->paint_setPathEffect, b)
#define ANDROID_PAINT_SET_COLOR(a,A,R,G,B) \
	(*(a->env))->CallVoidMethod(a->env, a->state->paint, a->paint_setARGB, A, R, G, B)
#define ANDROID_PAINT_SET_SHADER(a,S) \
	(void)(*(a->env))->CallObjectMethod(a->env, a->state->paint, a->paint_setShader, S)
#define ANDROID_PAINT_SET_MITER_LIMIT(a,b)	\
	(*(a->env))->CallVoidMethod(a->env, a->state->paint, a->paint_setStrokeMiter, b)
#define ANDROID_PAINT_SET_STROKE_WIDTH(a,b)	\
	(*(a->env))->CallVoidMethod(a->env, a->state->paint, a->paint_setStrokeWidth, b)
#define ANDROID_PAINT_SET(a,p,P)					\
	(*(a->env))->CallVoidMethod(a->env, p, a->paint_set, P)
#define ANDROID_PAINT_RESET(a,p)	\
	(*(a->env))->CallVoidMethod(a->env, p, a->paint_reset)

#define ANDROID_TEXT_PATH(e,T,X,Y)					\
	(*(e->env))->CallVoidMethod(e->env, e->state->paint, e->paint_getTextPath, (*(e->env))->NewStringUTF(e->env, T), 0, strlen_UTF8(T), X, Y, e->state->path)
#define ANDROID_SET_ANTIALIAS(e,P,B)					\
	(*(e->env))->CallVoidMethod(e->env, P, e->paint_setAntialias, B)
	
#define ANDROID_GET_DASHEFFECT(a,b,c)		\
	(*(a->env))->NewObject(a->env, a->dashPathEffect_clazz, a->dashPathEffect_constructor,b,c)
	
	
/* svg_android_state.c */

svg_android_status_t
_svg_android_state_create (svg_android_state_t **state, svg_android_t *svg_android);

svg_android_status_t
_svg_android_state_init (svg_android_state_t *state);

svg_android_status_t
_svg_android_state_init_copy (svg_android_state_t *state, const svg_android_state_t *other);

svg_android_status_t
_svg_android_state_deinit (svg_android_state_t *state);

svg_android_status_t
_svg_android_state_destroy (svg_android_state_t *state);

svg_android_state_t *
_svg_android_state_push (svg_android_t *instance, svg_android_state_t *state, jobject path_cache);

svg_android_state_t *
_svg_android_state_pop (svg_android_state_t *state);

/* svg_android_render.c */

	int _svg_android_get_last_bounding_box(void *closure, svg_bounding_box_t *bbox);
	void _svg_android_update_bounding_box(svg_bounding_box_t *bbox, svg_bounding_box_t *o_bbox);
	void _svg_android_update_last_bounding_box(svg_android_t *svg_android, svg_bounding_box_t *bbox);

	svg_status_t
_svg_android_begin_group (void *closure, double opacity);

svg_status_t
_svg_android_begin_element (void *closure, void *path_cache);

svg_status_t
_svg_android_end_element (void *closure);

svg_status_t
_svg_android_end_group (void *closure, double opacity);

svg_status_t
_svg_android_move_to (void *closure, double x, double y);

svg_status_t
_svg_android_line_to (void *closure, double x, double y);

svg_status_t
_svg_android_curve_to (void *closure,
		       double x1, double y1,
		       double x2, double y2,
		       double x3, double y3);

svg_status_t
_svg_android_quadratic_curve_to (void *closure,
				 double x1, double y1,
				 double x2, double y2);

svg_status_t
_svg_android_arc_to (void	       *closure,
		     double	rx,
		     double	ry,
		     double	x_axis_rotation,
		     int		large_arc_flag,
		     int		sweep_flag,
		     double	x,
		     double	y);

void
_svg_path_arc_segment (svg_android_t *svg_android,
		       double   xc,  double yc,
		       double   th0, double th1,
		       double   rx,  double ry,
		       double   x_axis_rotation);

svg_status_t
_svg_android_close_path (void *closure);

svg_status_t
_svg_android_free_path_cache(void *closure, void **path_cache);
	
svg_status_t
_svg_android_set_color (void *closure, const svg_color_t *color);

svg_status_t
_svg_android_set_fill_opacity (void *closure, double fill_opacity);

svg_status_t
_svg_android_set_fill_paint (void *closure, const svg_paint_t *paint);

svg_status_t
_svg_android_set_fill_rule (void *closure, svg_fill_rule_t fill_rule);

svg_status_t
_svg_android_select_font (svg_android_t *svg_android);

svg_status_t
_svg_android_set_font_family (void *closure, const char *family);

svg_status_t
_svg_android_set_font_size (void *closure, double size);

svg_status_t
_svg_android_set_font_style (void *closure, svg_font_style_t font_style);

svg_status_t
_svg_android_set_font_weight (void *closure, unsigned int weight);

svg_status_t
_svg_android_set_opacity (void *closure, double opacity);

svg_status_t
_svg_android_set_stroke_dash_array (void *closure, double *dash, int num_dashes);

svg_status_t
_svg_android_set_stroke_dash_offset (void *closure, svg_length_t *offset);

svg_status_t
_svg_android_set_stroke_line_cap (void *closure, svg_stroke_line_cap_t line_cap);

svg_status_t
_svg_android_set_stroke_line_join (void *closure, svg_stroke_line_join_t line_join);

svg_status_t
_svg_android_set_stroke_miter_limit (void *closure, double limit);

svg_status_t
_svg_android_set_stroke_opacity (void *closure, double stroke_opacity);

svg_status_t
_svg_android_set_stroke_paint (void *closure, const svg_paint_t *paint);

svg_status_t
_svg_android_set_stroke_width (void *closure, svg_length_t *width);

svg_status_t
_svg_android_set_text_anchor (void *closure, svg_text_anchor_t text_anchor);

	svg_status_t _svg_android_apply_clip_box (void *closure,
						  svg_length_t *x,
						  svg_length_t *y,
						  svg_length_t *width,
						  svg_length_t *height);
	
svg_status_t
_svg_android_transform (void *closure,
			double a, double b,
			double c, double d,
			double e, double f);

svg_status_t
_svg_android_apply_view_box (void *closure,
			     svg_view_box_t view_box,
			     svg_length_t *width,
			     svg_length_t *height);

svg_status_t
_svg_android_set_viewport_dimension (void *closure,
				     svg_length_t *width,
				     svg_length_t *height);

svg_status_t
_svg_android_render_line (void *closure,
			  svg_length_t *x1_len, svg_length_t *y1_len,
			  svg_length_t *x2_len, svg_length_t *y2_len);

svg_status_t
_svg_android_render_path (void *closure, void **path_cache);

svg_status_t
_svg_android_render_ellipse (void *closure,
			     svg_length_t *cx,
			     svg_length_t *cy,
			     svg_length_t *rx,
			     svg_length_t *ry);

svg_status_t
_svg_android_render_rect (void 	     *closure,
			  svg_length_t *x,
			  svg_length_t *y,
			  svg_length_t *width,
			  svg_length_t *height,
			  svg_length_t *rx,
			  svg_length_t *ry);

svg_status_t
_svg_android_render_text (void 	      *closure,
			  svg_length_t  *x_len,
			  svg_length_t  *y_len,
			  const char    *utf8);

svg_status_t
_svg_android_render_image (void		*closure,
			   unsigned char	*data,
			   unsigned int	data_width,
			   unsigned int	data_height,
			   svg_length_t	*x,
			   svg_length_t	*y,
			   svg_length_t	*width,
			   svg_length_t	*height);

svg_status_t
_svg_android_push_state (svg_android_t     *svg_android,
			 jobject offscreen_bitmap,
			 jobject path_cache);

svg_status_t
_svg_android_pop_state (svg_android_t *svg_android);

/* svg_android_render_helper.c */
svg_status_t
_svg_android_length_to_pixel (svg_android_t *svg_android, svg_length_t *length, double *pixel);

svg_status_t
_svg_android_set_gradient (svg_android_t *svg_android,
			   svg_gradient_t *gradient,
			   svg_android_render_type_t type);

svg_status_t
_svg_android_set_color_and_alpha (svg_android_t *svg_android,
				  svg_color_t *color,
				  double alpha);

svg_status_t
_svg_android_set_paint_and_opacity (svg_android_t *svg_android, svg_paint_t *paint, double opacity, svg_android_render_type_t type);

svg_status_t
_svg_android_set_pattern (svg_android_t *svg_android,
			  svg_element_t *pattern_element,
			  svg_android_render_type_t type);

svg_status_t
_svg_android_select_font (svg_android_t *svg_android);

void
_svg_android_copy_canvas_state (svg_android_t *svg_android);

#ifdef __cplusplus
}
#endif

#endif
