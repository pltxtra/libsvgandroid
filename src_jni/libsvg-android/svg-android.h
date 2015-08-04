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

#ifndef SVG_ANDROID_H
#define SVG_ANDROID_H

#include <jni.h>

#include <svg.h>

#ifdef __cplusplus
extern "C" {
#endif
	
	typedef enum svg_android_status {
		SVG_ANDROID_STATUS_SUCCESS = SVG_STATUS_SUCCESS,
		SVG_ANDROID_STATUS_NO_MEMORY = SVG_STATUS_NO_MEMORY,
		SVG_ANDROID_STATUS_IO_ERROR = SVG_STATUS_IO_ERROR,
		SVG_ANDROID_STATUS_FILE_NOT_FOUND = SVG_STATUS_FILE_NOT_FOUND,
		SVG_ANDROID_STATUS_INVALID_VALUE = SVG_STATUS_INVALID_VALUE,
		SVG_ANDROID_STATUS_INVALID_CALL = SVG_STATUS_INVALID_CALL,
		SVG_ANDROID_STATUS_PARSE_ERROR = SVG_STATUS_PARSE_ERROR
	} svg_android_status_t;
	
	typedef struct svg_android svg_android_t;
	
	svg_android_t *svgAndroidCreate();
	svg_android_status_t svgAndroidDestroy(svg_android_t *svg_android);
	void svgAndroidSetAntialiasing(svg_android_t *svg_android, jboolean doAntiAlias);
	svg_status_t svgAndroidRender(
		JNIEnv *env, svg_android_t *svg_android, jobject android_canvas);
	svg_status_t svgAndroidRenderToArea(
		JNIEnv *env, svg_android_t *svg_android,
		jobject android_canvas, int x, int y, int w, int h) ;
	int svgAndroidGetInternalBoundingBox(svg_bounding_box_t *bbox);
	void svgAndroidEnablePathCache(svg_android_t *svg_android);
#ifdef __cplusplus
}
#endif

#endif
