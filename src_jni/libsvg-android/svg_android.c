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

#include <android/log.h>

#include "svg_android_debug.h"

svg_render_engine_t SVG_ANDROID_RENDER_ENGINE = {
	/* hierarchy */
	.begin_group = _svg_android_begin_group,
	.begin_element = _svg_android_begin_element,
	.end_element = _svg_android_end_element,
	.end_group = _svg_android_end_group,
	/* path creation */
	.move_to = _svg_android_move_to,
	.line_to = _svg_android_line_to,
	.curve_to = _svg_android_curve_to,
	.quadratic_curve_to = _svg_android_quadratic_curve_to,
	.arc_to = _svg_android_arc_to,
	.close_path = _svg_android_close_path,
	.free_path_cache = _svg_android_free_path_cache,
	/* style */
	.set_color = _svg_android_set_color,
	.set_fill_opacity = _svg_android_set_fill_opacity,
	.set_fill_paint = _svg_android_set_fill_paint,
	.set_fill_rule = _svg_android_set_fill_rule,
	.set_font_family = _svg_android_set_font_family,
	.set_font_size = _svg_android_set_font_size,
	.set_font_style = _svg_android_set_font_style,
	.set_font_weight = _svg_android_set_font_weight,
	.set_opacity = _svg_android_set_opacity,
	.set_stroke_dash_array = _svg_android_set_stroke_dash_array,
	.set_stroke_dash_offset = _svg_android_set_stroke_dash_offset,
	.set_stroke_line_cap = _svg_android_set_stroke_line_cap,
	.set_stroke_line_join = _svg_android_set_stroke_line_join,
	.set_stroke_miter_limit = _svg_android_set_stroke_miter_limit,
	.set_stroke_opacity = _svg_android_set_stroke_opacity,
	.set_stroke_paint = _svg_android_set_stroke_paint,
	.set_stroke_width = _svg_android_set_stroke_width,
	.set_text_anchor = _svg_android_set_text_anchor,
	.set_filter = _svg_android_set_filter,
	/* filter */
	.begin_filter = _svg_android_begin_filter,
	.add_filter_feBlend = _svg_android_add_filter_feBlend,
	.add_filter_feComposite = _svg_android_add_filter_feComposite,
	.add_filter_feFlood = _svg_android_add_filter_feFlood,
	.add_filter_feGaussianBlur = _svg_android_add_filter_feGaussianBlur,
	.add_filter_feOffset = _svg_android_add_filter_feOffset,
	/* transform */
	.apply_clip_box = _svg_android_apply_clip_box,
	.transform = _svg_android_transform,
	.apply_view_box = _svg_android_apply_view_box,
	.set_viewport_dimension = _svg_android_set_viewport_dimension,

	/* drawing */
	.render_line = _svg_android_render_line,
	.render_path = _svg_android_render_path,
	.render_ellipse = _svg_android_render_ellipse,
	.render_rect = _svg_android_render_rect,
	.render_text = _svg_android_render_text,
	.render_image = _svg_android_render_image,

	/* get bounding box of last drawing, in pixels */
	.get_last_bounding_box = _svg_android_get_last_bounding_box
};

svg_android_status_t svgAndroidDestroy(svg_android_t *svg_android) {
	svg_android_status_t status;

	_svg_android_pop_state (svg_android);

	status = svg_destroy (svg_android->svg);

	free (svg_android);

	return status;
}

svg_android_t *svgAndroidCreate(void) {
	svg_android_t *svg_android;

	svg_android = (svg_android_t *)malloc (sizeof (svg_android_t));

	if (svg_android != NULL) {
		svg_android->do_antialias = JNI_FALSE;

		svg_android->canvas = NULL;
		svg_android->state = NULL;

		if(svg_create (&(svg_android)->svg)) {
			free(svg_android);
			svg_android = NULL;
		}
	}

	return svg_android;
}

JNIEXPORT jlong JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidCreate
(JNIEnv * env, jclass jc)
{
	return (jlong)svgAndroidCreate();
}

JNIEXPORT jint JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidDestroy
(JNIEnv *env, jclass jc, jlong _svg_android_r)
{
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;

	return svgAndroidDestroy(svg_android);
}

JNIEXPORT jint JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidParseBuffer
(JNIEnv *env, jclass jc, jlong _svg_android_r, jstring _bfr)
{
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;


	const char *buf = (*env)->GetStringUTFChars(env, _bfr, JNI_FALSE);

	svg_android_status_t status = svg_parse_buffer (svg_android->svg, buf, strlen(buf));

	(*env)->ReleaseStringUTFChars(env, _bfr, buf);

	return status;
}

