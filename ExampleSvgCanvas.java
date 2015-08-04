/*
 * Example usage of libsvgandroid
 * Written by Anton Persson
 *
 * This exmple file is in the public domain
 *
 * The rest of libsvgandroid is licensed under lgpl v3+
 *
 */

package com.examplesvg;

import android.util.Log;
import android.view.View;
import android.graphics.Canvas;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.StringBuilder;
import java.io.BufferedReader;
import android.content.Context;
import javax.xml.parsers.*;
import org.w3c.dom.*;
import com.toolkits.libsvgandroid.*;

class ExampleSvgCanvas extends View {
	private int width, height;

	private long svgId;

	// We must first load the svgandroid native library
	static {
		System.loadLibrary("svgandroid");
	}

	public HelloSvgCanvas(Context context) {
		super(context);

		InputStream is;
		is = this.getResources().openRawResource(R.raw.hellosvg);

		try {
			StringBuilder sb = new StringBuilder();
			String line;
			BufferedReader reader = new BufferedReader(new InputStreamReader(is, "UTF-8"));
			while ((line = reader.readLine()) != null) {
				sb.append(line).append("\n");
			}
			svgId = SvgRaster.svgAndroidCreate();
			SvgRaster.svgAndroidParseBuffer(svgId, sb.toString());
			SvgRaster.svgAndroidSetAntialiasing(svgId, true);
		} catch(java.io.UnsupportedEncodingException e) {
		} catch(java.io.IOException e) {
		}
	}

	@Override
	protected void onLayout (boolean changed, int left, int top, int right, int bottom) {
		// get visible area
		width = right - left;
		height = bottom - top;
	}

	@Override
	public void onDraw(Canvas canvas) {
		// paint a white background...
		canvas.drawARGB(255,255,255,255);
		// OK, rasterize the SVG data
//		SvgRaster.svgAndroidRender(svgId, canvas);
		int k;
		for(k = 1; k < 6; k++)
			SvgRaster.svgAndroidRenderToArea(
				svgId, canvas,
				k * 30, k * 30, 80 * k, 80 * k);
	}
}
