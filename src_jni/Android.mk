#
# libsvg-android
# Copyright (C) 2015 by Anton Persson
#
# This program is free software; you can redistribute it and/or modify it under the terms of
# the GNU General Public License version 2; see COPYING for the complete License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with this program;
# if not, write to the
# Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

LOCAL_PATH := $(call my-dir)

#
#
#   Build libjpeg
#
#
include $(CLEAR_VARS)
LOCAL_MODULE    := libjpeg
LOCAL_SRC_FILES := ../prereqs/lib/libjpeg.a
include $(PREBUILT_STATIC_LIBRARY)

#
#
#   Build libzlib
#
#
include $(CLEAR_VARS)
LOCAL_MODULE    := libz
LOCAL_SRC_FILES := ../prereqs/lib/libz.a
include $(PREBUILT_STATIC_LIBRARY)

#
#
#   Build libpng
#
#
include $(CLEAR_VARS)
LOCAL_MODULE    := libpng
LOCAL_SRC_FILES := ../prereqs/lib/libpng.a
include $(PREBUILT_STATIC_LIBRARY)

#
#
#   Build libexpat
#
#
include $(CLEAR_VARS)
LOCAL_MODULE    := libexpat
LOCAL_SRC_FILES := ../prereqs/lib/libexpat.a
include $(PREBUILT_STATIC_LIBRARY)

#
#
#   Build libsvg-android
#
#
include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := .cc

LOCAL_MODULE    := svgandroid

LOCAL_CFLAGS += -DLIBSVG_EXPAT -DCONFIG_DIR=\"/\" \
-I../prereqs/include/ \
-Ijni/libsvg/ \
-DHAVE_CONFIG_H -Wall
LOCAL_CPPFLAGS += -DASIO_STANDALONE -std=c++11

# debugging
#LOCAL_CFLAGS += -DDEBUG_LIBSVG_ANDROID

LOCAL_STATIC_LIBRARIES := libjpeg libz libpng libexpat

# libsvg stuff
LIBSVG_EXTRA_SOURCES = libsvg/svg_parser_expat.c libsvg/strhmap_cc.cc

LIBSVG_SOURCES = \
	libsvg/svg.c \
	libsvg/svg.h \
	libsvg/svgint.h \
	libsvg/svg_ascii.h \
	libsvg/svg_ascii.c \
	libsvg/svg_attribute.c \
	libsvg/svg_color.c \
	libsvg/svg_element.c \
	libsvg/svg_gradient.c \
	libsvg/svg_group.c \
	libsvg/svg_length.c \
	libsvg/svg_paint.c \
	libsvg/svg_parser.c \
	libsvg/svg_pattern.c \
	libsvg/svg_image.c \
	libsvg/svg_path.c \
	libsvg/svg_str.c \
	libsvg/svg_style.c \
	libsvg/svg_text.c \
	libsvg/svg_transform.c \
	libsvg/svg_version.h \
	libsvg/svg_filter.c \
	libsvg/svg_filter.h \
	$(LIBSVG_EXTRA_SOURCES)

# libsvg-android stuff
LIBSVG_ANDROID_SOURCES = \
	libsvg-android/svg_android.c \
	libsvg-android/svg_android_render.c \
	libsvg-android/svg_android_render_helper.c \
	libsvg-android/svg_android_state.c \
	libsvg-android/svg-android.h \
	libsvg-android/svg-android-internal.h

# package it
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := $(LIBSVG_SOURCES) $(LIBSVG_ANDROID_SOURCES)

include $(BUILD_SHARED_LIBRARY)
