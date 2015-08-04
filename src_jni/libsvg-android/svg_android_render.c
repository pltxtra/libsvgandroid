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
 *
 */

#include <stdlib.h>
#include <string.h>

#include "svg-android-internal.h"
#include "math.h"

//#define __DO_SVG_ANDROID_DEBUG
#include "svg_android_debug.h"

#ifdef __DO_SVG_DETAIL_ANDROID_DEBUG
#define DEBUG_ENTRY(s) SVG_ANDROID_DEBUG("Entering function %s\n", s)
#define DEBUG_EXIT(s) SVG_ANDROID_DEBUG("Leaving function %s\n", s)
#else
#define DEBUG_ENTRY(s)
#define DEBUG_EXIT(s)
#endif

int _svg_android_get_last_bounding_box(void *closure, svg_bounding_box_t *bbox) {
	svg_android_t *svg_android = closure;

//	SVG_ANDROID_DEBUG("       --- boundingbox: %s\n", svg_android->state->bounding_box.left == -1 ? "INVALID" : "VALID");
	
	*bbox = svg_android->state->bounding_box;
	if(svg_android->state->bounding_box.left == -1)
		return 0;
	return -1;
}

void _svg_android_update_bounding_box(svg_bounding_box_t *bbox, svg_bounding_box_t *o_bbox) {
	if(bbox->left > o_bbox->left)
		bbox->left = o_bbox->left;
	if(bbox->top > o_bbox->top)
		bbox->top = o_bbox->top;
	if(bbox->right < o_bbox->right)
		bbox->right = o_bbox->right;
	if(bbox->bottom < o_bbox->bottom)
		bbox->bottom = o_bbox->bottom;
}

void _svg_android_update_last_bounding_box(svg_android_t *svg_android, svg_bounding_box_t *bbox) {
	_svg_android_update_bounding_box(&(svg_android->state->bounding_box), bbox);
}

