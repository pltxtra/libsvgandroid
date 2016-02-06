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
import android.app.Activity;
import android.renderscript.*;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

public class Filter {

	static Activity current_activity;

	static public void setActivity(Activity crnt) {
		current_activity = crnt;
	}

	interface feBaseOperation {
		public void execute(FilterStack fstack);
		public Allocation getOut();
	}

	class FilterStack {
		public RenderScript rs;
		public List<feBaseOperation> operations;
		private Allocation background;
		private Allocation source;
		private Allocation backgroundAlpha;
		private Allocation sourceAlpha;
		private Bitmap pureAlpha;
		public Bitmap scratch;
		public ScriptIntrinsicBlend blendOp;
		public ScriptIntrinsicBlur blurOp;
		public int width, height;
		feBaseOperation lastOperation;

		public FilterStack() {
			operations = new ArrayList<feBaseOperation>();
			Log.v("Kamoflage", "Filter.java:FilterStack.FilterStack() - rs created");
		}

		public Allocation getInAllocation(int in) {
			switch(in) {
			case -1:
				Log.v("Kamoflage", "getInAllocation(-1, Source)");
				return source;
			case -2:
				Log.v("Kamoflage", "getInAllocation(-2, SourceAlpha)");
				return getSourceAlpha();
			case -3:
				Log.v("Kamoflage", "getInAllocation(-3, Background)");
				return background;
			case -4:
				Log.v("Kamoflage", "getInAllocation(-4, BackgroundAlpha)");
				return getBackgroundAlpha();
			case -5:
				Log.v("Kamoflage", "getInAllocation(-5, -- fail --)");
				return null;
			case -6:
				Log.v("Kamoflage", "getInAllocation(-6, -- fail --)");
				return null;
			default:
				Log.v("Kamoflage", "getInAllocation(" + in + ", <ref>)");
				return operations.get(in).getOut();
			}

		}
		public Bitmap getPureAlpha() {
			if(pureAlpha == null) {
				Log.v("Kamoflage", "Filter.java:FilterStack.getPureAlpha()" +
				      width + " x " + height);
				pureAlpha = Bitmap.createBitmap(
					width,
					height,
					Bitmap.Config.ARGB_8888);
				pureAlpha.eraseColor(0xff000000);
			}
			return pureAlpha;
		}

		public Allocation getSourceAlpha() {
			if(sourceAlpha == null) {
				sourceAlpha = Allocation.createFromBitmap(rs, getPureAlpha());
				blendOp.forEachDstIn(source, sourceAlpha);
			}
			return sourceAlpha;
		}

		public Allocation getBackgroundAlpha() {
			if(backgroundAlpha == null) {
				backgroundAlpha = Allocation.createFromBitmap(rs, getPureAlpha());
				blendOp.forEachDstIn(background, backgroundAlpha);
			}
			return backgroundAlpha;
		}

		public void add(feBaseOperation fe) {
			operations.add(fe);
			lastOperation = fe;
		}

