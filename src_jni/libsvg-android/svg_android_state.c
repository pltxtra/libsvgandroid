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

#include <stdlib.h>
#include <string.h>

#include "svg-android-internal.h"

#include <android/log.h>

//#define DEBUG_LIBSVG_ANDROID
#ifdef DEBUG_LIBSVG_ANDROID
#define DEBUG_ENTRY(s) __android_log_print(ANDROID_LOG_INFO, "libsvg-android", "Entering function %s\n", s); fflush(0)
#define DEBUG_ANDROID(s) __android_log_print(ANDROID_LOG_INFO, "libsvg-android", "%s", s)
#define DEBUG_ANDROID1(s,A) __android_log_print(ANDROID_LOG_INFO, "libsvg-android", s, A)
#define DEBUG_EXIT(s) __android_log_print(ANDROID_LOG_INFO, "libsvg-android", "Leaving function %s\n", s); fflush(0)
#else
#define DEBUG_ENTRY(s)
#define DEBUG_ANDROID(s)
#define DEBUG_ANDROID1(s,A)
#define DEBUG_EXIT(s)
#endif

static svg_android_state_t *state_store = NULL;
static size_t state_store_level = 0, state_store_depth = 0;

static void clear_state(svg_android_state_t *state) {
	memset(state, 0, sizeof(svg_android_state_t));
	state->matrix = NULL;
	state->paint = NULL;
	state->path = NULL;
	state->state_path = NULL;
}

static int dig_stack_deeper() {
	size_t new_store_depth = 2 * state_store_depth;

	if(new_store_depth == 0) new_store_depth = 10; // default

	svg_android_state_t *new_stack = NULL;
		
	if(state_store == NULL) {
		new_stack = (svg_android_state_t *)calloc(sizeof(svg_android_state_t), new_store_depth);
	} else {	
		new_stack = (svg_android_state_t *)realloc(state_store, sizeof(svg_android_state_t) * new_store_depth);
	}

	if(new_stack == NULL) return -1;

	int k;
	for(k = state_store_depth; k < new_store_depth; k++) {
		clear_state(&new_stack[k]);
	}

	state_store = new_stack;
	state_store_depth = new_store_depth;

	return 0;
}

static svg_android_status_t pop_state_store(svg_android_state_t **_state, svg_android_t *svg_android) {
	svg_android_state_t *state = NULL;
	if(state_store_level == state_store_depth)
		if(dig_stack_deeper()) return SVG_ANDROID_STATUS_NO_MEMORY; // failure
	
	state = &state_store[state_store_level++];
	
	/******** set initial values, the missing stuff is filled in by copy and/or init *****/
	
	/* trust libsvg to set all of these to reasonable defaults:
	   state->fill_paint;
	   state->stroke_paint;
	   state->fill_opacity;
	   state->stroke_opacity;
	*/
	state->instance = svg_android;

	state->offscreen_bitmap = NULL;
	state->saved_canvas = NULL;

	state->font_family = NULL;
	state->font_size = 1.0;
	state->font_style = SVG_FONT_STYLE_NORMAL;
	state->font_weight = 400;
	state->font_dirty = 1;

	state->bounding_box.left = -1;
	state->bounding_box.top = -1;
	state->bounding_box.right = 0;
	state->bounding_box.bottom = 0;	
	
	state->dash = NULL;
	state->num_dashes = 0;
	state->dash_offset = 0;

	state->opacity = 1.0;

	state->bbox = 0;

	state->text_anchor = SVG_TEXT_ANCHOR_START;

	state->next = NULL;

	if(state->matrix == NULL) {
		state->matrix = ANDROID_IDENTITY_MATRIX(state->instance);
		state->matrix =
			(*(state->instance->env))->NewGlobalRef(
				state->instance->env, state->matrix);
	} else {
		ANDROID_MATRIX_RESET(state->instance, state->matrix);
	}

	if(state->paint == NULL) {
		state->paint = ANDROID_PAINT_CREATE(state->instance);
		state->paint =
			(*(state->instance->env))->NewGlobalRef(
				state->instance->env, state->paint);
	} else {
		DEBUG_ANDROID1("   paint before reset %p", state->paint);
		ANDROID_PAINT_RESET(state->instance, state->paint);
		DEBUG_ANDROID1("   paint after reset %p", state->paint);
	}

	if(state->state_path == NULL) {
		state->state_path = ANDROID_PATH_CREATE(state->instance);
		state->state_path =
			(*(state->instance->env))->NewGlobalRef(
				state->instance->env, state->state_path);
		DEBUG_ANDROID1("   new path created %p", state->state_path);
	} else {
		DEBUG_ANDROID1("   path before clear %p", state->state_path);
		ANDROID_PATH_CLEAR(state->instance, state->state_path);
		DEBUG_ANDROID1("   path after clear %p", state->state_path);
	}
	state->path = state->state_path;
	
	*_state = state;
	
	return SVG_ANDROID_STATUS_SUCCESS;
}