JNIEXPORT jint JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidParseChunkBegin
(JNIEnv *env, jclass jc, jlong _svg_android_r)
{
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;
	return svg_parse_chunk_begin (svg_android->svg);
}

JNIEXPORT jint JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidParseChunk
(JNIEnv *env, jclass jc, jlong _svg_android_r, jstring _bfr)
{
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;

	const char *buf = (*env)->GetStringUTFChars(env, _bfr, JNI_FALSE);

	svg_android_status_t status = svg_parse_chunk (svg_android->svg, buf, strlen(buf));

	(*env)->ReleaseStringUTFChars(env, _bfr, buf);

	return status;
}

JNIEXPORT jint JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidParseChunkEnd
(JNIEnv *env, jclass jc, jlong _svg_android_r)
{
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;
	return svg_parse_chunk_end (svg_android->svg);
}

static jclass canvas_clazz = NULL; // android Canvas classj
static jclass raster_clazz = NULL; // android SvgRaster class
static jclass bitmap_clazz = NULL; // android Bitmap class
static jclass matrix_clazz = NULL; // android matrix class
static jclass shader_clazz = NULL; // android shader class
static jclass path_clazz = NULL;   // android path class
static jclass paint_clazz = NULL; // android paint class
static jclass dashPathEffect_clazz = NULL; // android paint dash effect class

void __prepare_android_interface(svg_android_t *svg_android, JNIEnv *env, jobject *android_canvas) {
	jclass d;

	svg_android->canvas = android_canvas;
	if(svg_android->env == env) {
#if 0
		/***************/
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_constructor: %p", svg_android->canvas_constructor);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_save: %p", svg_android->canvas_save);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_restore: %p", svg_android->canvas_restore);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->canvas_clip_rect);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->canvas_concat_matrix);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_draw_bitmap: %p", svg_android->canvas_draw_bitmap);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_draw_path: %p", svg_android->canvas_draw_path);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_draw_text: %p", svg_android->canvas_draw_text);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_drawRGB: %p", svg_android->canvas_drawRGB);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_getWidth: %p", svg_android->canvas_getWidth);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "canvas_getHeight: %p", svg_android->canvas_getHeight);

		/***************/
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_setTypeface: %p", svg_android->raster_setTypeface);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_getBounds: %p", svg_android->raster_getBounds);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_matrixInit: %p", svg_android->raster_matrixInit);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_createBitmap: %p", svg_android->raster_createBitmap);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_data2bitmap: %p", svg_android->raster_data2bitmap);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_setFillRule: %p", svg_android->raster_setFillRule);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_setPaintStyle: %p", svg_android->raster_setPaintStyle);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_setStrokeCap: %p", svg_android->raster_setStrokeCap);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_setStrokeJoin: %p", svg_android->raster_setStrokeJoin);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_createBitmapShader: %p", svg_android->raster_createBitmapShader);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_createLinearGradient: %p", svg_android->raster_createLinearGradient);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_createRadialGradient: %p", svg_android->raster_createRadialGradient);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_matrixInvert: %p", svg_android->raster_matrixInvert);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->raster_getBoundingBox);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_drawEllipse: %p", svg_android->raster_drawEllipse);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_drawRect: %p", svg_android->raster_drawRect);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "raster_debugMatrix: %p", svg_android->raster_debugMatrix);

		/* android bitmap method references */
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->bitmap_erase_color);

		/* android matrix method references */
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->matrix_constructor);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->matrix_postTranslate);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->matrix_postScale);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->matrix_postConcat);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->matrix_reset);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->matrix_set);

		/* android shader method references */
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->shader_setLocalMatrix);

		/* android path method references */
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_constructor);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_clone_constructor);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_transform);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_moveTo);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_lineTo);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_cubicTo);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_quadTo);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_close);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->path_reset);

		/* android paint method references */
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_constructor);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_setPathEffect);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_setARGB);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_setShader);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_setStrokeMiter);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_setStrokeWidth);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_getTextPath);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_setAntialias);
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->paint_set);

		/* DashPathEffect method/constructors refs */
		__android_log_print(ANDROID_LOG_INFO, "libsvg-android",
				    "local reference at line %d is %p", __LINE__, svg_android->dashPathEffect_constructor);