		public Bitmap execute(Bitmap _background, Bitmap _source) {
			width = _source.getWidth();
			height = _source.getHeight();

			_background = Bitmap.createBitmap(_background, 0, 0, width, height);
			scratch = Bitmap.createBitmap(_background, 0, 0, width, height);

			rs = RenderScript.create(current_activity);

			background = Allocation.createFromBitmap(rs, _background);
			source = Allocation.createFromBitmap(rs, _source);

			backgroundAlpha = null;
			sourceAlpha = null;
			pureAlpha = null;

			blendOp =
				ScriptIntrinsicBlend.create(rs, background.getElement());
			blurOp =
				ScriptIntrinsicBlur.create(rs, background.getElement());

			Log.v("Kamoflage", "Filter.java:FilterStack.execute() BEGIN operations.");
			Log.v("Kamoflage", "Filter.java:FilterStack.execute() bg: ["
			      + _background.getWidth()
			      + ", "
			      + _background.getHeight()
			      +"]");
			Log.v("Kamoflage", "Filter.java:FilterStack.execute() src: ["
			      + _source.getWidth()
			      + ", "
			      + _source.getHeight()
			      +"]");

			for(feBaseOperation op : operations) {
				op.execute(this);
			}

			Log.v("Kamoflage", "Filter.java:FilterStack.execute()" +
				      width + " x " + height);
			Bitmap finalBitmap = Bitmap.createBitmap(
				width,
				height,
				Bitmap.Config.ARGB_8888);
			Log.v("Kamoflage", "Filter.java:FilterStack.execute() - finalBitmap.eraseColor()");
			finalBitmap.eraseColor(0x00000000);

			Log.v("Kamoflage", "Filter.java:FilterStack.execute() - get last operation...");
			if(lastOperation != null)
				lastOperation.getOut().copyTo(finalBitmap);

			Log.v("Kamoflage", "Filter.java:FilterStack.execute() - rs.finish()");
			rs.finish();
			rs.destroy();
			rs = null;

			return finalBitmap;
		}
	}

	class feBlend implements feBaseOperation {
		ScriptIntrinsicBlend blendOp;
		Allocation out;

		@Override
		public Allocation getOut() {
			return out;
		}

