/* libsvg-android - Render SVG documents to an Android canvas
 *
 * Copyright © 2002 University of Southern California
 * Copyright © 2016 Anton Persson
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

#include "svg-android-internal.h"

#define __DO_SVG_ANDROID_DEBUG
#include "svg_android_debug.h"

static int prep_filter(JNIEnv* env, svg_android_t* svg_android) {
	svg_android->filter_clazz =
		(*env)->FindClass(env, "com/toolkits/libsvgandroid/Filter");

	if(svg_android->filter_clazz == NULL) {
		SVG_ANDROID_ERROR("svg_android_filter.c:prep_filter() - could not get Filter class.\n");
		return -1;
	}

	svg_android->create_filter = (*env)->GetStaticMethodID(
		env,
		svg_android->filter_clazz, "createFilter",
		"()Lcom/toolkits/libsvgandroid/Filter;"
		);

	if(svg_android->create_filter == NULL) {
		SVG_ANDROID_ERROR("svg_android_filter.c:prep_filter() - could not get Filter::createFilter().\n");
		return -1;
	}

	svg_android->begin_filter = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "beginFilter",
		"(Ljava/lang/String;)V"
		);

	if(svg_android->begin_filter == NULL) {
		SVG_ANDROID_ERROR("svg_android_filter.c:prep_filter() - could not get Filter::beginFilter().\n");
		return -1;
	}

	svg_android->set_filter = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "setFilter",
		"(Ljava/lang/String;)V"
		);

	if(svg_android->set_filter == NULL) {
		SVG_ANDROID_ERROR("svg_android_filter.c:prep_filter() - could not get Filter::setFilter().\n");
		return -1;
	}

	svg_android->add_filter_feBlend = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "addFilter_feBlend",
		"(IIIIIII)V"
		);

	if(svg_android->add_filter_feBlend == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not get Filter::addFilter_feBlend().\n");
		return -1;
	}

	svg_android->add_filter_feComposite = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "addFilter_feComposite",
		"(IIIIIIIDDDD)V"
		);

	if(svg_android->add_filter_feComposite == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not get Filter::addFilter_feComposite().\n");
		return -1;
	}

	svg_android->add_filter_feFlood = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "addFilter_feFlood",
		"(IIIIIID)V"
		);

	if(svg_android->add_filter_feFlood == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not get Filter::addFilter_feFlood().\n");
		return -1;
	}

	svg_android->add_filter_feGaussianBlur = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "addFilter_feGaussianBlur",
		"(IIIIIDD)V"
		);

	if(svg_android->add_filter_feGaussianBlur == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not get Filter::addFilter_feGaussianBlur().\n");
		return -1;
	}

	svg_android->add_filter_feOffset = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "addFilter_feOffset",
		"(IIIIIDD)V"
		);

	if(svg_android->add_filter_feOffset == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not get Filter::addFilter_feOffset().\n");
		return -1;
	}

	svg_android->filter_execute = (*env)->GetMethodID(
		env,
		svg_android->filter_clazz, "execute",
		"(Landroid/graphics/Canvas;Landroid/graphics/Bitmap;)V");

	if(svg_android->filter_execute == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not get Filter::execute().\n");
		return -1;
	}

	svg_android->filter = (*env)->CallStaticObjectMethod(env,
							     svg_android->filter_clazz,
							     svg_android->create_filter);

	if(svg_android->filter == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not create filter object.\n");
		return -1;
	}

	svg_android->filter = (jobject)((*env)->NewGlobalRef(env, (jobject)svg_android->filter));

	if(svg_android->filter == NULL) {
		SVG_ANDROID_ERROR(
			"svg_android_filter.c:prep_filter() - could not create global reference for object.\n");
		return -1;
	}

	return 0;
}

svg_status_t _svg_android_set_filter (void *closure, const char* id) {
	svg_android_t* svg_android = closure;
	JNIEnv* env = svg_android->env;

	if(svg_android->filter == NULL && prep_filter(env, svg_android))
		return SVG_ANDROID_STATUS_NO_MEMORY;

	SVG_ANDROID_DEBUG("_svg_android_set_filter(%p, %s)\n", closure, id);
	SVG_ANDROID_DEBUG(" --- _svg_android_add_filter_feFlood = %p\n",
			  _svg_android_add_filter_feFlood);

	(*env)->CallVoidMethod(env,
			       svg_android->filter,
			       svg_android->set_filter,
			       (*env)->NewStringUTF(env, id));

	_svg_android_prepare_filter(svg_android);

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t _svg_android_begin_filter (void *closure, const char* id) {
	svg_android_t* svg_android = closure;
	JNIEnv* env = svg_android->env;

	if(svg_android->filter == NULL && prep_filter(env, svg_android))
		return SVG_ANDROID_STATUS_NO_MEMORY;

	(*env)->CallVoidMethod(env,
			       svg_android->filter,
			       svg_android->begin_filter,
			       (*env)->NewStringUTF(env, id));

	return SVG_ANDROID_STATUS_SUCCESS;
}

static inline int in2int(svg_filter_in_t in, int in_op_reference) {
	int retv = 0;

	switch(in) {
	case in_SourceGraphic:
		retv = -1;
		break;
	case in_SourceAlpha:
		retv = -2;
		break;
	case in_BackgroundGraphic:
		retv = -3;
		break;
	case in_BackgroundAlpha:
		retv = -4;
		break;
	case in_FillPaint:
		retv = -5;
		break;
	case in_StrokePaint:
		retv = -6;
		break;
	case in_Reference:
		retv = in_op_reference;
		break;
	}

	return retv;
}

svg_status_t _svg_android_add_filter_feBlend (void *closure,
					      svg_length_t* x, svg_length_t* y,
					      svg_length_t* width, svg_length_t* height,
					      svg_filter_in_t in, int in_op_reference,
					      svg_filter_in_t in2, int in2_op_reference,
					      feBlendMode_t mode) {
	svg_android_t* svg_android = closure;
	JNIEnv* env = svg_android->env;

	SVG_ANDROID_DEBUG("_svg_android_add_filter_feBlend()\n");

	if(svg_android->filter == NULL && prep_filter(env, svg_android))
		return SVG_ANDROID_STATUS_NO_MEMORY;

	double __x, __y, __w, __h;
	_svg_android_length_to_pixel (svg_android, x, &__x);
	_svg_android_length_to_pixel (svg_android, y, &__y);
	_svg_android_length_to_pixel (svg_android, width, &__w);
	_svg_android_length_to_pixel (svg_android, height, &__h);

	int __in = in2int(in, in_op_reference);
	int __in2 = in2int(in2, in2_op_reference);

	int bmode = -1;
	switch(mode) {
	case feBlend_normal:
		bmode = 0;
		break;
	case feBlend_multiply:
		bmode = 1;
		break;
	case feBlend_screen:
		bmode = 2;
		break;
	case feBlend_darken:
		bmode = 3;
		break;
	case feBlend_lighten:
		bmode = 4;
		break;
	}

	(*env)->CallVoidMethod(env,
			       svg_android->filter,
			       svg_android->add_filter_feBlend,
			       (int)__x, (int)__y, (int)__w, (int)__h,
			       __in, __in2,
			       bmode
		);

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t _svg_android_add_filter_feComposite (void *closure,
						  svg_length_t* x, svg_length_t* y,
						  svg_length_t* width, svg_length_t* height,
						  feCompositeOperator_t oprt,
						  svg_filter_in_t in, int in_op_reference,
						  svg_filter_in_t in2, int in2_op_reference,
						  double k1, double k2, double k3, double k4) {
	svg_android_t* svg_android = closure;
	JNIEnv* env = svg_android->env;

	SVG_ANDROID_DEBUG("_svg_android_add_filter_feComposite()\n");

	if(svg_android->filter == NULL && prep_filter(env, svg_android))
		return SVG_ANDROID_STATUS_NO_MEMORY;

	double __x, __y, __w, __h;
	_svg_android_length_to_pixel (svg_android, x, &__x);
	_svg_android_length_to_pixel (svg_android, y, &__y);
	_svg_android_length_to_pixel (svg_android, width, &__w);
	_svg_android_length_to_pixel (svg_android, height, &__h);

	int __in = in2int(in, in_op_reference);
	int __in2 = in2int(in2, in2_op_reference);

	int boprt = -1;
	switch(oprt) {
	case feComposite_over:
		boprt = 1;
		break;
	case feComposite_in:
		boprt = 2;
		break;
	case feComposite_out:
		boprt = 3;
		break;
	case feComposite_atop:
		boprt = 4;
		break;
	case feComposite_xor:
		boprt = 5;
		break;
	case feComposite_arithmetic:
		boprt = 6;
		break;
	}

	(*env)->CallVoidMethod(env,
			       svg_android->filter,
			       svg_android->add_filter_feComposite,
			       (int)__x, (int)__y, (int)__w, (int)__h,
			       boprt,
			       __in, __in2,
			       k1, k2, k3, k4
		);

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t _svg_android_add_filter_feFlood (void *closure,
					      svg_length_t* x, svg_length_t* y,
					      svg_length_t* width, svg_length_t* height,
					      svg_filter_in_t in, int in_op_reference,
					      const svg_color_t* color, double opacity) {
	svg_android_t* svg_android = closure;
	JNIEnv* env = svg_android->env;

	SVG_ANDROID_DEBUG("_svg_android_add_filter_feFlood()\n");

	if(svg_android->filter == NULL && prep_filter(env, svg_android))
		return SVG_ANDROID_STATUS_NO_MEMORY;

	double __x, __y, __w, __h;
	_svg_android_length_to_pixel (svg_android, x, &__x);
	_svg_android_length_to_pixel (svg_android, y, &__y);
	_svg_android_length_to_pixel (svg_android, width, &__w);
	_svg_android_length_to_pixel (svg_android, height, &__h);

	int __in = in2int(in, in_op_reference);
	int _color = (0x00ffffff) & (color->rgb);

	(*env)->CallVoidMethod(env,
			       svg_android->filter,
			       svg_android->add_filter_feFlood,
			       (int)__x, (int)__y, (int)__w, (int)__h,
			       __in,
			       _color, opacity
		);

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t _svg_android_add_filter_feGaussianBlur (void *closure,
						     svg_length_t* x, svg_length_t* y,
						     svg_length_t* width, svg_length_t* height,
						     svg_filter_in_t in, int in_op_reference,
						     double std_dev_x, double std_dev_y) {
	svg_android_t* svg_android = closure;
	JNIEnv* env = svg_android->env;

	SVG_ANDROID_DEBUG("_svg_android_add_filter_feGaussianBlur()\n");

	if(svg_android->filter == NULL && prep_filter(env, svg_android))
		return SVG_ANDROID_STATUS_NO_MEMORY;

	double __x, __y, __w, __h;
	_svg_android_length_to_pixel (svg_android, x, &__x);
	_svg_android_length_to_pixel (svg_android, y, &__y);
	_svg_android_length_to_pixel (svg_android, width, &__w);
	_svg_android_length_to_pixel (svg_android, height, &__h);

	int __in = in2int(in, in_op_reference);

	(*env)->CallVoidMethod(env,
			       svg_android->filter,
			       svg_android->add_filter_feGaussianBlur,
			       (int)__x, (int)__y, (int)__w, (int)__h,
			       __in,
			       std_dev_x, std_dev_y
		);

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_status_t _svg_android_add_filter_feOffset (void *closure,
					       svg_length_t* x, svg_length_t* y,
					       svg_length_t* width, svg_length_t* height,
					       svg_filter_in_t in, int in_op_reference,
					       double dx, double dy) {
	svg_android_t* svg_android = closure;
	JNIEnv* env = svg_android->env;

	SVG_ANDROID_DEBUG("_svg_android_add_filter_feBlend()\n");

	if(svg_android->filter == NULL && prep_filter(env, svg_android))
		return SVG_ANDROID_STATUS_NO_MEMORY;

	double __x, __y, __w, __h;
	_svg_android_length_to_pixel (svg_android, x, &__x);
	_svg_android_length_to_pixel (svg_android, y, &__y);
	_svg_android_length_to_pixel (svg_android, width, &__w);
	_svg_android_length_to_pixel (svg_android, height, &__h);

	int __in = in2int(in, in_op_reference);

	(*env)->CallVoidMethod(env,
			       svg_android->filter,
			       svg_android->add_filter_feOffset,
			       (int)__x, (int)__y, (int)__w, (int)__h,
			       __in,
			       dx, dy
		);

	return SVG_ANDROID_STATUS_SUCCESS;
}

void
_svg_android_prepare_filter (svg_android_t* svg_android) {
	SVG_ANDROID_DEBUG("_svg_android_prepare_filter(%p, state: %p) - w: %f, h: %f\n",
			  svg_android,
			  svg_android->state,
			  svg_android->state->viewport_width,
			  svg_android->state->viewport_height);

	svg_android->state->filter_source_bitmap = ANDROID_CREATE_BITMAP(svg_android,
									 (int)svg_android->state->viewport_width,
									 (int)svg_android->state->viewport_height);

	ANDROID_FILL_BITMAP(svg_android, svg_android->state->filter_source_bitmap, 0x00000000);
	jobject new_canvas = ANDROID_CANVAS_CREATE(
		svg_android, svg_android->state->filter_source_bitmap);

	svg_android->state->saved_filter_canvas = svg_android->canvas;
	svg_android->canvas = new_canvas;

	_svg_android_copy_canvas_state (svg_android);
}

void
_svg_android_execute_filter (svg_android_t* svg_android) {
	JNIEnv* env = svg_android->env;

	if(svg_android->state && svg_android->state->saved_filter_canvas) {
		svg_android->canvas = svg_android->state->saved_filter_canvas;
		svg_android->state->saved_filter_canvas = NULL;

		(*env)->CallVoidMethod(env,
				       svg_android->filter,
				       svg_android->filter_execute,
				       svg_android->canvas,
				       svg_android->state->filter_source_bitmap
			);

		ANDROID_DRAW_BITMAP2(svg_android,
				     svg_android->state->filter_source_bitmap, 0.0f, 0.0f);
	}
}