#endif
		return;
	}

	svg_android->env = env;

	// prepare canvas class/methods
	if(!canvas_clazz) {
		d = (*env)->FindClass(env, "android/graphics/Canvas");
		canvas_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->canvas_clazz = canvas_clazz;

	svg_android->canvas_constructor = (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "<init>", "(Landroid/graphics/Bitmap;)V");
	svg_android->canvas_save = (*env)->GetMethodID(env,svg_android->canvas_clazz, "save", "()I");
	svg_android->canvas_restore = (*env)->GetMethodID(env,svg_android->canvas_clazz, "restore", "()V");
	svg_android->canvas_clip_rect = (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "clipRect",
		"(FFFF)Z");
	svg_android->canvas_concat = (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "concat",
		"(Landroid/graphics/Matrix;)V");
	svg_android->canvas_draw_bitmap = (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "drawBitmap",
		"(Landroid/graphics/Bitmap;Landroid/graphics/Matrix;Landroid/graphics/Paint;)V");
	svg_android->canvas_draw_bitmap2 = (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "drawBitmap",
		"(Landroid/graphics/Bitmap;FFLandroid/graphics/Paint;)V");
	svg_android->canvas_draw_path = (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "drawPath",
		"(Landroid/graphics/Path;Landroid/graphics/Paint;)V");
	svg_android->canvas_draw_text = (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "drawText",
		"(Ljava/lang/String;FFLandroid/graphics/Paint;)V");
	svg_android->canvas_drawRGB =  (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "drawRGB",
		"(III)V");
	svg_android->canvas_getWidth =  (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "getWidth",
		"()I");
	svg_android->canvas_getHeight =  (*env)->GetMethodID(env,
		svg_android->canvas_clazz, "getHeight",
		"()I");

	// prepare raster
	if(!raster_clazz) {
		d = (*env)->FindClass(env, "com/toolkits/libsvgandroid/SvgRaster");
		raster_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->raster_clazz = raster_clazz;

	svg_android->raster_setTypeface = (*env)->GetStaticMethodID(
		env,
		svg_android->raster_clazz, "setTypeface",
		"(Landroid/graphics/Paint;Ljava/lang/String;IFI)V"
		);
	svg_android->raster_getBounds = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "getBounds",
		"(Landroid/graphics/Path;)[F"
		);
	svg_android->raster_matrixCreate = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "matrixCreate",
		"(FFFFFF)Landroid/graphics/Matrix;"
		);
	svg_android->raster_matrixInit = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "matrixInit",
		"(Landroid/graphics/Matrix;FFFFFF)V"
		);
	svg_android->raster_createBitmap = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "createBitmap",
		"(II)Landroid/graphics/Bitmap;"
		);
	svg_android->raster_data2bitmap = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "data2bitmap",
		"(II[I)Landroid/graphics/Bitmap;"
		);
	svg_android->raster_setFillRule = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "setFillRule",
		"(Landroid/graphics/Path;Z)V"
		);
	svg_android->raster_setPaintStyle = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "setPaintStyle",
		"(Landroid/graphics/Paint;Z)V"
		);
	svg_android->raster_setStrokeCap = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "setStrokeCap",
		"(Landroid/graphics/Paint;I)V"
		);
	svg_android->raster_setStrokeJoin = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "setStrokeJoin",
		"(Landroid/graphics/Paint;I)V"
		);
	svg_android->raster_createBitmapShader = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "createBitmapShader",
		"(Landroid/graphics/Bitmap;)Landroid/graphics/Shader;"
		);
	svg_android->raster_createLinearGradient = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "createLinearGradient",
		"(FFFF[I[FI)Landroid/graphics/Shader;"
		);
	svg_android->raster_createRadialGradient = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "createRadialGradient",
		"(FFF[I[FI)Landroid/graphics/Shader;"
		);
	svg_android->raster_matrixInvert = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "matrixInvert", "(Landroid/graphics/Matrix;)Landroid/graphics/Matrix;");
	svg_android->raster_getBoundingBox = (*env)->GetStaticMethodID(
		env, svg_android->raster_clazz, "getBoundingBox", "(Landroid/graphics/Path;Landroid/graphics/Canvas;)V");
	svg_android->raster_drawEllipse = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "drawEllipse", "(Landroid/graphics/Canvas;Landroid/graphics/Paint;FFFF)V");
	svg_android->raster_drawRect = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "drawRect", "(Landroid/graphics/Canvas;Landroid/graphics/Paint;FFFFFF)V");
	svg_android->raster_debugMatrix = (*env)->GetStaticMethodID(env,
		svg_android->raster_clazz, "debugMatrix", "(Landroid/graphics/Matrix;)V");

	// prepare bitmap class/methods
	if(!bitmap_clazz) {
		d = (*env)->FindClass(env, "android/graphics/Bitmap");
		bitmap_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->bitmap_clazz = bitmap_clazz;

	svg_android->bitmap_erase_color = (*env)->GetMethodID(env,
		svg_android->bitmap_clazz, "eraseColor",
		"(I)V"
		);

	// prepare matrix class/methods
	if(!matrix_clazz) {
		d = (*env)->FindClass(env, "android/graphics/Matrix");
		matrix_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->matrix_clazz = matrix_clazz;

	svg_android->matrix_constructor = (*env)->GetMethodID(env,
		svg_android->matrix_clazz, "<init>", "()V");
	svg_android->matrix_postTranslate = (*env)->GetMethodID(env,
		svg_android->matrix_clazz, "postTranslate", "(FF)Z");
	svg_android->matrix_postScale = (*env)->GetMethodID(env,
		svg_android->matrix_clazz, "postScale", "(FF)Z");
	svg_android->matrix_postConcat = (*env)->GetMethodID(env,
		svg_android->matrix_clazz, "postConcat", "(Landroid/graphics/Matrix;)Z");
	svg_android->matrix_reset = (*env)->GetMethodID(env,
		svg_android->matrix_clazz, "reset", "()V");
	svg_android->matrix_set = (*env)->GetMethodID(env,
		svg_android->matrix_clazz, "set", "(Landroid/graphics/Matrix;)V");

	// prepare shader class/methods
	if(!shader_clazz) {
		d = (*env)->FindClass(env, "android/graphics/Shader");
		shader_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->shader_clazz = shader_clazz;

	svg_android->shader_setLocalMatrix = (*env)->GetMethodID(env,
		svg_android->shader_clazz, "setLocalMatrix", "(Landroid/graphics/Matrix;)V");

	// prepare path class/methods
	if(!path_clazz) {
		d = (*env)->FindClass(env, "android/graphics/Path");
		path_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->path_clazz = path_clazz;

	svg_android->path_constructor = (*env)->GetMethodID(env,
		svg_android->path_clazz, "<init>", "()V");
	svg_android->path_clone_constructor = (*env)->GetMethodID(env,
		svg_android->path_clazz, "<init>", "(Landroid/graphics/Path;)V");
	svg_android->path_transform = (*env)->GetMethodID(env,
		svg_android->path_clazz, "transform", "(Landroid/graphics/Matrix;)V");
	svg_android->path_moveTo = (*env)->GetMethodID(env,
		svg_android->path_clazz, "moveTo", "(FF)V");
	svg_android->path_lineTo = (*env)->GetMethodID(env,
		svg_android->path_clazz, "lineTo", "(FF)V");
	svg_android->path_cubicTo = (*env)->GetMethodID(env,
		svg_android->path_clazz, "cubicTo", "(FFFFFF)V");
	svg_android->path_quadTo = (*env)->GetMethodID(env,
		svg_android->path_clazz, "quadTo", "(FFFF)V");
	svg_android->path_close = (*env)->GetMethodID(env,
		svg_android->path_clazz, "close", "()V");
	svg_android->path_reset = (*env)->GetMethodID(env,
		svg_android->path_clazz, "reset", "()V");

	// prepare paint class/methods
	if(!paint_clazz) {
		d = (*env)->FindClass(env, "android/graphics/Paint");
		paint_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->paint_clazz = paint_clazz;

	svg_android->paint_constructor = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "<init>", "()V");
	svg_android->paint_setPathEffect = (*env)->GetMethodID(
		env,
		svg_android->paint_clazz, "setPathEffect",
		"(Landroid/graphics/PathEffect;)Landroid/graphics/PathEffect;");
	svg_android->paint_setARGB = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "setARGB", "(IIII)V");
	svg_android->paint_setShader = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "setShader", "(Landroid/graphics/Shader;)Landroid/graphics/Shader;");
	svg_android->paint_setStrokeMiter = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "setStrokeMiter", "(F)V");
	svg_android->paint_setStrokeWidth = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "setStrokeWidth", "(F)V");
	svg_android->paint_getTextPath = (*env)->GetMethodID(
		env,
		svg_android->paint_clazz, "getTextPath",
		"(Ljava/lang/String;IIFFLandroid/graphics/Path;)V");
	svg_android->paint_setAntialias = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "setAntiAlias", "(Z)V");
	svg_android->paint_set = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "set", "(Landroid/graphics/Paint;)V");
	svg_android->paint_reset = (*env)->GetMethodID(env,
		svg_android->paint_clazz, "reset", "()V");

	// prepare dash path effect class/methods
	if(!dashPathEffect_clazz) {
		d = (*env)->FindClass(env, "android/graphics/DashPathEffect");
		dashPathEffect_clazz = (jclass)((*env)->NewGlobalRef(env, (jobject)d));
	}
	svg_android->dashPathEffect_clazz = dashPathEffect_clazz;

	svg_android->dashPathEffect_constructor = (*env)->GetMethodID(env,
		svg_android->dashPathEffect_clazz, "<init>", "([FF)V");
}

