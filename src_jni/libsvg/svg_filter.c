/*
 * svg_filter.h
 *
 * part of libsvgandroid
 *
 * Copyright 2015 by Anton Persson ( https://github.com/pltxtra/libsvgandroid )
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#include "svg_filter.h"
#include "svgint.h"
#include "svg_parser.h"

#define __DO_SVG_DEBUG
#include "svg_debug.h"

svg_status_t
_svg_filter_init(svg_filter_t *filter) {
	filter->results = StrHmapAlloc(12);
	if(filter->results == NULL) {
		return SVG_STATUS_NO_MEMORY;
	}
	filter->last_primitive = NULL;
	filter->first_primitive = NULL;
	filter->flag_dirty = 1; // we start off dirty..

	return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_filter_deinit(svg_filter_t *filter) {
	StrHmapFree(filter->results);
	return SVG_STATUS_SUCCESS;
}

static inline void _svg_filter_render_feBlend(
	svg_filter_t* filter,
	svg_filter_primitive_t* prim,
	svg_render_engine_t* engine,
	void* closure) {
	engine->add_filter_feBlend(
		closure,
		&prim->x, &prim->y,
		&prim->width, &prim->height,
		prim->in, prim->in_ref ? prim->in_ref->primitive_order : -1,
		prim->p.fe_blend.in2, prim->p.fe_blend.in2_ref ? prim->p.fe_blend.in2_ref->primitive_order : -1,
		prim->p.fe_blend.mode);
}

static inline void _svg_filter_render_feComposite(
	svg_filter_t* filter,
	svg_filter_primitive_t* prim,
	svg_render_engine_t* engine,
	void* closure) {
	engine->add_filter_feComposite(
		closure,
		&prim->x, &prim->y,
		&prim->width, &prim->height,
		prim->p.fe_composite.oprt,
		prim->in,
		prim->in_ref ? prim->in_ref->primitive_order : -1,
		prim->p.fe_composite.in2,
		prim->p.fe_composite.in2_ref ? prim->p.fe_composite.in2_ref->primitive_order : -1,
		prim->p.fe_composite.k1,
		prim->p.fe_composite.k2,
		prim->p.fe_composite.k3,
		prim->p.fe_composite.k4
		);
}

static inline void _svg_filter_render_feFlood(
	svg_filter_t* filter,
	svg_filter_primitive_t* prim,
	svg_render_engine_t* engine,
	void* closure) {
	SVG_DEBUG("will call engine->add_filter_feFlood() -- %p -> %p\n",
		  engine, engine->add_filter_feFlood);
	engine->add_filter_feFlood(
		closure,
		&prim->x, &prim->y,
		&prim->width, &prim->height,
		prim->in,
		prim->in_ref ? prim->in_ref->primitive_order : -1,
		&prim->p.fe_flood.color,
		prim->p.fe_flood.opacity
		);
}

static inline void _svg_filter_render_feGaussianBlur(
	svg_filter_t* filter,
	svg_filter_primitive_t* prim,
	svg_render_engine_t* engine,
	void* closure) {
	engine->add_filter_feGaussianBlur(
		closure,
		&prim->x, &prim->y,
		&prim->width, &prim->height,
		prim->in,
		prim->in_ref ? prim->in_ref->primitive_order : -1,
		prim->p.fe_gaussian_blur.std_dev_x,
		prim->p.fe_gaussian_blur.std_dev_y
		);
}

static inline void _svg_filter_render_feOffset(
	svg_filter_t* filter,
	svg_filter_primitive_t* prim,
	svg_render_engine_t* engine,
	void* closure) {
	engine->add_filter_feOffset(
		closure,
		&prim->x, &prim->y,
		&prim->width, &prim->height,
		prim->in,
		prim->in_ref ? prim->in_ref->primitive_order : -1,
		prim->p.fe_offset.dx,
		prim->p.fe_offset.dy
		);
}

svg_status_t
_svg_filter_render (svg_filter_t* filter,
		    svg_render_engine_t* engine,
		    void* closure) {
	SVG_DEBUG("_svg_filter_render() -- dirty? %d\n", filter->flag_dirty);
	if(filter->flag_dirty) {
		/* There has been a modification
		 * to this filter since the
		 * last render cycle - we need
		 * to update the engine.
		 */
		engine->begin_filter(closure, filter->element->id);

		svg_filter_primitive_t* prim = filter->first_primitive;

		SVG_DEBUG("_svg_filter_render() -- prim: %p\n", prim);
		while(prim != NULL) {

			SVG_DEBUG("_svg_filter_render() -- %p operation: %d\n",
				  prim, prim->fe_operation);
			switch(prim->fe_operation) {
			case op_feBlend:
				SVG_DEBUG("_svg_filter_render() -- feBlend\n");
				_svg_filter_render_feBlend(filter, prim, engine, closure);
				break;
			case op_feColorMatrix:
				SVG_DEBUG("_svg_filter_render() -- feColorMatrix\n");
				break;
			case op_feComponentTransfer:
				SVG_DEBUG("_svg_filter_render() -- feComponentTransfer\n");
				break;
			case op_feComposite:
				SVG_DEBUG("_svg_filter_render() -- feComposite\n");
				_svg_filter_render_feComposite(filter, prim, engine, closure);
				break;
			case op_feConvolveMatrix:
				SVG_DEBUG("_svg_filter_render() -- feConvolveMatrix\n");
				break;
			case op_feDiffuseLighting:
				SVG_DEBUG("_svg_filter_render() -- feDiffuseLighting\n");
				break;
			case op_feDisplacementMap:
				SVG_DEBUG("_svg_filter_render() -- feDisplacementMap\n");
				break;
			case op_feFlood:
				SVG_DEBUG("_svg_filter_render() -- feFlood\n");
				_svg_filter_render_feFlood(filter, prim, engine, closure);
				break;
			case op_feGaussianBlur:
				SVG_DEBUG("_svg_filter_render() -- feGaussianBlur\n");
				_svg_filter_render_feGaussianBlur(filter, prim, engine, closure);
				break;
			case op_feImage:
				SVG_DEBUG("_svg_filter_render() -- feImage\n");
				break;
			case op_feMerge:
				SVG_DEBUG("_svg_filter_render() -- feMerge\n");
				break;
			case op_feMorphology:
				SVG_DEBUG("_svg_filter_render() -- feMorphology\n");
				break;
			case op_feOffset:
				SVG_DEBUG("_svg_filter_render() -- feOffset\n");
				_svg_filter_render_feOffset(filter, prim, engine, closure);
				break;
			case op_feSpecularLighting:
				SVG_DEBUG("_svg_filter_render() -- feSpecularLighting\n");
				break;
			case op_feTile:
				SVG_DEBUG("_svg_filter_render() -- feTile\n");
				break;
			case op_feTurbulence:
				SVG_DEBUG("_svg_filter_render() -- feTurbulence\n");
				break;
			default:
				SVG_ERROR("_svg_filter_render() -- unknown filter operation.\n");
				break;
			}

			prim = prim->next;
		}

		/* unset the flag */
		filter->flag_dirty = 0;
	}

	return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_parser_parse_filter (svg_parser_t *parser,
			  const char **attributes,
			  svg_element_t **filter_element) {
	svg_status_t status;

	status = _svg_parser_new_leaf_element (parser, filter_element, SVG_ELEMENT_TYPE_FILTER);
	if (status)
		return status;

	/* The only thing that distinguishes a group from a leaf is that
	   the group becomes the new parent for future elements. */
	parser->state->filter_element = *filter_element;
	(*filter_element)->e.filter.element = (*filter_element);

	SVG_DEBUG("_svg_parser_parse_filter() created new Filter element (%p, %s, %d).\n",
		  (*filter_element), (*filter_element)->id, status);

	return status;
}

