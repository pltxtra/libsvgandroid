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

#include <stdlib.h>
#include <string.h>

#include "svg-android-internal.h"
#include "math.h"

#include <android/log.h>

#ifdef DEBUG_LIBSVG_ANDROID
#define DEBUG_ENTRY(s) __android_log_print(ANDROID_LOG_INFO, "libsvg-android", "Entering function %s\n", s); fflush(0)
#define DEBUG_ANDROID(s) __android_log_print(ANDROID_LOG_INFO, "libsvg-android", "%s", s)
#define DEBUG_EXIT(s) __android_log_print(ANDROID_LOG_INFO, "libsvg-android", "Leaving function %s\n", s); fflush(0)
#else
#define DEBUG_ENTRY(s)
#define DEBUG_ANDROID(s)
#define DEBUG_EXIT(s)
#endif

svg_status_t _svg_android_set_gradient (svg_android_t *svg_android,
			   svg_gradient_t *gradient,
			   svg_android_render_type_t type)
{
	svg_gradient_stop_t *stop;
	int i;
	jobject matrix, gradient_matrix;
	jobject gradient_shader = NULL;

	matrix = ANDROID_IDENTITY_MATRIX(svg_android);

	switch (gradient->units) {
	case SVG_GRADIENT_UNITS_USER:
		break;
	case SVG_GRADIENT_UNITS_BBOX:
	{
		jfloatArray farr;
		jfloat *coords;
		
		farr = ANDROID_PATH_GET_BOUNDS(svg_android, svg_android->state->path);
		
		coords = (*(svg_android->env))->GetFloatArrayElements((svg_android->env), farr, 0);

		// Maybe we need to add the stroke width to be correct here? (if type == SVG_ANDROID_RENDER_TYPE_STROKE)
		ANDROID_MATRIX_TRANSLATE(svg_android, matrix, coords[0], coords[1]);
		ANDROID_MATRIX_SCALE(svg_android, matrix, coords[2] - coords[0], coords[3] - coords[1]);
		(*(svg_android->env))->ReleaseFloatArrayElements(svg_android->env, farr, coords, 0);		
		
#if 0 // note here how the cairo version checks for fill or stroke, the extents might be different.. but for android I use the bounds of the path, this doesn't account for stroke painting outside..
		double x1, y1, x2, y2;

		if (type == SVG_ANDROID_RENDER_TYPE_FILL)
			cairo_fill_extents (svg_android->cr, &x1, &y1, &x2, &y2);
		else
			cairo_stroke_extents (svg_android->cr, &x1, &y1, &x2, &y2);

		cairo_matrix_translate (&matrix, x1, y1);
		cairo_matrix_scale (&matrix, x2 - x1, y2 - y1);

#endif
		svg_android->state->bbox = 1;
	} break;
	}

	// create java float array for the stops offsets, and int array for the colors
	jfloat offsets[gradient->num_stops];
	jint colors[gradient->num_stops];
	for (i = 0; i < gradient->num_stops; i++) {
		stop = &gradient->stops[i];
		offsets[i] = stop->offset;
		unsigned long r, g, b, o;
		r = svg_color_get_red (&stop->color);
		g = svg_color_get_green (&stop->color);
		b = svg_color_get_blue (&stop->color);
		o = (unsigned long)(stop->opacity * 255.0);
		colors[i] =
			(o & 0xff) << 24 |
			(r & 0xff) << 16 |
			(g & 0xff) << 8 |
			(b & 0xff);		
	}
	jfloatArray offsets_a;
	jintArray colors_a;
	offsets_a = (*(svg_android->env))->NewFloatArray(svg_android->env, gradient->num_stops);
	colors_a = (*(svg_android->env))->NewIntArray(svg_android->env, gradient->num_stops);

	(*(svg_android->env))->SetFloatArrayRegion(svg_android->env, offsets_a, 0, gradient->num_stops, offsets);
	(*(svg_android->env))->SetIntArrayRegion(svg_android->env, colors_a, 0, gradient->num_stops, colors);

	int spreadType = 2;
	
	switch (gradient->spread) {
	case SVG_GRADIENT_SPREAD_REPEAT:
		spreadType = 0;
		break;
	case SVG_GRADIENT_SPREAD_REFLECT:
		spreadType = 1;
		break;
	default:
		spreadType = 2;
		break;
	}
	    
	switch (gradient->type) {
	case SVG_GRADIENT_LINEAR:
	{
		double x1, y1, x2, y2;

		_svg_android_length_to_pixel (svg_android, &gradient->u.linear.x1, &x1);
		_svg_android_length_to_pixel (svg_android, &gradient->u.linear.y1, &y1);
		_svg_android_length_to_pixel (svg_android, &gradient->u.linear.x2, &x2);
		_svg_android_length_to_pixel (svg_android, &gradient->u.linear.y2, &y2);

		if((*(svg_android->env))->ExceptionOccurred(svg_android->env)) {
			(*(svg_android->env))->ExceptionDescribe(svg_android->env);
		}
		gradient_shader = ANDROID_CREATE_LINEAR_GRADIENT(svg_android, x1, y1, x2, y2, colors_a, offsets_a, spreadType);
		if((*(svg_android->env))->ExceptionOccurred(svg_android->env)) {
			(*(svg_android->env))->ExceptionDescribe(svg_android->env);
		}
	}
	break;
	case SVG_GRADIENT_RADIAL:
	{
		double cx, cy, r, fx, fy;
      
		_svg_android_length_to_pixel (svg_android, &gradient->u.radial.cx, &cx);
		_svg_android_length_to_pixel (svg_android, &gradient->u.radial.cy, &cy);
		_svg_android_length_to_pixel (svg_android, &gradient->u.radial.r, &r);
		_svg_android_length_to_pixel (svg_android, &gradient->u.radial.fx, &fx);
		_svg_android_length_to_pixel (svg_android, &gradient->u.radial.fy, &fy);

		gradient_shader = ANDROID_CREATE_RADIAL_GRADIENT(svg_android, fx, fy, r, colors_a, offsets_a, spreadType);
#if 0 // note here that there is a start and an end circel, android doesn't support that. The end circle in Android always have the same coords as the start circle
		pattern = cairo_pattern_create_radial (fx, fy, 0.0, cx, cy, r);
#endif
	} break;
	}    

	gradient_matrix = ANDROID_MATRIX_CREATE(svg_android,
						gradient->transform[0], gradient->transform[1],
						gradient->transform[2], gradient->transform[3],
						gradient->transform[4], gradient->transform[5]);
	ANDROID_MATRIX_MULTIPLY(svg_android, matrix, gradient_matrix);

	ANDROID_MATRIX_MULTIPLY(svg_android, matrix, svg_android->state->matrix);
	
	if(svg_android->fit_to_area)
		ANDROID_MATRIX_MULTIPLY(svg_android, matrix, svg_android->fit_to_MATRIX);

	if(gradient_shader) {
		ANDROID_SHADER_SET_MATRIX(svg_android, gradient_shader, matrix);
		ANDROID_PAINT_SET_SHADER(svg_android, gradient_shader);
	}
	
	svg_android->state->bbox = 0;
    
	return SVG_STATUS_SUCCESS;
}

