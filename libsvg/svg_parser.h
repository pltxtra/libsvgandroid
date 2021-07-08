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

#pragma once

#include "svgint.h"
#include "svg_filter.h"

#ifdef __cplusplus
extern "C" {
#endif
	typedef struct svg_parser svg_parser_t;

	svg_status_t
	_svg_parser_new_group_element (svg_parser_t *parser,
				       svg_element_t **group_element,
				       svg_element_type_t type);

	svg_status_t
	_svg_parser_new_svg_group_element (svg_parser_t *parser, svg_element_t **group_element);

	svg_status_t
	_svg_parser_new_leaf_element (svg_parser_t *parser,
				      svg_element_t **child_element,
				      svg_element_type_t type);

	svg_status_t
	_svg_parser_parse_filter (svg_parser_t *parser,
				  const char **attributes,
				  svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feBlend (svg_parser_t *parser,
				   const char **attributes,
				   svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feColorMatrix (svg_parser_t *parser,
					 const char **attributes,
					 svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feComponentTransfer (svg_parser_t *parser,
					       const char **attributes,
					       svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feComposite (svg_parser_t *parser,
				       const char **attributes,
				       svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feConvolveMatrix (svg_parser_t *parser,
					    const char **attributes,
					    svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feDiffuseLighting (svg_parser_t *parser,
					     const char **attributes,
					     svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feDisplacementMap (svg_parser_t *parser,
					     const char **attributes,
					     svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feFlood (svg_parser_t *parser,
				   const char **attributes,
				   svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feGaussianBlur (svg_parser_t *parser,
					  const char **attributes,
					  svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feImage (svg_parser_t *parser,
				   const char **attributes,
				   svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feMerge (svg_parser_t *parser,
				   const char **attributes,
				   svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feMorphology (svg_parser_t *parser,
					const char **attributes,
					svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feOffset (svg_parser_t *parser,
				    const char **attributes,
				    svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feSpecularLightning (svg_parser_t *parser,
					       const char **attributes,
					       svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feTile (svg_parser_t *parser,
				  const char **attributes,
				  svg_element_t **not_used);
	svg_status_t
	_svg_parser_parse_feTurbulence (svg_parser_t *parser,
					const char **attributes,
					svg_element_t **not_used);


#ifdef __cplusplus
}
#endif