static void parse_filter_in(svg_element_t* filter_element,
			    const char** attributes, const char* in_attr,
			    svg_filter_in_t* in,
			    svg_filter_primitive_t** ref) {
	const char *in_str;

	/* set defaults */
	if(filter_element->e.filter.last_primitive) {
		*in = in_Reference;
		*ref = filter_element->e.filter.last_primitive;
	} else {
		*in = in_SourceGraphic;
		*ref = NULL;
	}

	if(_svg_attribute_get_string (attributes, in_attr, &in_str, NULL) ==
	   SVG_STATUS_SUCCESS &&
	   in_str != NULL) {
		if       (strcmp("SourceGraphic", in_str) == 0) {
			*in = in_SourceGraphic;
			*ref = NULL;
		} else if(strcmp("SourceAlpha", in_str) == 0) {
			*in = in_SourceAlpha;
			*ref = NULL;
		} else if(strcmp("BackgroundGraphic", in_str) == 0) {
			*in = in_BackgroundGraphic;
			*ref = NULL;
		} else if(strcmp("BackgroundAlpha", in_str) == 0) {
			*in = in_BackgroundAlpha;
			*ref = NULL;
		} else if(strcmp("FillPaint", in_str) == 0) {
			*in = in_FillPaint;
			*ref = NULL;
		} else if(strcmp("StrokePaint", in_str) == 0) {
			*in = in_StrokePaint;
			*ref = NULL;
		} else {
			svg_filter_primitive_t* fprim =
				StrHmapFind(filter_element->e.filter.results, in_str);
			if(fprim) {
				*in = in_Reference;
				*ref = fprim;
			}
		}
	}

	SVG_DEBUG("parse_filter_in() called.\n");
}