void svgAndroidSetAntialiasing(svg_android_t *svg_android, jboolean doAntiAlias) {
	svg_android->do_antialias = doAntiAlias;
}

JNIEXPORT int JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidSetAntialiasing
(JNIEnv *env, jclass jc, jlong _svg_android_r, jboolean doAntiAlias) {
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;
	svg_android->do_antialias = doAntiAlias;
	return 0;
}

svg_status_t svgAndroidRender
(JNIEnv *env, svg_android_t *svg_android, jobject android_canvas)
{
	__prepare_android_interface(svg_android, env, android_canvas);

	_svg_android_push_state (svg_android, NULL, NULL);
	svg_android->state->viewport_width = ANDROID_GET_WIDTH(svg_android);
	svg_android->state->viewport_height = ANDROID_GET_HEIGHT(svg_android);

	svg_android->fit_to_area = 0;
	svg_status_t return_status = svg_render (svg_android->svg, &SVG_ANDROID_RENDER_ENGINE, svg_android);

	SVG_ANDROID_ERROR("SVG Android Engine = %p\n",
			  &SVG_ANDROID_RENDER_ENGINE);
	SVG_ANDROID_ERROR("SVG_ANDROID_RENDER_ENGINE.add_filter_feFlood = %p\n",
			  SVG_ANDROID_RENDER_ENGINE.add_filter_feFlood);

	(void) _svg_android_pop_state (svg_android);

	return return_status;
}