svg_status_t _svg_android_set_color_and_alpha (svg_android_t *svg_android,
				  svg_color_t *color,
				  double alpha)
{
	if (color->is_current_color)
		color = &svg_android->state->color;

	alpha *= 255.0;
	ANDROID_PAINT_SET_SHADER(svg_android, NULL); // remove shader
	ANDROID_PAINT_SET_COLOR(svg_android,
				(int)(alpha),
				svg_color_get_red   (color),
				svg_color_get_green (color),
				svg_color_get_blue  (color)
		);

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t _svg_android_set_paint_and_opacity (svg_android_t *svg_android, svg_paint_t *paint, double opacity, svg_android_render_type_t type)
{
	svg_status_t status;

	opacity *= svg_android->state->opacity;
    
	switch (paint->type) {
	case SVG_PAINT_TYPE_NONE:
		break;
	case SVG_PAINT_TYPE_COLOR:
		status = _svg_android_set_color_and_alpha (svg_android,
							   &paint->p.color,
							   opacity);
		if (status)
			return status;
		break;
	case SVG_PAINT_TYPE_GRADIENT:
		status = _svg_android_set_gradient (svg_android, paint->p.gradient, type);
		if (status)
			return status;
		break;
	case SVG_PAINT_TYPE_PATTERN:
		status = _svg_android_set_pattern (svg_android, paint->p.pattern_element, type);
		if (status)
			return status;
		break;
	}

	if(type == SVG_ANDROID_RENDER_TYPE_FILL) {
		ANDROID_SET_PAINT_STYLE(svg_android, svg_android->state->paint, JNI_FALSE);
	} else {
		ANDROID_SET_PAINT_STYLE(svg_android, svg_android->state->paint, JNI_TRUE);
	}
	
	return SVG_STATUS_SUCCESS;
}

svg_status_t _svg_android_set_pattern (svg_android_t *svg_android,
				       svg_element_t *pattern_element,
				       svg_android_render_type_t type)
{
	svg_pattern_t *pattern = svg_element_pattern (pattern_element);
	jobject pattern_bitmap;
	jobject pattern_shader;
	double x_px, y_px, width_px, height_px;
	jobject path;

	_svg_android_length_to_pixel (svg_android, &pattern->x, &x_px);
	_svg_android_length_to_pixel (svg_android, &pattern->y, &y_px);
	_svg_android_length_to_pixel (svg_android, &pattern->width, &width_px);
	_svg_android_length_to_pixel (svg_android, &pattern->height, &height_px);

	/* OK. We've got the final path to be filled/stroked inside the
	 * android context right now. But we're also going to re-use that
	 * same context to draw the pattern. And since the path is no
	 * longer in the graphics state, android_save/restore will not help
	 * us here.
	 *
	 * Currently we deal with this by manually saving/restoring the
	 * path.
	 *
	 */
	path = svg_android->state->path;
	svg_android->state->path = ANDROID_PATH_CREATE(svg_android);
	ANDROID_SAVE(svg_android);

	pattern_bitmap = ANDROID_CREATE_BITMAP(svg_android,
					       (int) (width_px + 0.5),
					       (int) (height_px + 0.5));
	
	_svg_android_push_state (svg_android, pattern_bitmap, NULL);

	svg_android->state->matrix = ANDROID_IDENTITY_MATRIX(svg_android);
    
	svg_android->state->fill_paint.type = SVG_PAINT_TYPE_NONE;
	svg_android->state->stroke_paint.type = SVG_PAINT_TYPE_NONE;
    
	svg_element_render (pattern->group_element, &SVG_ANDROID_RENDER_ENGINE, svg_android);
	_svg_android_pop_state (svg_android);

	ANDROID_RESTORE(svg_android);

	svg_android->state->path = path ;

	pattern_shader = ANDROID_CREATE_BITMAP_SHADER(svg_android, pattern_bitmap);
	ANDROID_PAINT_SET_SHADER(svg_android, pattern_shader);
	
	return SVG_STATUS_SUCCESS;
}

svg_status_t _svg_android_select_font (svg_android_t *svg_android)
{
	char *family = svg_android->state->font_family;
	unsigned int font_weight = svg_android->state->font_weight;
	svg_font_style_t font_style = svg_android->state->font_style;

	int text_align = 0;
	int weight_n_slant = 0x0;
	
	if (! svg_android->state->font_dirty)
		return SVG_STATUS_SUCCESS;

	if (font_weight >= 700)
		weight_n_slant &= 0x2; // BOLD = bit 0 equal 0
	else
		weight_n_slant |= 0x1; // NO BOLD = bit 0 equal 1
	
	 // Android does not distinguish between italic and oblique.. Bad android.. BAD android!
	switch (font_style) {
	case SVG_FONT_STYLE_ITALIC:
	case SVG_FONT_STYLE_OBLIQUE:
		weight_n_slant &= 0x1; // ITALIC/OBLIQUE = bit 1 equal 0
		break;
	default:
		weight_n_slant |= 0x2; // Normal = bit 1 equal 1
		break;
	}

	switch(svg_android->state->text_anchor) {
	default:
	case SVG_TEXT_ANCHOR_START:
		text_align = 0;
		break;
	case SVG_TEXT_ANCHOR_MIDDLE:
		text_align = 1;
		break;
	case SVG_TEXT_ANCHOR_END:
		text_align = 2;
		break;
	}

	ANDROID_SET_TYPEFACE(svg_android, family, weight_n_slant, svg_android->state->font_size, text_align);
	
	svg_android->state->font_dirty = 0;

	return SVG_ANDROID_STATUS_SUCCESS;
}

void _svg_android_copy_canvas_state (svg_android_t *svg_android)
{
	_svg_android_set_fill_rule(svg_android, svg_android->state->fill_rule);
	_svg_android_set_stroke_width(svg_android, &(svg_android->state->width_len));
	_svg_android_set_stroke_line_cap(svg_android, svg_android->state->line_cap);
	_svg_android_set_stroke_line_join(svg_android, svg_android->state->line_join);
	_svg_android_set_stroke_miter_limit(svg_android, svg_android->state->miter_limit);

	svg_length_t offset_len;
	offset_len.unit = SVG_LENGTH_UNIT_PX;
	offset_len.value = svg_android->state->dash_offset;
	_svg_android_set_stroke_dash_offset(svg_android, &offset_len);
	
}

#define	DPI	100.0 // This is NOT what we want. How to reach global DPI? (Rob)

svg_status_t
_svg_android_length_to_pixel (svg_android_t * svg_android, svg_length_t *length, double *pixel)
{
	double width, height;

	switch (length->unit) {
	case SVG_LENGTH_UNIT_PX:
		*pixel = length->value;
		break;
	case SVG_LENGTH_UNIT_CM:
		*pixel = (length->value / 2.54) * DPI;
		break;
	case SVG_LENGTH_UNIT_MM:
		*pixel = (length->value / 25.4) * DPI;
		break;
	case SVG_LENGTH_UNIT_IN:
		*pixel = length->value * DPI;
		break;
	case SVG_LENGTH_UNIT_PT:
		*pixel = (length->value / 72.0) * DPI;
		break;
	case SVG_LENGTH_UNIT_PC:
		*pixel = (length->value / 6.0) * DPI;
		break;
	case SVG_LENGTH_UNIT_EM:
		*pixel = length->value * svg_android->state->font_size;
		break;
	case SVG_LENGTH_UNIT_EX:
		*pixel = length->value * svg_android->state->font_size / 2.0;
		break;
	case SVG_LENGTH_UNIT_PCT:
		if (svg_android->state->bbox) {
			width = 1.0;
			height = 1.0;
		} else {
			width = svg_android->state->viewport_width;
			height = svg_android->state->viewport_height;
		}
		if (length->orientation == SVG_LENGTH_ORIENTATION_HORIZONTAL)
			*pixel = (length->value / 100.0) * width;
		else if (length->orientation == SVG_LENGTH_ORIENTATION_VERTICAL)
			*pixel = (length->value / 100.0) * height;
		else
			*pixel = (length->value / 100.0) * sqrt(pow(width, 2) + pow(height, 2)) * sqrt(2);
		break;
	default:
		*pixel = length->value;
	}

	return SVG_STATUS_SUCCESS;
}