static svg_filter_primitive_t* parse_filter_primitive (svg_parser_t *parser,
						       const char **attributes,
						       svg_filter_operation_t op) {
	svg_element_t* filter_element = parser->state->filter_element;

	svg_filter_primitive_t* fprim =
		(svg_filter_primitive_t*)calloc(1, sizeof(svg_filter_primitive_t));
	if(fprim == NULL)
		return NULL;

	fprim->fe_operation = op;
	fprim->primitive_order = filter_element->e.filter.number_of_primitives++;

	_svg_attribute_get_length (attributes, "x", &(fprim->x), "0");
	_svg_attribute_get_length (attributes, "y", &(fprim->x), "0");
	_svg_attribute_get_length (attributes, "width", &(fprim->x), "0");
	_svg_attribute_get_length (attributes, "height", &(fprim->x), "0");

	const char *result_str;
	if(_svg_attribute_get_string (attributes, "result", &result_str, NULL) ==
	   SVG_STATUS_SUCCESS &&
	   result_str != NULL) {
		(void) StrHmapInsert(filter_element->e.filter.results, result_str, fprim);
	}

	parse_filter_in(filter_element, attributes, "in", &(fprim->in), &(fprim->in_ref));

	if(filter_element->e.filter.first_primitive == NULL) {
		filter_element->e.filter.first_primitive = fprim;
	}
	if(filter_element->e.filter.last_primitive != NULL) {
		filter_element->e.filter.last_primitive->next = fprim;
	}

	filter_element->e.filter.last_primitive = fprim;
	filter_element->e.filter.flag_dirty = 1;

	SVG_DEBUG("parse_filter_primitive() called.\n");

	return fprim;
}