		@Override
		public void execute(FilterStack fstack) {
			Log.v("Kamoflage", "Filter.java:feBlend.execute() -- begin");
			Allocation in_al = fstack.getInAllocation(in);
			out = Allocation.createFromBitmap(fstack.rs, fstack.getPureAlpha());
			out.copyFrom(fstack.getInAllocation(in2));

			switch(mode) {
			case 0:
				fstack.blendOp.forEachSrcOver(in_al, out);
				break;
			case 1:
				fstack.blendOp.forEachMultiply(in_al, out);
				break;
			case 2:
				// this is not right - should be "screen"
				fstack.blendOp.forEachMultiply(in_al, out);
				break;
			case 3:
				// this is not right - should be "darken"
				fstack.blendOp.forEachMultiply(in_al, out);
				break;
			case 4:
				// this is not right - should be "lighten"
				fstack.blendOp.forEachMultiply(in_al, out);
				break;
			}
			Log.v("Kamoflage", "Filter.java:feBlend.execute() -- end");
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
		Allocation out;

		@Override
		public Allocation getOut() {
			return out;
		}

		@Override
		public void execute(FilterStack fstack) {
			Log.v("Kamoflage", "Filter.java:feComposite.execute() -- begin");

			Allocation in_al = fstack.getInAllocation(in);
			Log.v("Kamoflage", "Filter.java:feComposite.execute() -- in_al : "
			      + in_al.getBytesSize());
			Allocation in2_al = fstack.getInAllocation(in2);
			Log.v("Kamoflage", "Filter.java:feComposite.execute() -- in2_al : "
			      + in2_al.getBytesSize());
			in2_al.copyTo(fstack.scratch);
			out = Allocation.createFromBitmap(fstack.rs, fstack.scratch);
			Log.v("Kamoflage", "Filter.java:feComposite.execute() -- out (post) : "
			      + out.getBytesSize());

			switch(oprt) {
			case 1:
				fstack.blendOp.forEachSrcOver(in_al, out);
				break;
			case 2:
				fstack.blendOp.forEachSrcIn(in_al, out);
				break;
			case 3:
				fstack.blendOp.forEachSrcOut(in_al, out);
				break;
			case 4:
				fstack.blendOp.forEachSrcAtop(in_al, out);
				break;
			case 5:
				fstack.blendOp.forEachXor(in_al, out);
				break;
			case 6:
				// this is not right - should be "arithmetic"
				fstack.blendOp.forEachMultiply(in_al, out);
				break;
			}
			Log.v("Kamoflage", "Filter.java:feComposite.execute() -- end");
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
		Allocation out;

		@Override
		public Allocation getOut() {
			return out;
		}

		@Override
		public void execute(FilterStack fstack) {
			Log.v("Kamoflage", "Filter.java:feFlood.execute()" +
				      fstack.width + " x " + fstack.height);
			floodBitmap = Bitmap.createBitmap(
				fstack.width,
				fstack.height,
				Bitmap.Config.ARGB_8888);
			floodBitmap.eraseColor((color & 0x00ffffff) | (opacity << 24));
			Log.v("Kamoflage", "Filter.java:feFlood.execute() -- Allocation.createFromBitmap()");
			out = Allocation.createFromBitmap(fstack.rs, floodBitmap);
			Log.v("Kamoflage", "Filter.java:feFlood.execute() -- done");
		}

		int x, y, width, height, in, color;
		int opacity;

		Bitmap floodBitmap;

		feFlood(int _x, int _y, int _width, int _height, int _in,
			int _color, double _opacity) {
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			in = _in;
			color = _color;
			_opacity *= 255.0;
			opacity = (int)_opacity;
		}
	}

	class feGaussianBlur implements feBaseOperation {
		Allocation out;

		@Override
		public Allocation getOut() {
			return out;
		}

		@Override
		public void execute(FilterStack fstack) {
			out = Allocation.createFromBitmap(fstack.rs, fstack.getPureAlpha());

			Log.v("Kamoflage", "Filter.java:feGaussian.execute() -- begin ("
			      + (float)std_dev_x +
			      ")");
			Allocation in_al = fstack.getInAllocation(in);
			out = Allocation.createFromBitmap(fstack.rs, fstack.getPureAlpha());
			fstack.blurOp.setInput(in_al);
			fstack.blurOp.setRadius((float)std_dev_x); // we only support circular blur
			fstack.blurOp.forEach(out);
			Log.v("Kamoflage", "Filter.java:feGaussian.execute() -- end");

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
		Allocation out;

		@Override
		public Allocation getOut() {
			return out;
		}

		@Override
		public void execute(FilterStack fstack) {
			Log.v("Kamoflage", "Filter.java:feOffset.execute() -- begin");
			Allocation in_al = fstack.getInAllocation(in);
			in_al.copyTo(fstack.scratch);
			Bitmap tmp = Bitmap.createBitmap(fstack.width, fstack.height, Bitmap.Config.ARGB_8888);
			tmp.eraseColor(0x00000000);
			Canvas tc = new Canvas(tmp);
			tc.drawBitmap(fstack.scratch, dx, dy, null);
			out = Allocation.createFromBitmap(fstack.rs, tmp);
			Log.v("Kamoflage", "Filter.java:feOffset.execute() -- end");
			// actual offset not implemented yet
		}

		int x, y, width, height, in;
		float dx, dy;

		feOffset(int _x, int _y, int _width, int _height,
			 int _in, double _dx, double _dy) {
			x = _x;
			y = _y;
			width = _width;
			height = _height;
			in = _in;
			dx = (float)_dx;
			dy = (float)_dy;
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
		Log.v("Kamoflage", "Filter.java:Filter.addFilter_feGaussianBlur("
		      + std_dev_x +
		      ", "
		      + std_dev_y +
		      ")");
		if(current != null) {
			if(std_dev_x == 0.0 && std_dev_y > 0.0)
				std_dev_x = std_dev_y;
			else if(std_dev_y == 0.0 && std_dev_x > 0.0)
				std_dev_y = std_dev_x;
			if(std_dev_x == 0.0 || std_dev_y == 0.0) {
				current.add(new feComposite(x, y, width, height,
							    1, in, in,
							    0.0, 0.0, 0.0, 0.0));
			} else {
				current.add(new feGaussianBlur(x, y, width, height, in,
							       std_dev_x, std_dev_y));
			}
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

	public Bitmap execute(Bitmap background, Bitmap source) {
		Log.v("Kamoflage", "Filter.java:Filter.execute()");
		if(current != null) {
			return current.execute(background, source);
		}
		return null;
	}
}
