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

#include "svg.h"
#include "strhmap_cc.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum {
		op_feBlend,
		op_feColorMatrix,
		op_feComponentTransfer,
		op_feComposite,
		op_feConvolveMatrix,
		op_feDiffuseLighting,
		op_feDisplacementMap,
		op_feFlood,
		op_feGaussianBlur,
		op_feImage,
		op_feMerge,
		op_feMorphology,
		op_feOffset,
		op_feSpecularLighting,
		op_feTile,
		op_feTurbulence
	} svg_filter_operation_t;

	typedef enum {
		in_SourceGraphic, in_SourceAlpha,
		in_BackgroundGraphic, in_BackgroundAlpha,
		in_FillPaint, in_StrokePaint,
		in_Reference
	} svg_filter_in_t;

	typedef enum {
		feBlend_normal, feBlend_multiply,
		feBlend_screen, feBlend_darken, feBlend_lighten
	} feBlendMode_t;

	struct feBlend {
		feBlendMode_t mode;
		svg_filter_in_t in2;
		struct svg_filter_primitive *in2_ref;
	};

	struct feColorMatrix {
		/* not supported */
	};

	struct feComponentTransfer {
		/* not supported */
	};

	typedef enum {
		feComposite_over, feComposite_in, feComposite_out,
		feComposite_atop, feComposite_xor, feComposite_arithmetic
	} feCompositeOperator_t;

	struct feComposite {
		feCompositeOperator_t oprt;
		svg_filter_in_t in2;
		struct svg_filter_primitive *in2_ref;
		double k1, k2, k3, k4; /* only applicable if operator = cm_arithmetic */
	};

	struct feConvolveMatrix {
		/* not supported */
	};

	struct feDiffuseLighting {
		/* not supported */
	};

	struct feDisplacementMap {
		/* not supported */
	};

	struct feFlood {
		svg_color_t color;
		double opacity;
	};

	struct feGaussianBlur {
		double std_dev_x, std_dev_y;
	};

	struct feImage {
		/* not supported */
	};

	struct feMerge {
		/* not supported */
	};

	struct feMorphology {
		/* not supported */
	};

	struct feOffset {
		double dx, dy;
	};

	struct feSpecularLighting {
		/* not supported */
	};

	struct feTile {
		/* not supported */
	};

	struct feTurbulence {
		/* not supported */
	};

	typedef struct svg_filter_primitive {
		int primitive_order; /* starting at first element: 0 */

		svg_filter_operation_t fe_operation;

		svg_length_t x, y, width, height;
		char *result;

		svg_filter_in_t in;
		struct svg_filter_primitive *in_ref;

		union {
			struct feBlend fe_blend;
			struct feColorMatrix fe_color_matrix;
			struct feComponentTransfer fe_component_transfer;
			struct feComposite fe_composite;
			struct feConvolveMatrix fe_convolve_matrix;
			struct feDiffuseLighting fe_diffuse_lighting;
			struct feDisplacementMap fe_displacement_map;
			struct feFlood fe_flood;
			struct feGaussianBlur fe_gaussian_blur;
			struct feImage fe_image;
			struct feMerge fe_merge;
			struct feMorphology fe_morphology;
			struct feOffset fe_offset;
			struct feSpecularLighting fe_specular_lighting;
			struct feTile fe_tile;
			struct feTurbulence fe_turbulence;
		} p;

		struct svg_filter_primitive *next;
	} svg_filter_primitive_t;

	typedef struct svg_filter {
		int number_of_primitives; /* no primitives == 0 */

		svg_filter_primitive_t *last_primitive;
		svg_filter_primitive_t *first_primitive;
		StrHmap* results;
	} svg_filter_t;


	svg_status_t _svg_filter_init(svg_filter_t *filter_element);
	svg_status_t _svg_filter_deinit(svg_filter_t *filter);
	svg_status_t _svg_filter_render(svg_filter_t* filter,
					svg_render_engine_t* engine,
					void* closure);

#ifdef __cplusplus
}
#endif