JNIEXPORT jint JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidRender
(JNIEnv *env, jclass jc, jlong _svg_android_r, jobject android_canvas)
{
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;
	return svgAndroidRender(env, svg_android, android_canvas);
}

svg_status_t svgAndroidRenderToArea(JNIEnv *env, svg_android_t *svg_android, jobject android_canvas, int x, int y, int w, int h) {
	__prepare_android_interface(svg_android, env, android_canvas);

	_svg_android_push_state (svg_android, NULL, NULL);

	svg_android->state->viewport_width = w;
	svg_android->state->viewport_height = h;

	svg_android->fit_to_area = -1;
	svg_android->fit_to_x = x;
	svg_android->fit_to_y = y;
	svg_android->fit_to_w = w;
	svg_android->fit_to_h = h;
	svg_android->fit_to_MATRIX = NULL;

	svg_status_t return_status =  svg_render (svg_android->svg, &SVG_ANDROID_RENDER_ENGINE, svg_android);

	_svg_android_pop_state (svg_android);

	return return_status;
}

JNIEXPORT jint JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidRenderToArea
(JNIEnv *env, jclass jc, jlong _svg_android_r, jobject android_canvas,
 jint x, jint y, jint w, jint h)
{
	svg_android_t *svg_android = (svg_android_t *)_svg_android_r;
	return svgAndroidRenderToArea(env, svg_android, android_canvas, x, y, w, h);
}

static int __internal_bounding_box_is_in_clip;
static svg_bounding_box_t __internal_bounding_box;
JNIEXPORT void JNICALL Java_com_toolkits_libsvgandroid_SvgRaster_svgAndroidSetBoundingBox
(JNIEnv *env, jclass jc, jboolean is_in_clip, jint left, jint top, jint right, jint bottom)
{
	if(left < 0) left = 0;
	if(top < 0) top = 0;
	if(right < 0) right = 0;
	if(bottom < 0) bottom = 0;

	__internal_bounding_box_is_in_clip = is_in_clip == JNI_TRUE ? -1 : 0;
	__internal_bounding_box.left = left;
	__internal_bounding_box.top = top;
	__internal_bounding_box.right = right;
	__internal_bounding_box.bottom = bottom;
}

int svgAndroidGetInternalBoundingBox(svg_bounding_box_t *bbox) {
	*bbox = __internal_bounding_box; // straight copy of data
	return __internal_bounding_box_is_in_clip;
}

void svgAndroidEnablePathCache(svg_android_t *svg_android) {

#if 0 // caching does not work on all devices it seems, I do not know why...
	svg_enable_path_cache(svg_android->svg);
#endif
}
