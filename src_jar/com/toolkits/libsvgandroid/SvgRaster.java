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

public class SvgRaster {

	public native static long svgAndroidCreate();
	public native static void svgAndroidDestroy(long id);

	public native static int svgAndroidParseBuffer(long id, String bfr);

	public native static int svgAndroidParseChunkBegin(long id);
	public native static int svgAndroidParseChunk(long id, String bfr);
	public native static int svgAndroidParseChunkEnd(long id);

	public native static int svgAndroidSetAntialiasing(long id, boolean doIt);

	public native static int svgAndroidRender(long id, Canvas target);
	public native static int svgAndroidRenderToArea(long id, Canvas target, int x, int y, int w, int h);

	public native static void svgAndroidSetBoundingBox(
		boolean is_in_clip,
		int left, int top, int right, int bottom);

	public static void debugMatrix(Matrix m) {
		Log.v("Kamoflage", m.toString());
	}

	public static void setTypeface(
		Paint p, String family, int weight_n_slant, float textSize, int talign) {
		int style = android.graphics.Typeface.NORMAL;
		Paint.Align al = Paint.Align.LEFT;

		switch(weight_n_slant) {
		case 0: // italic, bold
			style = android.graphics.Typeface.BOLD_ITALIC;
			break;
		case 1: // italic, no bold
			style = android.graphics.Typeface.ITALIC;
			break;
		case 2: // normal, bold
			style = android.graphics.Typeface.BOLD;
			break;
		default:
		case 3: // normal, no bold
			style = android.graphics.Typeface.NORMAL;
			break;
		}

		switch(talign) {
		default:
		case 0:
			al = Paint.Align.LEFT;
			break;
		case 1:
			al = Paint.Align.CENTER;
			break;
		case 2:
			al = Paint.Align.RIGHT;
			break;
		}

		Typeface tf = Typeface.create(family, style);

		p.setTypeface(tf);
		p.setTextSize(textSize);
		p.setTextAlign(al);
	}

	public static Matrix matrixInvert(Matrix m) {
		Matrix r = new Matrix();

		if(m.invert(r)) return r;
		return null;
	}

	public static Shader createBitmapShader(Bitmap b) {
	       return new BitmapShader(b, Shader.TileMode.REPEAT, Shader.TileMode.REPEAT);
	}

	public static Shader createLinearGradient(float x1, float y1, float x2, float y2,
						  int[] colors, float[] offsets, int spreadType) {
		Shader.TileMode tm = Shader.TileMode.REPEAT;
		switch(spreadType) {
		case 0:
			tm = Shader.TileMode.REPEAT;
			break;
		case 1:
			tm = Shader.TileMode.MIRROR;
			break;
		case 2:
			tm = Shader.TileMode.CLAMP;
			break;
		}
		LinearGradient ret = null;

		try {
			ret = new LinearGradient(x1, y1, x2, y2, colors, offsets, tm);
		} catch(java.lang.IllegalArgumentException iae) { /* ignore */ }

		return ret;
	}

	public static Shader createRadialGradient(float x, float y, float r,
						  int[] colors, float[] offsets, int spreadType) {
		Shader.TileMode tm = Shader.TileMode.REPEAT;
		switch(spreadType) {
		case 0:
			tm = Shader.TileMode.REPEAT;
			break;
		case 1:
			tm = Shader.TileMode.MIRROR;
			break;
		case 2:
			tm = Shader.TileMode.CLAMP;
			break;
		}

		RadialGradient ret = null;
		try {
			ret = new RadialGradient(x, y, r, colors, offsets, tm);
		} catch(java.lang.IllegalArgumentException iae) { /* ignore */ }

		return ret;
	}

	public static float[] getBounds(Path p) {
		float[] rv = {(float)0.0, (float)0.0, (float)0.0, (float)0.0};

		RectF r = new RectF();
		p.computeBounds(r, false);
		rv[0] = r.left;
		rv[1] = r.top;
		rv[2] = r.right;
		rv[3] = r.bottom;

		return rv;
	}

	public static Matrix matrixCreate(float xx, float yx, float xy, float yy, float x0, float y0) {
		android.graphics.Matrix x = new android.graphics.Matrix();
		float[] val = {
			(float)xx,  (float)xy,  (float)x0,
			(float)yx,  (float)yy,  (float)y0,
			(float)0.0, (float)0.0, (float)1.0
		};
		x.setValues(val);
		return x;
	}

	public static void matrixInit(Matrix x, float xx, float yx, float xy, float yy, float x0, float y0) {
		float[] val = {
			(float)xx,  (float)xy,  (float)x0,
			(float)yx,  (float)yy,  (float)y0,
			(float)0.0, (float)0.0, (float)1.0
		};
		x.setValues(val);
	}

	public static void setPaintStyle(Paint p, boolean isStroke) {
		p.setStyle(isStroke ? Paint.Style.STROKE : Paint.Style.FILL);
	}