svg_status_t
_svg_android_set_viewport_dimension (void *closure,
				     svg_length_t *width,
				     svg_length_t *height)
{
	svg_android_t *svg_android = closure;
	double vwidth, vheight;

	DEBUG_ENTRY("set_viewpoert_dimension");

	_svg_android_length_to_pixel (svg_android, width, &vwidth);
	_svg_android_length_to_pixel (svg_android, height, &vheight);

	svg_android->state->viewport_width  = vwidth;
	svg_android->state->viewport_height = vheight;

	if(svg_android->fit_to_area) {
		// calculate fit_to_MATRIX values
		double xx, x0, w;
		double yy, y0, h;

		w = (double)(svg_android->fit_to_w);
		h = (double)(svg_android->fit_to_h);
		
		xx = w / vwidth;
		yy = h / vheight;

		// force equal scaling...
		if(xx < yy) yy = xx;
		if(xx > yy) xx = yy;

		svg_android->fit_to_scale = xx;
		
		x0 = (double)(svg_android->fit_to_x);
		y0 = (double)(svg_android->fit_to_y);

		svg_android->fit_to_MATRIX =
			ANDROID_MATRIX_CREATE(svg_android, xx, 0.0, 0.0, yy, x0, y0);

		ANDROID_CANVAS_CONCAT_MATRIX(svg_android, svg_android->fit_to_MATRIX);
		
	} else svg_android->fit_to_scale = 1.0;

	DEBUG_EXIT("set_viewpoert_dimension");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_begin_group (void *closure, double opacity)
{
	svg_android_t *svg_android = closure;
	jobject offscreen_bitmap = NULL;

	DEBUG_ENTRY("begin_group");
	
	ANDROID_SAVE(svg_android);
	
	if (opacity != 1.0) {
		opacity *= 255;
		jint opacity_i = opacity;
		opacity_i = ((opacity_i & 0xff) << 24) & 0xffffffff;
		
		offscreen_bitmap = ANDROID_CREATE_BITMAP(svg_android,
							 svg_android->state->viewport_width,
							 svg_android->state->viewport_height);
		ANDROID_FILL_BITMAP(svg_android, offscreen_bitmap, opacity_i);
		
		svg_android->state->offscreen_bitmap = offscreen_bitmap;
	} else svg_android->state->offscreen_bitmap = NULL;

	_svg_android_push_state (svg_android, offscreen_bitmap, NULL);

	DEBUG_EXIT("begin_group");
	return SVG_ANDROID_STATUS_SUCCESS;
}

/* XXX: begin_element could be made more efficient in that no extra
   group is needed if there is only one element in a group */
svg_status_t
_svg_android_begin_element (void *closure, void *path_cache)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("begin_element");
	ANDROID_SAVE(svg_android);

	_svg_android_push_state (svg_android, NULL, (jobject)path_cache);

	DEBUG_EXIT("begin_element");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_end_element (void *closure)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("end_element");
	_svg_android_pop_state (svg_android);

	ANDROID_RESTORE(svg_android);

	DEBUG_EXIT("end_element");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_end_group (void *closure, double opacity)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("end_group");
	if (opacity != 1.0) {
		ANDROID_DRAW_BITMAP2(svg_android, svg_android->state->offscreen_bitmap, 0.0f, 0.0f);
	}

	_svg_android_pop_state (svg_android);

	ANDROID_RESTORE(svg_android);
	DEBUG_EXIT("end_group");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_move_to (void *closure, double x, double y)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("move_to");
	ANDROID_PATH_MOVE_TO(svg_android, x, y);
	svg_android->state->last_x = x;
	svg_android->state->last_y = y;
	DEBUG_EXIT("move_to");
	
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_line_to (void *closure, double x, double y)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("line_to");
	ANDROID_PATH_LINE_TO(svg_android, x, y);
	svg_android->state->last_x = x;
	svg_android->state->last_y = y;
	DEBUG_EXIT("line_to");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_curve_to (void *closure,
		       double x1, double y1,
		       double x2, double y2,
		       double x3, double y3)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("curve_to");
	ANDROID_PATH_CURVE_TO(svg_android, x1, y1, x2, y2, x3, y3);
	svg_android->state->last_x = x3;
	svg_android->state->last_y = y3;
	DEBUG_EXIT("curve_to");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_quadratic_curve_to (void *closure,
				 double x1, double y1,
				 double x2, double y2)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("quadratic_curve_to");
	ANDROID_PATH_QUADRATIC_CURVE_TO(svg_android, x1, y1, x2, y2);
	svg_android->state->last_x = x2;
	svg_android->state->last_y = y2;
	DEBUG_EXIT("quadratic_curve_to");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_close_path (void *closure)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("close_path");
	ANDROID_PATH_CLOSE(svg_android);
	svg_android->state->last_x = 0.0; // don't know if this is right, really... 
	svg_android->state->last_y = 0.0;
	DEBUG_EXIT("close_path");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_free_path_cache(void *closure, void **path_cache) {
	svg_android_t *svg_android = closure;

	(*(svg_android->env))->DeleteGlobalRef(svg_android->env, (jobject)(*path_cache));
	*path_cache = NULL;

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_color (void *closure, const svg_color_t *color)
{
	svg_android_t *svg_android = closure;
	DEBUG_ENTRY("set_color");

	svg_android->state->color = *color;

	DEBUG_EXIT("set_color");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_fill_opacity (void *closure, double fill_opacity)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_fill_opacity");
	svg_android->state->fill_opacity = fill_opacity;

	DEBUG_EXIT("set_fill_opacity");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_fill_paint (void *closure, const svg_paint_t *paint)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_fill_paint");
	svg_android->state->fill_paint = *paint;
	DEBUG_EXIT("set_fill_paint");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_fill_rule (void *closure, svg_fill_rule_t fill_rule)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_fill_rule");
	switch (fill_rule) {
	case SVG_FILL_RULE_NONZERO:
		ANDROID_SET_FILL_TYPE(svg_android, svg_android->state->path, JNI_FALSE);
		break;
	case SVG_FILL_RULE_EVEN_ODD:
		ANDROID_SET_FILL_TYPE(svg_android, svg_android->state->path, JNI_TRUE);
		break;
	}

	svg_android->state->fill_rule = fill_rule;
	
	DEBUG_EXIT("set_fill_rule");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_font_family (void *closure, const char *family)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_font_family");
	if (svg_android->state->font_family)
		free (svg_android->state->font_family);

	svg_android->state->font_family = strdup (family);
	svg_android->state->font_dirty = 1;
	DEBUG_EXIT("set_font_family");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_font_size (void *closure, double size)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_font_size");
	svg_android->state->font_size = size;
	svg_android->state->font_dirty = 1;
	DEBUG_EXIT("set_font_size");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_font_style (void *closure, svg_font_style_t font_style)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_font_style");
	svg_android->state->font_style = font_style;
	svg_android->state->font_dirty = 1;
	DEBUG_EXIT("set_font_style");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_font_weight (void *closure, unsigned int font_weight)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_font_weight");
	svg_android->state->font_weight = font_weight;
	svg_android->state->font_dirty = 1;
	DEBUG_EXIT("set_font_weight");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_opacity (void *closure, double opacity)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_opacity");
	svg_android->state->opacity = opacity;
	DEBUG_EXIT("set_opacity");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_dash_array (void *closure, double *dash, int num_dashes)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_stroke_dash_array");
	if(svg_android->state->dash != NULL) {
		free (svg_android->state->dash);
		svg_android->state->dash = NULL;
	}

	svg_android->state->num_dashes = num_dashes;

	if (svg_android->state->num_dashes) {
		jfloatArray farr;

		svg_android->state->dash = malloc(svg_android->state->num_dashes * sizeof(double));
		if (svg_android->state->dash == NULL)
			return SVG_STATUS_NO_MEMORY;

		memcpy(svg_android->state->dash, dash, svg_android->state->num_dashes * sizeof(double));

		// prepare Android float array with dashes, make sure the array is even in length..
		{
			int max_k;
			int k;
			jfloat *buf;

			max_k = svg_android->state->num_dashes;
			if(max_k & 0x1) max_k++; // make even

			buf = (jfloat *)malloc(sizeof(jfloat) * max_k);
			if(buf == NULL)
				return SVG_STATUS_NO_MEMORY;
			
			farr = (*(svg_android->env))->NewFloatArray(svg_android->env, max_k);
			
			if (farr == NULL) {
				free(buf);
				return SVG_ANDROID_STATUS_NO_MEMORY; /* out of memory error thrown */
			}
			
			for(k = 0; k < svg_android->state->num_dashes; k++) {
				buf[k] = svg_android->state->dash[k];
			}
			for(; k < max_k; k++) buf[k] = 0.0;
			
			(*(svg_android->env))->SetFloatArrayRegion(svg_android->env, farr, 0, max_k, buf);

			free(buf);
		}

		jobject effect = ANDROID_GET_DASHEFFECT(
			svg_android, farr,
			svg_android->state->dash_offset);
		ANDROID_PAINT_SET_EFFECT(svg_android, effect);
	}

	DEBUG_EXIT("set_stroke_dash_array");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_dash_offset (void *closure, svg_length_t *offset_len)
{
	svg_android_t *svg_android = closure;
	double offset;
	
	DEBUG_ENTRY("set_stroke_dash_offset");

	_svg_android_length_to_pixel (svg_android, offset_len, &offset);

	svg_android->state->dash_offset = offset;

	if (svg_android->state->num_dashes) {
		jfloatArray farr;

		// prepare Android float array with dashes, make sure the array is even in length..
		{
			int max_k;
			int k;
			jfloat *buf;

			max_k = svg_android->state->num_dashes;
			if(max_k & 0x1) max_k++; // make even

			buf = (jfloat *)malloc(sizeof(jfloat) * max_k);
			if(buf == NULL)
				return SVG_STATUS_NO_MEMORY;
			
			farr = (*(svg_android->env))->NewFloatArray(svg_android->env, max_k);
			
			if (farr == NULL) {
				free(buf);
				return SVG_ANDROID_STATUS_NO_MEMORY; /* out of memory error thrown */
			}
			
			for(k = 0; k < svg_android->state->num_dashes; k++) {
				buf[k] = svg_android->state->dash[k];
			}
			for(; k < max_k; k++) buf[k] = 0.0;
			
			(*(svg_android->env))->SetFloatArrayRegion(svg_android->env, farr, 0, max_k, buf);

			free(buf);
		}

		jobject effect = ANDROID_GET_DASHEFFECT(
			svg_android, farr,
			svg_android->state->dash_offset);
		ANDROID_PAINT_SET_EFFECT(svg_android, effect);
	}
	
	DEBUG_EXIT("set_stroke_dash_offset");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_line_cap (void *closure, svg_stroke_line_cap_t line_cap)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_stroke_line_cap");

	switch (line_cap) {
	case SVG_STROKE_LINE_CAP_BUTT:
		ANDROID_SET_STROKE_CAP(svg_android, svg_android->state->paint, 0);
		break;
	case SVG_STROKE_LINE_CAP_ROUND:
		ANDROID_SET_STROKE_CAP(svg_android, svg_android->state->paint, 1);
		break;
	case SVG_STROKE_LINE_CAP_SQUARE:
		ANDROID_SET_STROKE_CAP(svg_android, svg_android->state->paint, 2);
		break;
	}

	svg_android->state->line_cap = line_cap;

	DEBUG_EXIT("set_stroke_line_cap");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_line_join (void *closure, svg_stroke_line_join_t line_join)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_stroke_line_join");

	switch (line_join) {
	case SVG_STROKE_LINE_JOIN_MITER:
		ANDROID_SET_STROKE_JOIN(svg_android, svg_android->state->paint, 0);
		break;
	case SVG_STROKE_LINE_JOIN_ROUND:
		ANDROID_SET_STROKE_JOIN(svg_android, svg_android->state->paint, 1);
		break;
	case SVG_STROKE_LINE_JOIN_BEVEL:
		ANDROID_SET_STROKE_JOIN(svg_android, svg_android->state->paint, 2);
		break;
	}

	svg_android->state->line_join = line_join;

	DEBUG_EXIT("set_stroke_line_join");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_miter_limit (void *closure, double limit)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_stroke_miter_limit");
	ANDROID_PAINT_SET_MITER_LIMIT(svg_android, limit);

	svg_android->state->miter_limit = limit;
	DEBUG_EXIT("set_stroke_miter_limit");
	
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_opacity (void *closure, double stroke_opacity)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_stroke_opacity");
	svg_android->state->stroke_opacity = stroke_opacity;
	DEBUG_EXIT("set_stroke_opacity");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_paint (void *closure, const svg_paint_t *paint)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_stroke_paint");
	svg_android->state->stroke_paint = *paint;
	DEBUG_EXIT("set_stroke_paint");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_stroke_width (void *closure, svg_length_t *width_len)
{
	svg_android_t *svg_android = closure;
	double width;

	DEBUG_ENTRY("set_stroke_width");
	_svg_android_length_to_pixel (svg_android, width_len, &width);
	
	svg_android->state->width_len.unit = width_len->unit;
	svg_android->state->width_len.value = width_len->value;

	// make sure stroke width is also scaled to fit area...
	ANDROID_PAINT_SET_STROKE_WIDTH(svg_android, width * svg_android->fit_to_scale);
	DEBUG_EXIT("set_stroke_width");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_set_text_anchor (void *closure, svg_text_anchor_t text_anchor)
{
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("set_text_anchor");
	svg_android->state->text_anchor = text_anchor;
	DEBUG_EXIT("set_text_anchor");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_apply_clip_box (void *closure,
			     svg_length_t *x_l,
			     svg_length_t *y_l,
			     svg_length_t *width_l,
			     svg_length_t *height_l) {
	svg_android_t *svg_android = closure;

	DEBUG_ENTRY("apply_clip_box");

	double x, y, width, height;
	
	_svg_android_length_to_pixel (svg_android, x_l, &x);
	_svg_android_length_to_pixel (svg_android, y_l, &y);
	_svg_android_length_to_pixel (svg_android, width_l, &width);
	_svg_android_length_to_pixel (svg_android, height_l, &height);

	width += x; height += y;

	SVG_ANDROID_DEBUG("clip box: %f, %f, %f, %f\n", x, y, width, height);
	
	ANDROID_CANVAS_CLIP_RECT(svg_android,
				 (float)(x), (float)(y),
				 (float)(width),
				 (float)(height));

	DEBUG_EXIT("apply_clip_box");

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_transform (void *closure,
			double xx, double yx,
			double xy, double yy,
			double x0, double y0)
{
	svg_android_t *svg_android = closure;

	jobject new_matrix = svg_android->state->matrix;

	DEBUG_ENTRY("transform");
	ANDROID_MATRIX_INIT(svg_android, new_matrix, xx, yx, xy, yy, x0, y0);

	ANDROID_CANVAS_CONCAT_MATRIX(svg_android, new_matrix);
	
	DEBUG_EXIT("transform");
	
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_render_line (void *closure,
			  svg_length_t *x1_len, svg_length_t *y1_len,
			  svg_length_t *x2_len, svg_length_t *y2_len)
{
	svg_android_t *svg_android = closure;
	svg_status_t status;
	double x1, y1, x2, y2;

	DEBUG_ENTRY("render_line");
	_svg_android_length_to_pixel (svg_android, x1_len, &x1);
	_svg_android_length_to_pixel (svg_android, y1_len, &y1);
	_svg_android_length_to_pixel (svg_android, x2_len, &x2);
	_svg_android_length_to_pixel (svg_android, y2_len, &y2);

	status = _svg_android_move_to (svg_android, x1, y1);
	if (status)
		return status;

	status = _svg_android_line_to (svg_android, x2, y2);
	if (status)
		return status;

	status = _svg_android_render_path (svg_android, NULL);
	if (status)
		return status;

	DEBUG_EXIT("render_line");
	return SVG_ANDROID_STATUS_SUCCESS;

}

svg_status_t
_svg_android_render_path (void *closure, void **path_cache)
{
	svg_android_t *svg_android = closure;
	svg_paint_t *fill_paint, *stroke_paint;

	DEBUG_ENTRY("render_path");
	fill_paint = &svg_android->state->fill_paint;
	stroke_paint = &svg_android->state->stroke_paint;
	
	if (fill_paint->type) {
		_svg_android_set_paint_and_opacity (svg_android, fill_paint,
						    svg_android->state->fill_opacity,
						    SVG_ANDROID_RENDER_TYPE_FILL);
		
		if((*(svg_android->env))->ExceptionOccurred(svg_android->env)) {
			(*(svg_android->env))->ExceptionDescribe(svg_android->env);
		}
		
		ANDROID_DRAW_PATH(svg_android,
				  svg_android->state->path, svg_android->state->paint);		
		if((*(svg_android->env))->ExceptionOccurred(svg_android->env)) {
			(*(svg_android->env))->ExceptionDescribe(svg_android->env);
		}
	}

	if (stroke_paint->type) {
		_svg_android_set_paint_and_opacity (svg_android, stroke_paint,
						    svg_android->state->stroke_opacity,
						    SVG_ANDROID_RENDER_TYPE_STROKE);

		ANDROID_DRAW_PATH(svg_android, svg_android->state->path, svg_android->state->paint);
	}

	{
		ANDROID_GET_PATH_BOUNDING_BOX(svg_android, svg_android->state->path);
		static svg_bounding_box_t bbox;
		if(svgAndroidGetInternalBoundingBox(&bbox)) {
			_svg_android_update_last_bounding_box(svg_android, &bbox);
		}
		
	}

	if(path_cache && (*path_cache == NULL)) {
		jobject cloned_path = ANDROID_PATH_CLONE(svg_android, svg_android->state->path);
		cloned_path = (*(svg_android->state->instance->env))->NewGlobalRef(
				svg_android->state->instance->env, cloned_path);
		*path_cache = (void *)cloned_path;
	} else if(!path_cache) {
		ANDROID_PATH_CLEAR(svg_android, svg_android->state->path);
	}
	
	DEBUG_EXIT("render_path");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_render_ellipse (void *closure,
			     svg_length_t *cx_len,
			     svg_length_t *cy_len,
			     svg_length_t *rx_len,
			     svg_length_t *ry_len)
{
	svg_android_t *svg_android = closure;

	double cx, cy, rx, ry;

	DEBUG_ENTRY("render_ellipse");

	_svg_android_length_to_pixel (svg_android, cx_len, &cx);
	_svg_android_length_to_pixel (svg_android, cy_len, &cy);
	_svg_android_length_to_pixel (svg_android, rx_len, &rx);
	_svg_android_length_to_pixel (svg_android, ry_len, &ry);

	ANDROID_DRAW_ELLIPSE(svg_android, cx, cy, rx, ry);

	{
		static svg_bounding_box_t bbox;
		if(svgAndroidGetInternalBoundingBox(&bbox)) {
			_svg_android_update_last_bounding_box(svg_android, &bbox);
		}
	}
	
	DEBUG_EXIT("render_ellipse");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_render_rect (void *closure,
			  svg_length_t *x_len,
			  svg_length_t *y_len,
			  svg_length_t *width_len,
			  svg_length_t *height_len,
			  svg_length_t *rx_len,
			  svg_length_t *ry_len)
{
	svg_android_t *svg_android = closure;

	double x, y, width, height, rx, ry;
 
	DEBUG_ENTRY("render_rect");
	_svg_android_length_to_pixel (svg_android, x_len, &x);
	_svg_android_length_to_pixel (svg_android, y_len, &y);
	_svg_android_length_to_pixel (svg_android, width_len, &width);
	_svg_android_length_to_pixel (svg_android, height_len, &height);
	_svg_android_length_to_pixel (svg_android, rx_len, &rx);
	_svg_android_length_to_pixel (svg_android, ry_len, &ry);
 
	if (rx > width / 2.0)
		rx = width / 2.0;
	if (ry > height / 2.0)
		ry = height / 2.0;

	svg_paint_t *fill_paint, *stroke_paint;

	fill_paint = &svg_android->state->fill_paint;
	stroke_paint = &svg_android->state->stroke_paint;

	if (fill_paint->type) {
		_svg_android_set_paint_and_opacity (svg_android, fill_paint,
						    svg_android->state->fill_opacity,
						    SVG_ANDROID_RENDER_TYPE_FILL);

		ANDROID_DRAW_RECT(svg_android, x, y, width, height, rx, ry);
	}

	if (stroke_paint->type) {
		_svg_android_set_paint_and_opacity (svg_android, stroke_paint,
						    svg_android->state->stroke_opacity,
						    SVG_ANDROID_RENDER_TYPE_STROKE);
		
		ANDROID_DRAW_RECT(svg_android, x, y, width, height, rx, ry);
	}

	{
		static svg_bounding_box_t bbox;
		if(svgAndroidGetInternalBoundingBox(&bbox)) {
			_svg_android_update_last_bounding_box(svg_android, &bbox);
		}
	}

	DEBUG_EXIT("render_rect");
	return SVG_ANDROID_STATUS_SUCCESS;
}

// returns the length of a null terminated string, considering some characters are multi-byte encoded.
size_t strlen_UTF8(const char *utf8) {	
	size_t length = 0;

	while(*utf8 != '\0') {
		length++;
		
		int step = 0;
	
		// check for multi byte characters
		if(((*utf8) & 0xe0) == 0xc0) {
			// b110xxxxx => two byte encoded
			step = 1;
		} else if(((*utf8) & 0xf0) == 0xe0) {
			// b1110xxxx => three byte encoded
			step = 2;
		}
		else if(((*utf8) & 0xf8) == 0xf0) {
			// b11110xxx => four byte encoded
			step = 3;
		}
		utf8 = &(utf8[1]);
		for(; step > 0; step--) {
			// don't increase if followup byte is illegaly encoded
			if(((*utf8) & 0xc0) == 0x80)
				utf8 = &(utf8[1]);
		}
	}

	return length;
}

svg_status_t
_svg_android_render_text (void *closure,
			  svg_length_t *x_len,
			  svg_length_t *y_len,
			  const char *utf8)
{
	svg_android_t *svg_android = closure;
	double x, y;
	svg_status_t status;
	svg_paint_t *fill_paint, *stroke_paint;

	DEBUG_ENTRY("render_text");
	fill_paint = &svg_android->state->fill_paint;
	stroke_paint = &svg_android->state->stroke_paint;

	_svg_android_select_font (svg_android);

	_svg_android_length_to_pixel (svg_android, x_len, &x);
	_svg_android_length_to_pixel (svg_android, y_len, &y);
	
	status = _svg_android_move_to (svg_android, x, y);
	if (status)
		return status;

	ANDROID_TEXT_PATH(svg_android, utf8, x, y);
	_svg_android_close_path (svg_android);
	
	if (fill_paint->type) {
		_svg_android_set_paint_and_opacity (svg_android, fill_paint,
						    svg_android->state->fill_opacity,
						    SVG_ANDROID_RENDER_TYPE_FILL);
		_svg_android_render_path (svg_android, NULL);

	}

	if (stroke_paint->type) {
		_svg_android_set_paint_and_opacity (svg_android, stroke_paint,
						    svg_android->state->stroke_opacity,
						    SVG_ANDROID_RENDER_TYPE_STROKE);
		_svg_android_render_path (svg_android, NULL);
	}

	DEBUG_EXIT("render_text");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_render_image (void		*closure,
			   unsigned char	*data,
			   unsigned int	data_width,
			   unsigned int	data_height,
			   svg_length_t	*x_len,
			   svg_length_t	*y_len,
			   svg_length_t	*width_len,
			   svg_length_t	*height_len)
{
	svg_android_t *svg_android = closure;
	double x, y, width, height;

	jintArray iarr;
	jobject bitmap;
	jobject matrix;

	DEBUG_ENTRY("render_image");
	ANDROID_SAVE(svg_android);

	_svg_android_length_to_pixel (svg_android, x_len, &x);
	_svg_android_length_to_pixel (svg_android, y_len, &y);
	_svg_android_length_to_pixel (svg_android, width_len, &width);
	_svg_android_length_to_pixel (svg_android, height_len, &height);

	// copy bitmap into an java int array
	iarr = (*(svg_android->env))->NewIntArray(svg_android->env, data_width * data_height);
	(*(svg_android->env))->SetIntArrayRegion(svg_android->env, iarr, 0, data_width * data_height , (jint *)data);

	// create bitmap
	bitmap = ANDROID_DATA_2_BITMAP(svg_android, iarr, data_width, data_height);

	// prepare matrix
	matrix = ANDROID_IDENTITY_MATRIX(svg_android);
	ANDROID_MATRIX_TRANSLATE(svg_android, matrix, x, y);
	ANDROID_MATRIX_SCALE(svg_android, matrix, width / data_width, height / data_height);

	// and draw!
	ANDROID_DRAW_BITMAP(svg_android, bitmap, matrix);	

	ANDROID_RESTORE(svg_android);

	DEBUG_EXIT("render_image");
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t
_svg_android_push_state (svg_android_t     *svg_android,
			 jobject offscreen_bitmap,
			 jobject path_cache)
{
	DEBUG_ENTRY("push_state");
	if (svg_android->state == NULL)
	{
		if((svg_android->state = _svg_android_state_push (svg_android, NULL, path_cache)) == NULL)
			return SVG_STATUS_NO_MEMORY;
		
		svg_android->state->viewport_width = svg_android->viewport_width;
		svg_android->state->viewport_height = svg_android->viewport_height;
	}
	else
	{
		if (offscreen_bitmap)
		{
			jobject new_canvas = ANDROID_CANVAS_CREATE(svg_android, offscreen_bitmap);

			svg_android->state->saved_canvas = svg_android->canvas;
			svg_android->canvas = new_canvas;

			_svg_android_copy_canvas_state (svg_android);
		}
		if((svg_android->state = _svg_android_state_push (svg_android, svg_android->state, path_cache)) == NULL)
			return SVG_STATUS_NO_MEMORY;
	}    

	DEBUG_EXIT("push_state");
	return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_android_pop_state (svg_android_t *svg_android)
{
	
	DEBUG_ENTRY("pop_state");
	svg_android->state = _svg_android_state_pop (svg_android->state);

	if (svg_android->state && svg_android->state->saved_canvas) {
		svg_android->canvas = svg_android->state->saved_canvas;
		svg_android->state->saved_canvas = NULL;
	}
	DEBUG_EXIT("pop_state");

	return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_android_apply_view_box (void *closure,
			     svg_view_box_t view_box,
			     svg_length_t *width,
			     svg_length_t *height)
{
	svg_android_t *svg_android = closure;
	double vpar, svgar;
	double logic_width, logic_height;
	double logic_x, logic_y;
	double phys_width, phys_height;

	DEBUG_ENTRY("apply_view_box");
	_svg_android_length_to_pixel (svg_android, width, &phys_width);
	_svg_android_length_to_pixel (svg_android, height, &phys_height);

	vpar = view_box.box.width / view_box.box.height;
	svgar = phys_width / phys_height;
	logic_x = view_box.box.x;
	logic_y = view_box.box.y;
	logic_width = view_box.box.width;
	logic_height = view_box.box.height;

	
	
	if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_NONE)
	{
		ANDROID_MATRIX_SCALE(svg_android,svg_android->state->matrix,
				     phys_width / logic_width,
				     phys_height / logic_height);
		ANDROID_MATRIX_TRANSLATE(svg_android, svg_android->state->matrix, -logic_x, -logic_y);
	}
	else if ((vpar < svgar && view_box.meet_or_slice == SVG_MEET_OR_SLICE_MEET) ||
		 (vpar >= svgar && view_box.meet_or_slice == SVG_MEET_OR_SLICE_SLICE))
	{
		ANDROID_MATRIX_SCALE(svg_android, svg_android->state->matrix,
				     phys_height / logic_height, phys_height / logic_height);

		if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMIN ||
		    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMID ||
		    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMAX)
			ANDROID_MATRIX_TRANSLATE(
				svg_android, svg_android->state->matrix, -logic_x, -logic_y);
		else if(view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMID ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX)
			ANDROID_MATRIX_TRANSLATE(
				svg_android, svg_android->state->matrix,
				-logic_x -
				(logic_width - phys_width * logic_height / phys_height) / 2,
				-logic_y);
		else
			ANDROID_MATRIX_TRANSLATE(
				svg_android, svg_android->state->matrix,
				-logic_x - (logic_width - phys_width * logic_height / phys_height),
				-logic_y);
	}
	else
	{
		ANDROID_MATRIX_SCALE(svg_android, svg_android->state->matrix,
				     phys_width / logic_width, phys_width / logic_width);

		if (view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMIN ||
		    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN ||
		    view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN)
			ANDROID_MATRIX_TRANSLATE(
				svg_android, svg_android->state->matrix,
				-logic_x, -logic_y);
		else if(view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMINYMID ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMIDYMID ||
			view_box.aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_XMAXYMID)
			ANDROID_MATRIX_TRANSLATE(
				svg_android, svg_android->state->matrix,
				-logic_x,
				-logic_y - (logic_height - phys_height * logic_width / phys_width) / 2);
		else
			ANDROID_MATRIX_TRANSLATE(
				svg_android, svg_android->state->matrix,
				-logic_x,
				-logic_y - (logic_height - phys_height * logic_width / phys_width));
	}

	DEBUG_EXIT("apply_view_box");
	return SVG_STATUS_SUCCESS;
}

/* The ellipse and arc functions below are:
 
   Copyright (C) 2000 Eazel, Inc.
  
   Author: Raph Levien <raph@artofcode.com>

   This is adapted from svg-path in Gill.
*/
void
_svg_path_arc_segment (svg_android_t *svg_android,
		       double xc, double yc,
		       double th0, double th1,
		       double rx, double ry, double x_axis_rotation)
{
	double sin_th, cos_th;
	double a00, a01, a10, a11;
	double x1, y1, x2, y2, x3, y3;
	double t;
	double th_half;

	sin_th = sin (x_axis_rotation * (M_PI / 180.0));
	cos_th = cos (x_axis_rotation * (M_PI / 180.0)); 
	/* inverse transform compared with rsvg_path_arc */
	a00 = cos_th * rx;
	a01 = -sin_th * ry;
	a10 = sin_th * rx;
	a11 = cos_th * ry;

	th_half = 0.5 * (th1 - th0);
	t = (8.0 / 3.0) * sin (th_half * 0.5) * sin (th_half * 0.5) / sin (th_half);
	x1 = xc + cos (th0) - t * sin (th0);
	y1 = yc + sin (th0) + t * cos (th0);
	x3 = xc + cos (th1);
	y3 = yc + sin (th1);
	x2 = x3 + t * sin (th1);
	y2 = y3 - t * cos (th1);

	ANDROID_PATH_CURVE_TO(svg_android,
			      a00 * x1 + a01 * y1, a10 * x1 + a11 * y1,
			      a00 * x2 + a01 * y2, a10 * x2 + a11 * y2,
			      a00 * x3 + a01 * y3, a10 * x3 + a11 * y3);
}

/**
 * _svg_android_path_arc_to: Add an arc to the given path
 *
 * rx: Radius in x direction (before rotation).
 * ry: Radius in y direction (before rotation).
 * x_axis_rotation: Rotation angle for axes.
 * large_arc_flag: 0 for arc length <= 180, 1 for arc >= 180.
 * sweep: 0 for "negative angle", 1 for "positive angle".
 * x: New x coordinate.
 * y: New y coordinate.
 *
 **/
svg_status_t
_svg_android_arc_to (void		*closure,
		     double		rx,
		     double		ry,
		     double		x_axis_rotation,
		     int		large_arc_flag,
		     int		sweep_flag,
		     double		x,
		     double		y)
{
	svg_android_t *svg_android = closure;
	double sin_th, cos_th;
	double a00, a01, a10, a11;
	double x0, y0, x1, y1, xc, yc;
	double d, sfactor, sfactor_sq;
	double th0, th1, th_arc;
	int i, n_segs;
	double dx, dy, dx1, dy1, Pr1, Pr2, Px, Py, check;
	double curx, cury;

	DEBUG_ENTRY("arc_to");

	rx = fabs (rx);
	ry = fabs (ry);

	curx = svg_android->state->last_x;
	cury = svg_android->state->last_y;

	svg_android->state->last_x = x;
	svg_android->state->last_y = y;

	sin_th = sin (x_axis_rotation * (M_PI / 180.0));
	cos_th = cos (x_axis_rotation * (M_PI / 180.0));

	dx = (curx - x) / 2.0;
	dy = (cury - y) / 2.0;
	dx1 =  cos_th * dx + sin_th * dy;
	dy1 = -sin_th * dx + cos_th * dy;
	Pr1 = rx * rx;
	Pr2 = ry * ry;
	Px = dx1 * dx1;
	Py = dy1 * dy1;
	/* Spec : check if radii are large enough */
	check = Px / Pr1 + Py / Pr2;
	if(check > 1)
	{
		rx = rx * sqrt(check);
		ry = ry * sqrt(check);
	}

	a00 = cos_th / rx;
	a01 = sin_th / rx;
	a10 = -sin_th / ry;
	a11 = cos_th / ry;
	x0 = a00 * curx + a01 * cury;
	y0 = a10 * curx + a11 * cury;
	x1 = a00 * x + a01 * y;
	y1 = a10 * x + a11 * y;
	/* (x0, y0) is current point in transformed coordinate space.
	   (x1, y1) is new point in transformed coordinate space.
       
	   The arc fits a unit-radius circle in this space.
	*/
	d = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
	sfactor_sq = 1.0 / d - 0.25;
	if (sfactor_sq < 0) sfactor_sq = 0;
	sfactor = sqrt (sfactor_sq);
	if (sweep_flag == large_arc_flag) sfactor = -sfactor;
	xc = 0.5 * (x0 + x1) - sfactor * (y1 - y0);
	yc = 0.5 * (y0 + y1) + sfactor * (x1 - x0);
	/* (xc, yc) is center of the circle. */
    
	th0 = atan2 (y0 - yc, x0 - xc);
	th1 = atan2 (y1 - yc, x1 - xc);
    
	th_arc = th1 - th0;
	if (th_arc < 0 && sweep_flag)
		th_arc += 2 * M_PI;
	else if (th_arc > 0 && !sweep_flag)
		th_arc -= 2 * M_PI;

	/* XXX: I still need to evaluate the math performed in this
	   function. The critical behavior desired is that the arc must be
	   approximated within an arbitrary error tolerance, (which the
	   user should be able to specify as well). I don't yet know the
	   bounds of the error from the following computation of
	   n_segs. Plus the "+ 0.001" looks just plain fishy. -cworth */
	n_segs = ceil (fabs (th_arc / (M_PI * 0.5 + 0.001)));
    
	for (i = 0; i < n_segs; i++) {
		_svg_path_arc_segment (svg_android, xc, yc,
				       th0 + i * th_arc / n_segs,
				       th0 + (i + 1) * th_arc / n_segs,
				       rx, ry, x_axis_rotation);
	}

	DEBUG_EXIT("arc_to");
	return SVG_ANDROID_STATUS_SUCCESS;
}


