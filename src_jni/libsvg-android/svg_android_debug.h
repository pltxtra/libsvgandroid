/*
 * Copyright (C) 2013 by Anton Persson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program;
 * if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#define WHERESTR  "[file %s, line %d]: "
#define WHEREARG  __FILE__, __LINE__

#ifdef __DO_SVG_ANDROID_DEBUG

#ifdef ANDROID

#include <android/log.h>
#define SVG_ANDROID_DEBUG_(...)       __android_log_print(ANDROID_LOG_INFO, "SVG_ANDROID", __VA_ARGS__)
#define SVG_ANDROID_DEBUG(...)       __android_log_print(ANDROID_LOG_INFO, "SVG_ANDROID", __VA_ARGS__)

#else

#include <stdio.h>
#define SVG_ANDROID_DEBUG_(...)       printf(__VA_ARGS__)
#define SVG_ANDROID_DEBUG(...)  printf(__VA_ARGS__)

#endif

#else
// disable debugging

#define SVG_ANDROID_DEBUG_(...)
#define SVG_ANDROID_DEBUG(_fmt, ...)

#endif

#ifdef ANDROID

#include <android/log.h>
#define SVG_ANDROID_ERROR(...)       __android_log_print(ANDROID_LOG_INFO, "SVG_ANDROID", __VA_ARGS__)

#else

#include <stdio.h>
#define SVG_ANDROID_ERROR(...)  printf(__VA_ARGS__)

#endif