	public static void setStrokeCap(Paint p, int cap) {
		switch(cap) {
		case 0:
			p.setStrokeCap(android.graphics.Paint.Cap.BUTT);
			break;
		case 1:
			p.setStrokeCap(android.graphics.Paint.Cap.ROUND);
			break;
		case 2:
		default:
			p.setStrokeCap(android.graphics.Paint.Cap.SQUARE);
			break;
		}
	}

	public static void setStrokeJoin(Paint p, int cap) {
		switch(cap) {
		case 0:
			p.setStrokeJoin(android.graphics.Paint.Join.MITER);
			break;
		case 1:
			p.setStrokeJoin(android.graphics.Paint.Join.ROUND);
			break;
		case 2:
		default:
			p.setStrokeJoin(android.graphics.Paint.Join.BEVEL);
			break;
		}
	}

	public static void setFillRule(Path p, boolean even_odd) {
		if(even_odd)
			p.setFillType(Path.FillType.EVEN_ODD);
		else
			p.setFillType(Path.FillType.WINDING);
	}

	// using this makes SvgRaster non-thread safe... boohoo.. :(
	// but it will also make us use the GC much less..
	private static RectF static_rect = new RectF(0.0f, 0.0f, 1.0f, 1.0f);
	private static RectF static_rect2 = new RectF(0, 0, 1, 1);
	private static Rect static_rect2i = new Rect(0, 0, 1, 1);
	private static Matrix static_matrix = new Matrix();

	private static Canvas current_screen_canvas = null;
	private static float current_screen_canvas_offset_x, current_screen_canvas_offset_y;

	public static void setCurrentScreenCanvas(Canvas cv, float off_x, float off_y) {
		current_screen_canvas = cv;
		current_screen_canvas_offset_x = off_x;
		current_screen_canvas_offset_y = off_y;
//		Log.v("Kamoflage", "   off_y: " + off_y);
	}

	private static void clipAndSetBoundingBox(Canvas c, RectF r) {
		// is_in_clip defaults to true, so that if there is an EMPTY clip
		// we will still have a valid bounding box..
		boolean is_in_clip = true;

		c.getMatrix(static_matrix);
		/* ignore result */ static_matrix.mapRect(r);

		if(c.getClipBounds(static_rect2i)) {
			static_rect2.left = (float)static_rect2i.left;
			static_rect2.top = (float)static_rect2i.top;
			static_rect2.right = (float)static_rect2i.right;
			static_rect2.bottom = (float)static_rect2i.bottom;
			/* ignore result */ static_matrix.mapRect(static_rect2);
/*
			Log.v("Kamoflage", "                r(" + r.left + ", " + r.top + ", " + r.right + ", " + r.bottom + ")");
			Log.v("Kamoflage", "           stat_r(" + static_rect2.left + ", " + static_rect2.top + ", " + static_rect2.right + ", " + static_rect2.bottom + ")");*/
			is_in_clip = r.intersect(
				(float)static_rect2.left,
				(float)static_rect2.top,
				(float)static_rect2.right,
				(float)static_rect2.bottom
				);
			//Log.v("Kamoflage", "       -- is_in_clip? " + is_in_clip);
		}
		if(c == current_screen_canvas) {
			svgAndroidSetBoundingBox(is_in_clip,
						 (int)r.left - (int)current_screen_canvas_offset_x,
						 (int)r.top - (int)current_screen_canvas_offset_y,
						 (int)r.right - (int)current_screen_canvas_offset_x,
						 (int)r.bottom - (int)current_screen_canvas_offset_y);
		} else {
			svgAndroidSetBoundingBox(is_in_clip,
						 (int)r.left,
						 (int)r.top,
						 (int)r.right,
						 (int)r.bottom);
		}
	}

	public static void getBoundingBox(Path p, Canvas c) {
		p.computeBounds(static_rect, true);

//		Log.v("Kamoflage", "***** getBoundingBox for path.");
		clipAndSetBoundingBox(c, static_rect);
	}

	public static void drawEllipse(Canvas c, Paint p, float cx, float cy, float rx, float ry) {
		float l, t, r, b;

		l = cx - rx;
		t = cy - ry;
		r = cx + rx;
		b = cy + ry;

		static_rect.set(l, t, r, b);
		c.drawOval(static_rect, p);

		clipAndSetBoundingBox(c, static_rect);
	}

	public static void drawRect(Canvas c, Paint p,
				    float x, float y, float w, float h,
				    float rx, float ry) {
		static_rect.set(x, y, x + w, y + h);
		c.drawRoundRect(static_rect, rx, ry, p);

//		Log.v("Kamoflage", "***** getBoundingBox for rect.");
		clipAndSetBoundingBox(c, static_rect);
	}

	public static Bitmap data2bitmap(int w, int h, int[] data) {
		return Bitmap.createBitmap(data, w, h, Bitmap.Config.ARGB_8888);
	}

	public static Bitmap createBitmap(int w, int h) {
		return Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
	}

}
