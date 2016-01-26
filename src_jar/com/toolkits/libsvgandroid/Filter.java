/*
 * libsvg-android, ANDROID version
 * Copyright (C) 2010 by Anton Persson
 *
 * http://www.733kru.org/
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
 */

package com.toolkits.libsvgandroid;

import android.graphics.Matrix;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Path;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.BitmapShader;
import android.graphics.LinearGradient;
import android.graphics.RadialGradient;
import android.graphics.Typeface;
import android.util.Log;

public class Filter {

	public static Filter createFilter() {
		Log.v("Kamoflage", "Filter.java:Filter.createFilter()");
		return new Filter();
	}

	public void setFilter(String name) {
		Log.v("Kamoflage", "Filter.java:Filter.setFilter()");
	}

	public void beginFilter(String name) {
		Log.v("Kamoflage", "Filter.java:Filter.beginFilter()");
	}

	public void addFilter_feBlend(int  x, int y, int width, int height, int in, int in2, int mode) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feBlend()");
	}

	public void addFilter_feComposite(int  x, int y, int width, int height, int oprt,
					  int in, int in2,
					  double k1, double k2,
					  double k3, double k4
		) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feBlend()");
	}

	public void addFilter_feFlood(int  x, int y, int width, int height, int in,
				      int color, double opacity) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feBlend()");
	}

	public void addFilter_feGaussianBlur(int  x, int y, int width, int height,
					     int in,
					     double std_dev_x, double std_dev_y) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feBlend()");
	}

	public void addFilter_feOffset(int  x, int y, int width, int height,
				       int in, double dx, double dy) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feBlend()");
	}

}
