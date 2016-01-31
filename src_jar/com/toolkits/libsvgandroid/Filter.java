/*
 * libsvg-android, ANDROID version
 * Copyright (C) 2016 by Anton Persson
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
import android.renderscript.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

public class Filter {

	interface feBaseOperation {
		public void execute(FilterStack fstack);
	}

	class FilterStack {
		public RenderScript rs;
		public List<feBaseOperation> operations;

		public FilterStack() {
			operations = new ArrayList<feBaseOperation>();
		}

		public void add(feBaseOperation fe) {
			operations.add(fe);
		}

	}

	class feBlend implements feBaseOperation {
		@Override
		public void execute(FilterStack fstack) {
		}

		int x, y;
		int width, height;
		int in, in2;
		int mode;

		public feBlend(int  _x, int _y,
			       int _width, int _height,
			       int _in, int _in2,
			       int _mode) {
			x = _x; y = _y;
			width = _width;
			height = _height;
			in = _in;
			in2 = _in2;
			mode = _mode;
		}
	}

	class feComposite implements feBaseOperation {
		@Override
		public void execute(FilterStack fstack) {
		}

		int x, y, width, height, oprt, in, in2;
		double k1, k2, k3, k4;

		feComposite(int _x, int _y, int _width, int _height, int _oprt,
			    int _in, int _in2,
			    double _k1, double _k2,
			    double _k3, double _k4
			) {
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			oprt = _oprt;
			in = _in;
			in2 = _in2;
			k1 = _k1;
			k2 = _k2;
			k3 = _k3;
			k4 = _k4;
		}
	}

	class feFlood implements feBaseOperation {
		@Override
		public void execute(FilterStack fstack) {
		}

		int x, y, width, height, in, color;
		double opacity;

		feFlood(int _x, int _y, int _width, int _height, int _in,
			int _color, double _opacity) {
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			in = _in;
			color = _color;
			opacity = _opacity;
		}
	}

	class feGaussianBlur implements feBaseOperation {
		@Override
		public void execute(FilterStack fstack) {
		}

		int x, y, width, height, in;
		double std_dev_x, std_dev_y;

		feGaussianBlur(int _x, int _y, int _width, int _height,
			       int _in,
			       double _std_dev_x, double _std_dev_y) {
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			in = _in;
			std_dev_x = _std_dev_x;
			std_dev_y = _std_dev_y;
		}
	}

	class feOffset implements feBaseOperation {
		@Override
		public void execute(FilterStack fstack) {
		}

		int x, y, width, height, in;
		double dx, dy;

		feOffset(int _x, int _y, int _width, int _height,
			 int _in, double _dx, double _dy) {
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			in = _in;
			dx = _dx;
			dy = _dy;
		}
	}

	Map<String, FilterStack> filters;
	FilterStack current;

	public static Filter createFilter() {
		Log.v("Kamoflage", "Filter.java:Filter.createFilter()");
		return new Filter();
	}

	public Filter() {
		filters = new HashMap<String, FilterStack>();
	}

	public void setFilter(String name) {
		Log.v("Kamoflage", "Filter.java:Filter.setFilter(" + name + ")");
	}

	public void beginFilter(String name) {
		Log.v("Kamoflage", "Filter.java:Filter.beginFilter(" + name + ")");

		current = new FilterStack();
		filters.put(name, current);
	}

	public void addFilter_feBlend(int x, int y, int width, int height, int in, int in2, int mode) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feBlend()");
		if(current != null) {
			current.add(new feBlend(x, y, width, height, in, in2, mode));
		}
	}

	public void addFilter_feComposite(int x, int y, int width, int height, int oprt,
					  int in, int in2,
					  double k1, double k2,
					  double k3, double k4
		) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feComposite()");
		if(current != null) {
			current.add(new feComposite(x, y, width, height,
						    oprt, in, in2,
						    k1, k2, k3, k4));
		}
	}

	public void addFilter_feFlood(int x, int y, int width, int height, int in,
				      int color, double opacity) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feFlood()");
		if(current != null) {
			current.add(new feFlood(x, y, width, height,
						in, color, opacity));
		}
	}

	public void addFilter_feGaussianBlur(int x, int y, int width, int height,
					     int in,
					     double std_dev_x, double std_dev_y) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feGaussianBlur()");
		if(current != null) {
			current.add(new feGaussianBlur(x, y, width, height, in,
						       std_dev_x, std_dev_y));
		}
	}

	public void addFilter_feOffset(int x, int y, int width, int height,
				       int in, double dx, double dy) {
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feOffset()");
		if(current != null) {
			current.add(new feOffset(x, y, width, height,
						 in, dx, dy));
		}
	}
}
