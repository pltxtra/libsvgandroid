/* svg_style.c: Data structure for holding SVG style properties

   Copyright © 2002 USC/Information Sciences Institute

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Carl Worth <cworth@isi.edu>
*/

#include <math.h>
#include <string.h>

//#define __DO_SVG_DEBUG
#include "svg_debug.h"

#include "svgint.h"

static svg_status_t
_svg_style_parse_color (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_fill_opacity (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_fill_paint (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_fill_rule (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_filter (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_font_family (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_font_size (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_font_style (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_font_weight (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_image_rendering (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_opacity (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_dash_array (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_dash_offset (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_line_cap (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_line_join (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_miter_limit (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_opacity (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_paint (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stroke_width (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_text_anchor (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_visibility (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stop_color (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_parse_stop_opacity (svg_style_t *style, const char *str);

static svg_status_t
_svg_style_str_to_opacity (const char *str, double *ret);

typedef struct svg_style_parse_map {
    const char	*name;
    svg_status_t 	(*parse) (svg_style_t *style, const char *value);
    const char		*default_value;
} svg_style_parse_map_t;

static const svg_style_parse_map_t SVG_STYLE_PARSE_MAP[] = {
/* XXX: { "clip-rule",		_svg_style_parse_clip_rule,		"nonzero" }, */
    { "color",			_svg_style_parse_color,			"black" },
/* XXX: { "color-interpolation",_svg_style_parse_color_interpolation,	"sRGB" }, */
/* XXX: { "color-interpolation-filters",_svg_style_parse_color_interpolation_filters,	"linearRGB" }, */
/* XXX: { "color-profile",	_svg_style_parse_color_profile,		"auto" }, */
/* XXX: { "color-rendering",	_svg_style_parse_color_rendering,	"auto" }, */
/* XXX: { "cursor",		_svg_style_parse_cursor,		"auto" }, */
/* XXX: { "direction",		_svg_style_parse_direction,		"ltr" }, */
    { "display",		_svg_style_parse_display,		"inline" },
    { "fill-opacity",		_svg_style_parse_fill_opacity,		"1.0" },
    { "fill",			_svg_style_parse_fill_paint,		"black" },
    { "fill-rule",		_svg_style_parse_fill_rule,		"nonzero" },
/* XXX: { "font",		_svg_style_parse_font,			NULL }, */
    { "filter",			_svg_style_parse_filter,		"none" },
    { "font-family",		_svg_style_parse_font_family,		"sans-serif" },
    /* XXX: The default is supposed to be "medium" but I'm not parsing that yet */
    { "font-size",		_svg_style_parse_font_size,		"10.0" },
/* XXX: { "font-size-adjust",	_svg_style_parse_font_size_adjust,	"none" }, */
/* XXX: { "font-stretch",	_svg_style_parse_font_stretch,		"normal" }, */
    { "font-style",		_svg_style_parse_font_style,		"normal" },
/* XXX: { "font-variant",	_svg_style_parse_font_variant,		"normal" }, */
    { "font-weight",		_svg_style_parse_font_weight,		"normal" },
/* XXX: { "glyph-orientation-horizontal",	_svg_style_parse_glyph_orientation_horizontal,	"0deg" }, */
/* XXX: { "glyph-orientation-vertical",		_svg_style_parse_glyph_orientation_vertical,	"auto" }, */
    { "image-rendering",	_svg_style_parse_image_rendering,	"auto" },
/* XXX: { "kerning",		_svg_style_parse_kerning,		"auto" }, */
/* XXX: { "letter-spacing",	_svg_style_parse_letter_spacing,	"normal" }, */
/* XXX: { "marker",		_svg_style_parse_marker,		NULL }, */
/* XXX: { "marker-end",		_svg_style_parse_marker_end,		"none" }, */
/* XXX: { "marker-mid",		_svg_style_parse_marker_mid,		"none" }, */
/* XXX: { "marker-start",	_svg_style_parse_marker_start,		"none" }, */
    { "opacity",		_svg_style_parse_opacity,		"1.0" },
/* XXX: { "pointer-events",	_svg_style_parse_pointer_events,	"visiblePainted" }, */
/* XXX: { "shape-rendering",	_svg_style_parse_shape_rendering,	"auto" }, */
    { "stroke-dasharray",	_svg_style_parse_stroke_dash_array,	"none" },
    { "stroke-dashoffset",	_svg_style_parse_stroke_dash_offset,	"0.0" },
    { "stroke-linecap",		_svg_style_parse_stroke_line_cap,	"butt" },
    { "stroke-linejoin",	_svg_style_parse_stroke_line_join,	"miter" },
    { "stroke-miterlimit",	_svg_style_parse_stroke_miter_limit,	"4.0" },
    { "stroke-opacity",		_svg_style_parse_stroke_opacity,	"1.0" },
    { "stroke",			_svg_style_parse_stroke_paint,		"none" },
    { "stroke-width",		_svg_style_parse_stroke_width,		"1.0" },
    { "text-anchor",		_svg_style_parse_text_anchor,		"start" },
/* XXX: { "text-rendering",	_svg_style_parse_text_rendering,	"auto" }, */
    { "visibility",		_svg_style_parse_visibility,		"visible" },
/* XXX: { "word-spacing",	_svg_style_parse_word_spacing,		"normal" }, */
/* XXX: { "writing-mode",	_svg_style_parse_writing_mode,		"lr-tb" }, */
    { "stop-opacity",		_svg_style_parse_stop_opacity,			"1.0" },
    { "stop-color",		_svg_style_parse_stop_color,			"#ffffff" },
};

svg_status_t
_svg_style_init_empty (svg_style_t *style, svg_t *svg)
{
    style->svg = svg;
    style->flags = SVG_STYLE_FLAG_NONE;
    style->font_family = NULL;
    _svg_length_init_from_str (&style->font_size, "10px");
    style->num_dashes = 0;
    style->stroke_dash_array = NULL;
    style->stroke_dash_offset.value = 0;
    /* initialize unused elements so copies are predictable */
    style->stroke_line_cap = SVG_STROKE_LINE_CAP_BUTT;
    style->stroke_line_join = SVG_STROKE_LINE_JOIN_MITER;
    style->stroke_miter_limit = 4.0;
    style->stroke_opacity = 1.0;
    style->fill_opacity = 1.0;
    style->filter_element = NULL;

    /* opacity is not inherited */
    style->flags |= SVG_STYLE_FLAG_OPACITY;
    style->opacity = 1.0;

    style->flags |= SVG_STYLE_FLAG_VISIBILITY;
    style->flags |= SVG_STYLE_FLAG_DISPLAY;

    return SVG_STATUS_SUCCESS;
}

svg_status_t _svg_style_init_copy (svg_style_t *style, svg_style_t *other) {
	style->svg = other->svg;

	style->flags = other->flags;

	style->fill_opacity = other->fill_opacity;
	style->fill_paint = other->fill_paint;
	style->fill_rule = other->fill_rule;

	if (other->font_family) {
		style->font_family = strdup (other->font_family);
		if (style->font_family == NULL)
			return SVG_STATUS_NO_MEMORY;
	} else {
		style->font_family = NULL;
	}

	style->font_size = other->font_size;
	style->font_style = other->font_style;
	style->font_weight = other->font_weight;

	style->opacity = other->opacity;

	style->num_dashes = other->num_dashes;
	if (style->num_dashes) {
		style->stroke_dash_array = malloc (style->num_dashes * sizeof (double));
		if (style->stroke_dash_array == NULL)
			return SVG_STATUS_NO_MEMORY;
		memcpy (style->stroke_dash_array, other->stroke_dash_array,
			style->num_dashes * sizeof (double));
	} else {
		style->stroke_dash_array = NULL;
	}
	style->stroke_dash_offset = other->stroke_dash_offset;

	style->stroke_line_cap = other->stroke_line_cap;
	style->stroke_line_join = other->stroke_line_join;
	style->stroke_miter_limit = other->stroke_miter_limit;
	style->stroke_opacity = other->stroke_opacity;
	style->stroke_paint = other->stroke_paint;
	style->stroke_width = other->stroke_width;

	style->color = other->color;
	style->text_anchor = other->text_anchor;

	return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_init_defaults (svg_style_t *style, svg_t *svg)
{
    int i;
    svg_status_t status;

    style->svg = svg;

    for (i=0; i < SVG_ARRAY_SIZE(SVG_STYLE_PARSE_MAP); i++) {
	const svg_style_parse_map_t *map;
	map = &SVG_STYLE_PARSE_MAP[i];

	if (map->default_value) {
	    status = (map->parse) (style, map->default_value);
	    if (status)
		return status;
	}
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_deinit (svg_style_t *style)
{
    if (style->font_family)
	free (style->font_family);
    style->font_family = NULL;

    if (style->stroke_dash_array)
	free (style->stroke_dash_array);
    style->stroke_dash_array = NULL;
    style->num_dashes = 0;

    style->flags = SVG_STYLE_FLAG_NONE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_str_to_opacity (const char *str, double *ret)
{
    const char *end_ptr;
    double opacity;

    opacity = _svg_ascii_strtod (str, &end_ptr);

    if (end_ptr == str)
	return SVG_STATUS_PARSE_ERROR;

    if (end_ptr && end_ptr[0] == '%')
	opacity *= 0.01;

    *ret = opacity;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_color (svg_style_t *style, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0)
	return SVG_STATUS_SUCCESS;

    status = _svg_color_init_from_str (&style->color, str);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_COLOR;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_fill_opacity (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_style_str_to_opacity (str, &style->fill_opacity);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_FILL_OPACITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_fill_paint (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_paint_init (&style->fill_paint, style->svg, str);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_FILL_PAINT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_fill_rule (svg_style_t *style, const char *str)
{
    if (strcmp (str, "evenodd") == 0)
	style->fill_rule = SVG_FILL_RULE_EVEN_ODD;
    else if (strcmp (str, "nonzero") == 0)
	style->fill_rule = SVG_FILL_RULE_NONZERO;
    else
	/* XXX: Check SVG spec. for error name conventions */
	return SVG_STATUS_PARSE_ERROR;

    style->flags |= SVG_STYLE_FLAG_FILL_RULE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_filter (svg_style_t *style, const char *str)
{
    svg_status_t stat = SVG_STATUS_SUCCESS;

    if(strcmp ("none", str) == 0)
	return SVG_STATUS_SUCCESS;

    style->flags |= SVG_STYLE_FLAG_FILTER;

    SVG_ERROR("_svg_style_parse_filter(%s)\n", str);

    if(strcmp ("inherit", str) == 0)
	style->filter_element = NULL;
    else {
	char bfr[strlen(str) + 1];
	memcpy(bfr, str, sizeof(bfr));
	char* end = strchr(bfr,  ')');
	if(strncmp("url(#", bfr, 5) != 0 || end == NULL)
	    return SVG_STATUS_PARSE_ERROR;

	SVG_ERROR("_svg_style_parse_filter()  --> %s\n", bfr);

	char *id = &bfr[5]; // 5 is the char after the #
	end[0] = '\0'; // swap the ) for null termination

	SVG_ERROR("_svg_style_parse_filter()  --> 2 %s\n", id);

	stat = _svg_fetch_element_by_id (style->svg, id, &(style->filter_element));
	if(!(!stat && style->filter_element != NULL && style->filter_element->type == SVG_ELEMENT_TYPE_FILTER)) {
	    style->flags &= ~SVG_STYLE_FLAG_FILTER;
	    style->filter_element = NULL;
	}
	SVG_ERROR("_svg_style_parse_filter()  --> 3 complete -- %p\n", style);
    }

    return stat;
}

static svg_status_t
_svg_style_parse_font_family (svg_style_t *style, const char *str)
{
    free (style->font_family);
    style->font_family = strdup (str);
    if (style->font_family == NULL)
	return SVG_STATUS_NO_MEMORY;

    style->flags |= SVG_STYLE_FLAG_FONT_FAMILY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_font_size (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_length_init_from_str (&style->font_size, str);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_FONT_SIZE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_font_style (svg_style_t *style, const char *str)
{
    if (strcmp (str, "normal") == 0)
	style->font_style = SVG_FONT_STYLE_NORMAL;
    else if (strcmp (str, "italic") == 0)
	style->font_style = SVG_FONT_STYLE_ITALIC;
    else if (strcmp (str, "oblique") == 0)
	style->font_style = SVG_FONT_STYLE_OBLIQUE;
    else
	return SVG_STATUS_PARSE_ERROR;

    style->flags |= SVG_STYLE_FLAG_FONT_STYLE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_font_weight (svg_style_t *style, const char *str)
{
    if (strcmp (str, "normal") == 0)
	style->font_weight = 400;
    else if (strcmp (str, "bold") == 0)
	style->font_weight = 700;
    else if (strcmp (str, "lighter") == 0)
	style->font_weight -= 100;
    else if (strcmp (str, "bolder") ==0)
	style->font_weight += 100;
    else
	style->font_weight = _svg_ascii_strtod(str, NULL);

    if (style->font_weight < 100)
	style->font_weight = 100;
    if (style->font_weight > 900)
	style->font_weight = 900;

    style->flags |= SVG_STYLE_FLAG_FONT_WEIGHT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_image_rendering (svg_style_t *style, const char *str)
{
	if (strcmp (str,  "optimizeSpeed") == 0)
		style->image_rendering == SVG_IMAGERENDER_SPEED;
	else if (strcmp (str,  "optimizeQuality") == 0)
		style->image_rendering == SVG_IMAGERENDER_QUALITY;
	else if (strcmp (str,  "inherit") == 0)
		style->image_rendering == SVG_IMAGERENDER_INHERIT;
	else
		style->image_rendering == SVG_IMAGERENDER_AUTO;

	style->flags |= SVG_STYLE_FLAG_IMAGE_RENDERING;

	return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_opacity (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_style_str_to_opacity (str, &style->opacity);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_OPACITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_dash_array (svg_style_t *style, const char *str)
{
    svgint_status_t status;
    double *new_dash_array;
    const char *end;
    int i, j;

    free (style->stroke_dash_array);
    style->num_dashes = 0;

    if(strcmp (str, "none") == 0) {
	style->flags |= SVG_STYLE_FLAG_STROKE_DASH_ARRAY;
	return SVG_STATUS_SUCCESS;
    }

    status = _svg_str_parse_all_csv_doubles (str, &style->stroke_dash_array, &style->num_dashes, &end);
    if (status)
	return status;

    if (style->num_dashes % 2) {
	style->num_dashes *= 2;

	new_dash_array = realloc(style->stroke_dash_array, style->num_dashes * sizeof(double));
	if (new_dash_array == NULL)
	    return SVG_STATUS_NO_MEMORY;
	style->stroke_dash_array = new_dash_array;

	for (i=0, j=style->num_dashes / 2; j < style->num_dashes; i++, j++)
	    style->stroke_dash_array[j] = style->stroke_dash_array[i];
    }

    style->flags |= SVG_STYLE_FLAG_STROKE_DASH_ARRAY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_dash_offset (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_length_init_from_str (&style->stroke_dash_offset, str);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_STROKE_DASH_OFFSET;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_line_cap (svg_style_t *style, const char *str)
{
    if (strcmp (str, "butt") == 0)
	style->stroke_line_cap = SVG_STROKE_LINE_CAP_BUTT;
    else if (strcmp (str, "round") == 0)
	style->stroke_line_cap = SVG_STROKE_LINE_CAP_ROUND;
    else if (strcmp (str, "square") == 0)
	style->stroke_line_cap = SVG_STROKE_LINE_CAP_SQUARE;
    else
	/* XXX: Check SVG spec. for error name conventions */
	return SVG_STATUS_PARSE_ERROR;

    style->flags |= SVG_STYLE_FLAG_STROKE_LINE_CAP;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_line_join (svg_style_t *style, const char *str)
{
    if (strcmp (str, "miter") == 0)
	style->stroke_line_join = SVG_STROKE_LINE_JOIN_MITER;
    else if (strcmp (str, "round") == 0)
	style->stroke_line_join = SVG_STROKE_LINE_JOIN_ROUND;
    else if (strcmp (str, "bevel") == 0)
	style->stroke_line_join = SVG_STROKE_LINE_JOIN_BEVEL;
    else
	/* XXX: Check SVG spec. for error name conventions */
	return SVG_STATUS_PARSE_ERROR;

    style->flags |= SVG_STYLE_FLAG_STROKE_LINE_JOIN;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_miter_limit (svg_style_t *style, const char *str)
{
    const char *end;

    style->stroke_miter_limit = _svg_ascii_strtod (str, &end);
    if (end == (char *)str)
	return SVG_STATUS_PARSE_ERROR;

    style->flags |= SVG_STYLE_FLAG_STROKE_MITER_LIMIT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_opacity (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status =_svg_style_str_to_opacity (str, &style->stroke_opacity);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_STROKE_OPACITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_paint (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_paint_init (&style->stroke_paint, style->svg, str);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_STROKE_PAINT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_width (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_length_init_from_str (&style->stroke_width, str);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_STROKE_WIDTH;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_text_anchor (svg_style_t *style, const char *str)
{
    if (strcmp (str, "start") == 0)
	style->text_anchor = SVG_TEXT_ANCHOR_START;
    else if (strcmp (str, "middle") == 0)
	style->text_anchor = SVG_TEXT_ANCHOR_MIDDLE;
    else if (strcmp (str, "end") == 0)
	style->text_anchor = SVG_TEXT_ANCHOR_END;
    else
	return SVG_STATUS_PARSE_ERROR;

    style->flags |= SVG_STYLE_FLAG_TEXT_ANCHOR;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_visibility (svg_style_t *style, const char *str)
{
    /* XXX: Do we care about the CSS2 definitions for these? */
    if (strcmp (str, "hidden") == 0 || strcmp (str, "collapse") == 0)
	style->flags &= ~SVG_STYLE_FLAG_VISIBILITY;
    else if (strcmp (str, "visible") == 0)
	style->flags |= SVG_STYLE_FLAG_VISIBILITY;
    else if (strcmp (str, "inherit") != 0)
	return SVG_STATUS_PARSE_ERROR;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_parse_display (svg_style_t *style, const char *str)
{
    /* XXX: Do we care about the CSS2 definitions for these? */
    if (strcmp (str, "none") == 0)
	style->flags &= ~SVG_STYLE_FLAG_DISPLAY;
    else if (strcmp (str, "inline") == 0 || strcmp (str, "block") == 0 ||
	     strcmp (str, "list-item") == 0 || strcmp (str, "run-in") == 0 ||
	     strcmp (str, "compact") == 0 || strcmp (str, "marker") == 0 ||
	     strcmp (str, "table") == 0 || strcmp (str, "inline-table") == 0 ||
	     strcmp (str, "table-row-group") == 0 || strcmp (str, "table-header-group") == 0 ||
	     strcmp (str, "table-footer-group") == 0 || strcmp (str, "table-row") == 0 ||
	     strcmp (str, "table-column-group") == 0 || strcmp (str, "table-column") == 0 ||
	     strcmp (str, "table-cell") == 0 || strcmp (str, "table-caption") == 0)
	style->flags |= SVG_STYLE_FLAG_DISPLAY;
    else if (strcmp (str, "inherit") != 0)
	return SVG_STATUS_PARSE_ERROR;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stop_color (svg_style_t *style, const char *str)
{
    svg_status_t status;

    status = _svg_color_init_from_str (&style->color, str);
    if (status)
	return status;

    style->flags |= SVG_STYLE_FLAG_COLOR;

    return SVG_STATUS_SUCCESS;
}


static svg_status_t
_svg_style_parse_stop_opacity (svg_style_t *style, const char *str)
{
    svg_status_t status;
    double opacity = 1.0;

    status = _svg_style_str_to_opacity (str, &opacity);
    if (status)
	return status;

	style->opacity = opacity;
    style->flags |= SVG_STYLE_FLAG_OPACITY;

    return SVG_STATUS_SUCCESS;
}


static svg_status_t
_svg_style_split_nv_pair (char	*nv_pair,
			  char	**name,
			  char	**value)
{
    char *colon;

    *name = nv_pair;

    colon = strchr (nv_pair, ':');
    if (colon == NULL) {
	*name = NULL;
	*value = NULL;
	return SVG_STATUS_PARSE_ERROR;
    }

    *colon = '\0';
    colon++;

    // skip leading white space
    while (_svg_ascii_isspace (*colon))
	colon++;

    if (colon == NULL) {
	    *name = NULL;
	    *value = NULL;
	    return SVG_STATUS_NO_MEMORY;
    }

    *value = colon;

    return SVG_STATUS_SUCCESS;
}

/* Parse a CSS2 style argument */
static svg_status_t
_svg_style_parse_nv_pair (svg_style_t	*style,
			  char	        *nv_pair)
{
    unsigned int i;
    char *name, *value;
    svg_status_t status;

    status = _svg_style_split_nv_pair (nv_pair, &name, &value);
    if (status)
	return status;

    /* guilty until proven innocent */
    /* XXX: Check SVG spec. for this error condition */
    status = SVG_STATUS_PARSE_ERROR;

    for (i=0; i < SVG_ARRAY_SIZE(SVG_STYLE_PARSE_MAP); i++) {
	if (strcmp (SVG_STYLE_PARSE_MAP[i].name, name) == 0) {
	    status = (SVG_STYLE_PARSE_MAP[i].parse) (style, value);
	    break;
	}
    }

    return status;
}

/* This next function is:

   // completely rewritten
   Copyright 2014 by Anton Persson

   // original function
   Copyright © 2000 Eazel, Inc.
   Copyright © 2002 Dom Lachowicz <cinamod@hotmail.com>
   Copyright © 2002 USC/Information Sciences Institute

   Author: Raph Levien <raph@artofcode.com>
*/
/* Parse a complete CSS2 style string into individual name/value
   pairs.

   XXX: It's known that this is _way_ out of spec. A more complete
   CSS2 implementation will happen later.
*/
svg_status_t
_svg_style_parse_style_str (svg_style_t	*style,
			    const char	*str_orig)
{
    char *nv_pair, *str, *next_token;

    // to reduce the numbmer of malloc/free callse
    // we just strdup the entire string and free
    // it once. /Anton
    str = strdup(str_orig);
    if(str == NULL) return SVG_STATUS_NO_MEMORY;

    // then we can split the internally allocated string
    // into substrings containing the nv_pairs. /Anton
    next_token = str;
    while(next_token != NULL && *next_token != '\0') {
	    // skip leading whitespace
	    while (_svg_ascii_isspace (*next_token))
		    next_token++;

	    nv_pair = next_token;
	    next_token = strchr (next_token, ';');
	    if(next_token) {
		    *next_token = '\0';
		    next_token++;
	    }
	    if(nv_pair)
		    _svg_style_parse_nv_pair(style, nv_pair);
    }
    free(str);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_apply_attributes (svg_style_t	*style,
			     const char		**attributes)
{
    unsigned int i;
    svg_status_t status;
    const char *style_str, *str;

    _svg_attribute_get_string (attributes, "style", &style_str, NULL);

    if (style_str) {
	status = _svg_style_parse_style_str (style, style_str);
	if (status)
	    return status;
    }

    for (i=0; i < SVG_ARRAY_SIZE(SVG_STYLE_PARSE_MAP); i++) {
	const svg_style_parse_map_t *map;
	map = &SVG_STYLE_PARSE_MAP[i];

	_svg_attribute_get_string (attributes, map->name, &str, NULL);

	if (str) {
	    status = (map->parse) (style, str);
	    if (status)
		return status;
	}
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_render (svg_style_t		*style,
		   svg_render_engine_t	*engine,
		   void			*closure)
{
    svg_status_t status;

    if (style->flags & SVG_STYLE_FLAG_COLOR) {
	status = (engine->set_color) (closure, &style->color);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FILL_OPACITY) {
	status = (engine->set_fill_opacity) (closure, style->fill_opacity);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FILL_PAINT) {
			status = (engine->set_fill_paint) (closure, &style->fill_paint);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FILL_RULE) {
	status = (engine->set_fill_rule) (closure, style->fill_rule);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FONT_FAMILY) {
	status = (engine->set_font_family) (closure, style->font_family);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FONT_SIZE) {
	/* XXX: How to deal with units of svg_length_t ? */
	status = (engine->set_font_size) (closure, style->font_size.value);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FONT_STYLE) {
	status = (engine->set_font_style) (closure, style->font_style);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FONT_WEIGHT) {
	status = (engine->set_font_weight) (closure, style->font_weight);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_OPACITY) {
	status = (engine->set_opacity) (closure, style->opacity);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FILTER) {
	const char* flt = "inherit";

	SVG_ERROR("_svg_style_render(FILTER) - %p\n", style->filter_element);

	if(style->filter_element) {
	    SVG_ERROR("_svg_style_render(FILTER) - %s\n", style->filter_element->id);
	    status = _svg_filter_render (&(style->filter_element->e.filter), engine, closure);
	    SVG_ERROR("_svg_style_render(FILTER) - status %d\n", status);
	    if (status)
		return status;
	    flt = style->filter_element->id;
	}
	SVG_ERROR("_svg_style_render(FILTER) - flt %s\n", flt);

	status = (engine->set_filter) (closure, flt);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_DASH_ARRAY) {
	/* XXX: How to deal with units of svg_length_t ? */
	status = (engine->set_stroke_dash_array) (closure, style->stroke_dash_array, style->num_dashes);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_DASH_OFFSET) {
	status = (engine->set_stroke_dash_offset) (closure, &style->stroke_dash_offset);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_LINE_CAP) {
	status = (engine->set_stroke_line_cap) (closure, style->stroke_line_cap);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_LINE_JOIN) {
	status = (engine->set_stroke_line_join) (closure, style->stroke_line_join);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_MITER_LIMIT) {
	status = (engine->set_stroke_miter_limit) (closure, style->stroke_miter_limit);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_OPACITY) {
	status = (engine->set_stroke_opacity) (closure, style->stroke_opacity);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_PAINT) {
	status = (engine->set_stroke_paint) (closure, &style->stroke_paint);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_WIDTH) {
	status = (engine->set_stroke_width) (closure, &style->stroke_width);
	if (status)
	    return status;
    }

    if (style->flags & SVG_STYLE_FLAG_TEXT_ANCHOR) {
	status = (engine->set_text_anchor) (closure, style->text_anchor);
	if (status)
	    return status;
    }

    return SVG_STATUS_SUCCESS;
}

double
_svg_style_get_opacity (svg_style_t *style)
{
    return style->opacity;
}

svg_status_t
_svg_style_get_display (svg_style_t *style)
{
    if (style->flags & SVG_STYLE_FLAG_DISPLAY) {
	    return SVG_STATUS_SUCCESS;
    }

    return SVG_STATUS_INVALID_VALUE;
}

svg_status_t
_svg_style_get_visibility (svg_style_t *style)
{
    if (style->flags & SVG_STYLE_FLAG_VISIBILITY)
	return SVG_STATUS_SUCCESS;
    else
	return SVG_STATUS_INVALID_VALUE;
}