svg_status_t
_svg_parser_parse_feBlend (svg_parser_t *parser,
			   const char **attributes,
			   svg_element_t **not_used) {
	svg_element_t* filter_element = parser->state->filter_element;

	/* create the filter primitive object */
	svg_filter_primitive_t* fprim = parse_filter_primitive(parser, attributes, op_feBlend);
	if(fprim == NULL)
		return SVG_STATUS_NO_MEMORY;

	/* read out the feBlend mode */
	feBlendMode_t mode = feBlend_normal;
	const char *feBlendMode_str;
	if(_svg_attribute_get_string (attributes, "mode", &feBlendMode_str, "normal") ==
	   SVG_STATUS_SUCCESS) {
		if(strcmp("normal", feBlendMode_str) == 0)
			mode = feBlend_normal;
		else if(strcmp("multiply", feBlendMode_str) == 0)
			mode = feBlend_multiply;
		else if(strcmp("screen", feBlendMode_str) == 0)
			mode = feBlend_screen;
		else if(strcmp("darken", feBlendMode_str) == 0)
			mode = feBlend_darken;
		else if(strcmp("lighten", feBlendMode_str) == 0)
			mode = feBlend_lighten;
	}
	fprim->p.fe_blend.mode = mode;

	/* get the in2 attribute */
	parse_filter_in(filter_element, attributes, "in2",
			&(fprim->p.fe_blend.in2),
			&(fprim->p.fe_blend.in2_ref));

	SVG_DEBUG("_svg_parser_parse_feBlend() called.\n");

	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feColorMatrix (svg_parser_t *parser,
				 const char **attributes,
				 svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feComponentTransfer (svg_parser_t *parser,
				       const char **attributes,
				       svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feComposite (svg_parser_t *parser,
			       const char **attributes,
			       svg_element_t **not_used) {
	svg_element_t* filter_element = parser->state->filter_element;

	/* create the filter primitive object */
	svg_filter_primitive_t* fprim = parse_filter_primitive(parser, attributes, op_feComposite);
	if(fprim == NULL)
		return SVG_STATUS_NO_MEMORY;

	/* read out the feComposite operator */
	feCompositeOperator_t oprt = feComposite_over;
	const char *feCompositeOperator_str;
	if(_svg_attribute_get_string (attributes, "operator", &feCompositeOperator_str, "over") ==
	   SVG_STATUS_SUCCESS) {
		if(strcmp("over", feCompositeOperator_str) == 0)
			oprt = feComposite_over;
		else if(strcmp("in", feCompositeOperator_str) == 0)
			oprt = feComposite_in;
		else if(strcmp("out", feCompositeOperator_str) == 0)
			oprt = feComposite_out;
		else if(strcmp("atop", feCompositeOperator_str) == 0)
			oprt = feComposite_atop;
		else if(strcmp("xor", feCompositeOperator_str) == 0)
			oprt = feComposite_xor;
		else if(strcmp("arithmetic", feCompositeOperator_str) == 0)
			oprt = feComposite_arithmetic;
	}
	fprim->p.fe_composite.oprt = oprt;

	/* get the in2 attribute */
	parse_filter_in(filter_element, attributes, "in2",
			&(fprim->p.fe_composite.in2),
			&(fprim->p.fe_composite.in2_ref));

	/* get the k values for arithmetic mode */
	if(oprt == feComposite_arithmetic) {
		_svg_attribute_get_double (attributes, "k1", &fprim->p.fe_composite.k1, 0);
		_svg_attribute_get_double (attributes, "k2", &fprim->p.fe_composite.k2, 0);
		_svg_attribute_get_double (attributes, "k3", &fprim->p.fe_composite.k3, 0);
		_svg_attribute_get_double (attributes, "k4", &fprim->p.fe_composite.k4, 0);
	}

	SVG_DEBUG("_svg_parser_parse_feComposite() called.\n");

	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feConvolveMatrix (svg_parser_t *parser,
				    const char **attributes,
				    svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feDiffuseLighting (svg_parser_t *parser,
				     const char **attributes,
				     svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feDisplacementMap (svg_parser_t *parser,
				     const char **attributes,
				     svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feFlood (svg_parser_t *parser,
			   const char **attributes,
			   svg_element_t **not_used) {
	/* create the filter primitive object */
	svg_filter_primitive_t* fprim = parse_filter_primitive(parser, attributes, op_feFlood);
	if(fprim == NULL)
		return SVG_STATUS_NO_MEMORY;

	/* get the flood attributes */
	const char* color_str;
	if (_svg_attribute_get_string (attributes, "flood-color", &color_str, "#000000") == SVG_STATUS_SUCCESS)
		_svg_color_init_from_str (&(fprim->p.fe_flood.color), color_str);

	_svg_attribute_get_double (attributes, "flood-opacity", &(fprim->p.fe_flood.opacity), 0);

	SVG_DEBUG("_svg_parser_parse_feFlood() called.\n");

	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feGaussianBlur (svg_parser_t *parser,
				  const char **attributes,
				  svg_element_t **not_used) {
	/* create the filter primitive object */
	svg_filter_primitive_t* fprim = parse_filter_primitive(parser, attributes, op_feGaussianBlur);
	if(fprim == NULL)
		return SVG_STATUS_NO_MEMORY;

	/* get the Gaussian attributes */
	const char* deviation_str;
	double d_x = 0.0, d_y = 0.0;
	if (_svg_attribute_get_string (attributes, "deviation", &deviation_str, "0") == SVG_STATUS_SUCCESS) {
		(void) sscanf(deviation_str, "%lf %lf", &d_x, &d_y);
	}

	fprim->p.fe_gaussian_blur.std_dev_x = d_x;
	fprim->p.fe_gaussian_blur.std_dev_y = d_y;

	SVG_DEBUG("_svg_parser_parse_feGaussianBlur() called.\n");

	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feImage (svg_parser_t *parser,
			   const char **attributes,
			   svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feMerge (svg_parser_t *parser,
			   const char **attributes,
			   svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feMorphology (svg_parser_t *parser,
				const char **attributes,
				svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feOffset (svg_parser_t *parser,
			    const char **attributes,
			    svg_element_t **not_used) {
	/* create the filter primitive object */
	svg_filter_primitive_t* fprim = parse_filter_primitive(parser, attributes, op_feOffset);
	if(fprim == NULL)
		return SVG_STATUS_NO_MEMORY;

	_svg_attribute_get_double (attributes, "dx", &(fprim->p.fe_offset.dx), 0);
	_svg_attribute_get_double (attributes, "dy", &(fprim->p.fe_offset.dy), 0);

	SVG_DEBUG("_svg_parser_parse_feOffset() called.\n");

	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feSpecularLightning (svg_parser_t *parser,
				       const char **attributes,
				       svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feTile (svg_parser_t *parser,
			  const char **attributes,
			  svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}

svg_status_t
_svg_parser_parse_feTurbulence (svg_parser_t *parser,
				const char **attributes,
				svg_element_t **not_used) {
	/* we don't treat this as a regular SVG element internally */
	return SVGINT_STATUS_UNKNOWN_ELEMENT;
}