static svg_android_status_t push_state_store() {
	state_store_level--;
	if(state_store_level < 0) {
		DEBUG_ANDROID1("     state store level failure: %d", state_store_level);
		state_store_level = 0;
		return SVG_ANDROID_STATUS_INVALID_CALL;
	}

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_android_status_t
_svg_android_state_init (svg_android_state_t *state)
{	
	DEBUG_ANDROID(" ");
	DEBUG_ANDROID("-----------------------------");
	DEBUG_ANDROID1("        init for state pointer %p", state);
	DEBUG_ANDROID1("Created global refs for path at %p", state->path);
	DEBUG_ANDROID1("Created global refs for paint at %p", state->paint);
	DEBUG_ANDROID1("Created global refs for matrix at %p", state->matrix);
	
	ANDROID_SET_ANTIALIAS(state->instance, state->paint, state->instance->do_antialias);
	
	// this might already be set by copy
	if(state->font_family == NULL) {
		state->font_family = strdup (SVG_ANDROID_FONT_FAMILY_DEFAULT);
		if (state->font_family == NULL)
			return SVG_ANDROID_STATUS_NO_MEMORY;
	}
	
	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_android_status_t
_svg_android_state_init_copy (svg_android_state_t *state, const svg_android_state_t *other)
{
	if(other == NULL) // Nothing to copy => don't copy
		return SVG_ANDROID_STATUS_SUCCESS;

	// remember matrix and paint (they will be overwritten otherwise)
	jobject state_matrix = state->matrix;
	jobject state_paint = state->paint;
	jobject state_path = state->path;
	jobject state_state_path = state->state_path;
	
	// copy all fields as-is
	*state = *other;

	// restore matrix and paint
	state->matrix = state_matrix;
	state->paint = state_paint;
	state->path = state_path;
	state->state_path = state_state_path;

	// the bounding box should be reset always
	state->bounding_box.left = -1;
	state->bounding_box.top = -1;
	state->bounding_box.right = 0;
	state->bounding_box.bottom = 0;	
	
	/* We don't need our own child_surface or saved cr at this point. */
	state->offscreen_bitmap = NULL;
	state->saved_canvas = NULL;

	// copy paint
	ANDROID_PAINT_SET(state->instance, state->paint, other->paint);

	// copy matrix
	ANDROID_MATRIX_SET(state->instance, state->matrix, other->matrix);

	DEBUG_ANDROID("-----------------------------");
	DEBUG_ANDROID1("COPY created global refs for paint at %p", state->paint);
	DEBUG_ANDROID1("COPY created global refs for matrix at %p", state->matrix);

	// We need to duplicate the string
	if (other->font_family)
		state->font_family = strdup ((char *) other->font_family);

	// XXX anton: are these not copied already?!
	state->viewport_width = other->viewport_width;
	state->viewport_height = other->viewport_height;

	// Create a copy of the dash 
	if (other->dash) {
		state->dash = malloc (state->num_dashes * sizeof(double));
		if (state->dash == NULL)
			return SVG_ANDROID_STATUS_NO_MEMORY;
		memcpy (state->dash, other->dash, state->num_dashes * sizeof(double));
	}

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_android_status_t
_svg_android_state_deinit (svg_android_state_t *state)
{
	DEBUG_ANDROID("-----------------------------");
	DEBUG_ANDROID1("FREEING global refs for path at %p", state->path);

	if (state->offscreen_bitmap) {
		state->offscreen_bitmap = NULL;
	}

	if (state->saved_canvas) {
		state->saved_canvas = NULL;
	}

	if (state->font_family) {
		free (state->font_family);
		state->font_family = NULL;
	}

	if (state->dash) {
		free (state->dash);
		state->dash = NULL;
	}

	state->next = NULL;

	return SVG_ANDROID_STATUS_SUCCESS;
}

svg_android_status_t
_svg_android_state_destroy (svg_android_state_t *state)
{
	_svg_android_state_deinit (state);

	(*(state->instance->env))->PopLocalFrame(state->instance->env, NULL);	

	return push_state_store();
}

svg_android_state_t *
_svg_android_state_push (svg_android_t *instance, svg_android_state_t *state, jobject path_cache)
{
	svg_android_state_t *new;

	// create basic state
	if(pop_state_store(&new, instance) != SVG_ANDROID_STATUS_SUCCESS)
		return NULL;	

	DEBUG_ANDROID1("   path just after pop_state_store %p", new->path);

	// if pushing from a previous state, copy relevant data (this will do nothing if state == NULL)
	_svg_android_state_init_copy (new, state);

	DEBUG_ANDROID1("   path just after init_copy %p", new->path);

	// initialize the rest
	_svg_android_state_init (new);

	// if path_cache is defined, use it
	if(path_cache)
		new->path = path_cache;
	
	DEBUG_ANDROID1("   path just after init %p", new->path);

	// point to next
	new->next = state;
	
	DEBUG_ANDROID("-------STATE PUSH!! ----------------------");
	DEBUG_ANDROID1("RETURNING global refs for path at %p", new->path);
	DEBUG_ANDROID1("RETURNING global refs for state_path at %p", new->state_path);
	DEBUG_ANDROID1("RETURNING global refs for paint at %p", new->paint);
	DEBUG_ANDROID1("RETURNING global refs for matrix at %p", new->matrix);
	DEBUG_ANDROID1("     state store level %d", state_store_level);
	DEBUG_ANDROID ("     ");

	(*(instance->env))->PushLocalFrame(instance->env, 32);

	return new;
}

svg_android_state_t *
_svg_android_state_pop (svg_android_state_t *state)
{
	svg_android_state_t *next;

	if (state == NULL)
		return NULL;

	DEBUG_ANDROID("-------STATE POP!! ----------------------");
	DEBUG_ANDROID1("CLEARING global refs for path at %p", state->path);
	DEBUG_ANDROID1("CLEARING global refs for paint at %p", state->paint);
	DEBUG_ANDROID1("CLEARING global refs for matrix at %p", state->matrix);
	if(state->next) {
		DEBUG_ANDROID1("   next global refs for path at %p", state->next->path);
		DEBUG_ANDROID1("   next global refs for paint at %p", state->next->paint);
		DEBUG_ANDROID1("   next global refs for matrix at %p", state->next->matrix);
	}
	DEBUG_ANDROID1("     state store level %d", state_store_level);
	DEBUG_ANDROID ("     ");

	next = state->next;

	if(next != NULL)
		_svg_android_update_bounding_box(&(next->bounding_box), &(state->bounding_box));
	
	(void) _svg_android_state_destroy (state);

	return next;
}
