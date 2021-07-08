/* svg_element.c: Data structures for SVG graphics elements

   Copyright © 2002 USC/Information Sciences Institute

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Carl Worth <cworth@isi.edu>
*/

#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "svgint.h"

//#define __DO_SVG_DEBUG
#include "svg_debug.h"

static svg_element_t __deleted_element_object;
svg_element_t *SVG_DELETED_ELEMENT_OBJECT = &__deleted_element_object;

svgint_status_t
_svg_element_create (svg_element_t	**element,
		     svg_element_type_t	type,
		     svg_element_t	*parent,
		     svg_t		*doc)
{
	*element = calloc(1, sizeof (svg_element_t));
    if (*element == NULL)
	return SVG_STATUS_NO_MEMORY;

    if(StrHmapFindItem(doc->element_ids, element)) {
	    SVG_ERROR("(create) StrHmap already contains element %p\n", *element);
	    free(*element);
	    return SVG_STATUS_NO_MEMORY;
    }


    return _svg_element_init (*element, type, parent, doc);
}

svg_status_t
_svg_element_init (svg_element_t	*element,
		   svg_element_type_t	type,
		   svg_element_t	*parent,
		   svg_t		*doc)
{
    svg_status_t status;

    element->type = type;
    element->parent = parent;
    element->doc = doc;
    element->id = NULL;
    element->ref_count = 0;
    element->do_events = 0;
    element->next_event = NULL;

    element->b_header = 0xbeef;

    status = _svg_transform_init (&element->transform);
    if (status)
	return status;

    status = _svg_style_init_empty (&element->style, doc);
    if (status)
	return status;

    element->overflow = SVG_OVERFLOW_VISIBLE;

    switch (type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
    case SVG_ELEMENT_TYPE_GROUP:
    case SVG_ELEMENT_TYPE_DEFS:
    case SVG_ELEMENT_TYPE_USE:
    case SVG_ELEMENT_TYPE_SYMBOL:
	status = _svg_group_init (&element->e.group);
	break;
    case SVG_ELEMENT_TYPE_PATH:
	status = _svg_path_init (&element->e.path);
	break;
    case SVG_ELEMENT_TYPE_CIRCLE:
    case SVG_ELEMENT_TYPE_ELLIPSE:
	status = _svg_ellipse_init (&element->e.ellipse);
	break;
    case SVG_ELEMENT_TYPE_LINE:
	status = _svg_line_init (&element->e.line);
	break;
    case SVG_ELEMENT_TYPE_RECT:
	status = _svg_rect_init (&element->e.rect);
	break;
    case SVG_ELEMENT_TYPE_TEXT:
	status = _svg_text_init (&element->e.text);
	break;
    case SVG_ELEMENT_TYPE_IMAGE:
	status = _svg_image_init (&element->e.image);
	break;
    case SVG_ELEMENT_TYPE_GRADIENT:
	status = _svg_gradient_init (&element->e.gradient);
	break;
    case SVG_ELEMENT_TYPE_PATTERN:
	status = _svg_pattern_init (&element->e.pattern, parent, doc);
	break;
    case SVG_ELEMENT_TYPE_FILTER:
	status = _svg_filter_init (&element->e.filter);
	break;
    default:
	status = SVGINT_STATUS_UNKNOWN_ELEMENT;
	break;
    }
    if (status)
	return status;

    return SVG_STATUS_SUCCESS;
}

static void _svg_element_delete (svg_element_t *element)
{
    (void) _svg_transform_deinit (&element->transform);
    (void) _svg_style_deinit (&element->style);

    if (element->id) {
	    SVG_DEBUG("_svg_element_delete(%p) - id: %s  (tid: %d)\n", element, element->id, gettid());
	    // remove us from the lookup table
	    (void) StrHmapErase(element->doc->element_ids,
				element->id);
	    free (element->id);
	    element->id = NULL;
    }

    if (element->classes) {
	    int l;
	    for(l = 0; element->classes[l] != NULL; l++) {
		    free(element->classes[l]);
	    }
	    free(element->classes);
    }

    switch (element->type) {
    case SVG_ELEMENT_TYPE_USE:
    case SVG_ELEMENT_TYPE_SVG_GROUP:
    case SVG_ELEMENT_TYPE_GROUP:
    case SVG_ELEMENT_TYPE_DEFS:
    case SVG_ELEMENT_TYPE_SYMBOL:
	    SVG_DEBUG("_svg_element_delete(%p) - will deinit group.\n", element);
	    (void) _svg_group_deinit (&element->e.group);
	break;
    case SVG_ELEMENT_TYPE_PATH:
	    (void) _svg_path_deinit (element->doc, &element->e.path);
	break;
    case SVG_ELEMENT_TYPE_CIRCLE:
    case SVG_ELEMENT_TYPE_ELLIPSE:
    case SVG_ELEMENT_TYPE_LINE:
    case SVG_ELEMENT_TYPE_RECT:
	    (void) SVG_STATUS_SUCCESS;
	break;
    case SVG_ELEMENT_TYPE_TEXT:
	    (void) _svg_text_deinit (&element->e.text);
	    (void) _svg_group_deinit (&element->e.group);
	break;
    case SVG_ELEMENT_TYPE_GRADIENT:
	    (void) _svg_gradient_deinit (&element->e.gradient);
	break;
    case SVG_ELEMENT_TYPE_PATTERN:
	    (void) _svg_pattern_deinit (&element->e.pattern);
	break;
    case SVG_ELEMENT_TYPE_IMAGE:
	    (void) _svg_image_deinit (element->doc, &element->e.image);
	break;
    case SVG_ELEMENT_TYPE_FILTER:
	    (void) _svg_filter_deinit (&element->e.filter);
	break;
    default:
	break;
    }

    SVG_DEBUG("_svg_element_delete(%p) - will free final pointer\n", element);
    free (element);

}

void _svg_element_reference(svg_element_t *element) {
	element->ref_count++;
}

void _svg_element_dereference(svg_element_t *element) {
	if(element->b_header != 0xbeef) {
		SVG_ERROR("Element %p has a corrupted beef header (tid: %d).\n", element, gettid());
		exit(-1);
	}

	element->ref_count--;

	SVG_DEBUG("_svg_element_dereference(%p) - new count: %d\n", element, element->ref_count);
	if(element->ref_count == 0) {
		_svg_element_delete(element);
	}
}

void
_svg_element_get_viewport(svg_element_t *element,
			  svg_length_t *x, svg_length_t *y,
			  svg_length_t *w, svg_length_t *h) {
	if(
		(element->type == SVG_ELEMENT_TYPE_SVG_GROUP)
		||
		(element->type == SVG_ELEMENT_TYPE_SVG_GROUP)
		) {
		SVG_DEBUG("get_viewport, element %s, %f, %f -> %f, %f\n",
			  element->id,
			  element->e.group.x.value,
			  element->e.group.x.value,
			  element->e.group.width.value,
			  element->e.group.height.value);
		*x = element->e.group.x;
		*y = element->e.group.y;
		*w = element->e.group.width;
		*h = element->e.group.height;
	} else if(element->parent != NULL) {
		_svg_element_get_viewport(element->parent, x, y, w, h);
	} else {
		_svg_length_init(x, 0.0);
		_svg_length_init(y, 0.0);
		_svg_length_init(w, 0.0);
		_svg_length_init(h, 0.0);
	}
}

svg_status_t
svg_element_render (svg_element_t		*element,
		    svg_render_engine_t		*engine,
		    void			*closure)
{
	svg_status_t status, fail_status = SVG_STATUS_SUCCESS, return_status = SVG_STATUS_SUCCESS;
    svg_transform_t transform = element->transform;

    /* if the display property is not activated, we dont have to
       draw this element nor its children, so we can safely return here. */
    status = _svg_style_get_display (&element->style);
    if (status)
	return status;

    /* event handling */
    if(
	    element->do_events &&
	    (element->style.flags & SVG_STYLE_FLAG_DISPLAY)
	    ) {
	    element->next_event = element->doc->event_stack;
	    element->doc->event_stack = element;
    }

    if (element->type == SVG_ELEMENT_TYPE_SVG_GROUP
	|| element->type == SVG_ELEMENT_TYPE_GROUP) {

	status = (engine->begin_group) (closure, _svg_style_get_opacity(&element->style));
	if (status)
	    return status;

	/* if element->type == SVG_ELEMENT_TYPE_SVG_GROUP and
	 * the overflow attribute is set to hide or scroll
	 * we must apply a clip box too.
	 */
	if (element->type == SVG_ELEMENT_TYPE_SVG_GROUP) {
		switch(element->overflow) {
		case SVG_OVERFLOW_VISIBLE:
		case SVG_OVERFLOW_AUTO:
		case SVG_OVERFLOW_INHERIT:
			break;
		case SVG_OVERFLOW_HIDDEN:
		case SVG_OVERFLOW_SCROLL:
			status = (engine->apply_clip_box) (closure,
							   &(element->e.group.x),
							   &(element->e.group.y),
							   &(element->e.group.width),
							   &(element->e.group.height));

			break;
		}
	}

    } else {
	    if(element->type == SVG_ELEMENT_TYPE_PATH)
		    status = (engine->begin_element) (closure, element->e.path.cache);
	    else
		    status = (engine->begin_element) (closure, NULL);
	if (status)
	    return status;
    }

    if (element->type == SVG_ELEMENT_TYPE_SVG_GROUP)
    {
	status = (engine->set_viewport_dimension) (closure, &element->e.group.width, &element->e.group.height);
	if (status) {
		fail_status = status;
		goto fail;
	}
    }

    /* perform extra viewBox transform */
    if ((element->type == SVG_ELEMENT_TYPE_SVG_GROUP ||
	element->type == SVG_ELEMENT_TYPE_GROUP) &&
	element->e.group.view_box.aspect_ratio != SVG_PRESERVE_ASPECT_RATIO_UNKNOWN)
    {
	status = (engine->apply_view_box) (closure, element->e.group.view_box,
					   &element->e.group.width, &element->e.group.height);
    }
    /* TODO : this is probably not the right place to change transform, but
     atm we dont store svg_length_t in group, so... */
    if (element->type == SVG_ELEMENT_TYPE_SVG_GROUP ||
        element->type == SVG_ELEMENT_TYPE_USE)
	_svg_transform_add_translate (&transform, element->e.group.x.value, element->e.group.y.value);

    status = _svg_transform_render (&transform, engine, closure);
    if (status) {
	    fail_status = status;
	    goto fail;
    }

    status = _svg_style_render (&element->style, engine, closure);
    if (status) {
	    fail_status = status;
	    goto fail;
    }

    /* If the element doesnt have children, we can check visibility property, otherwise
       the children will have to be processed. */
    if (element->type != SVG_ELEMENT_TYPE_SVG_GROUP &&
	element->type != SVG_ELEMENT_TYPE_GROUP &&
	element->type != SVG_ELEMENT_TYPE_USE)
	status = _svg_style_get_visibility (&element->style);

    if (status == SVG_STATUS_SUCCESS) {
	switch (element->type) {
	case SVG_ELEMENT_TYPE_SVG_GROUP:
	case SVG_ELEMENT_TYPE_GROUP:
	case SVG_ELEMENT_TYPE_USE:
	    status = _svg_group_render (&element->e.group, engine, closure);
	    break;
	case SVG_ELEMENT_TYPE_PATH:
		status = _svg_path_render (&element->e.path, engine, closure, element->doc->do_path_cache);
	    break;
	case SVG_ELEMENT_TYPE_CIRCLE:
	    status = _svg_circle_render (&element->e.ellipse, engine, closure);
	    break;
	case SVG_ELEMENT_TYPE_ELLIPSE:
	    status = _svg_ellipse_render (&element->e.ellipse, engine, closure);
	    break;
	case SVG_ELEMENT_TYPE_LINE:
	    status = _svg_line_render (&element->e.line, engine, closure);
	    break;
	case SVG_ELEMENT_TYPE_RECT:
	    status = _svg_rect_render (&element->e.rect, engine, closure);
	    break;
	case SVG_ELEMENT_TYPE_TEXT:
	    status = _svg_text_render (&element->e.text, engine, closure);
	    break;
	case SVG_ELEMENT_TYPE_IMAGE:
	    status = _svg_image_render (&element->e.image, engine, closure);
	    break;
	case SVG_ELEMENT_TYPE_DEFS:
	    break;
	case SVG_ELEMENT_TYPE_GRADIENT:
	    break; /* Gradients are applied as paint, not rendered directly */
	case SVG_ELEMENT_TYPE_PATTERN:
	    break; /* Patterns are applied as paint, not rendered directly */
	case SVG_ELEMENT_TYPE_SYMBOL:
	    status = _svg_symbol_render (element, engine, closure);
	    break;
	default:
	    status = SVGINT_STATUS_UNKNOWN_ELEMENT;
	    break;
	}
    }
    if (status)
	    fail_status = status;


    (void) engine->get_last_bounding_box(closure, &(element->bounding_box));

fail:
    if (element->type == SVG_ELEMENT_TYPE_SVG_GROUP
	|| element->type == SVG_ELEMENT_TYPE_GROUP) {

	status = (engine->end_group) (closure, _svg_style_get_opacity(&element->style));
	if (status && !return_status)
	    return_status = status;
    } else {
	status = (engine->end_element) (closure);
	if (status && !return_status)
	    return_status = status;
    }

    if(fail_status) {
	    return_status = fail_status;
    }

    return return_status;
}

void svg_element_debug_id_table(svg_element_t *element) {
	StrHmapDump(element->doc->element_ids);
}

void svg_element_debug_render_tree(svg_element_t *element)
{
	svg_status_t status;

	SVG_ERROR("svg_element_debug_render_tree(%p -> %s -- ref: %d)\n", element, element->id, element->ref_count);

	if (element->parent == NULL) {
		SVG_ERROR("  --> element has no parent.\n");
	}

	/* if the display property is not activated, we dont have to
	   draw this element nor its children, so we can safely return here. */
	status = _svg_style_get_display (&element->style);
	if (status) {
		SVG_ERROR("element display parameter is set to none.\n");
		return;
	}

	/* If the element doesnt have children, we can check visibility property, otherwise
	   the children will have to be processed. */
	if (element->type != SVG_ELEMENT_TYPE_SVG_GROUP &&
	    element->type != SVG_ELEMENT_TYPE_GROUP &&
	    element->type != SVG_ELEMENT_TYPE_USE)
		status = _svg_style_get_visibility (&element->style);

	if (status == SVG_STATUS_SUCCESS) {
		switch (element->type) {
		case SVG_ELEMENT_TYPE_SVG_GROUP:
		case SVG_ELEMENT_TYPE_GROUP:
		case SVG_ELEMENT_TYPE_USE:
		{
			int i;
			svg_group_t *group = &element->e.group;
			for (i = 0; i < group->num_elements; i++) {
				svg_element_debug_render_tree(group->element[i]);
			}
		}
			break;
		case SVG_ELEMENT_TYPE_PATH:
			SVG_ERROR("    debug render path.\n");
			break;
		case SVG_ELEMENT_TYPE_CIRCLE:
			SVG_ERROR("    debug render circle.\n");
			break;
		case SVG_ELEMENT_TYPE_ELLIPSE:
			SVG_ERROR("    debug render ellipse.\n");
			break;
		case SVG_ELEMENT_TYPE_LINE:
			SVG_ERROR("    debug render line.\n");
			break;
		case SVG_ELEMENT_TYPE_RECT:
			SVG_ERROR("    debug render rect.\n");
			break;
		case SVG_ELEMENT_TYPE_TEXT:
			SVG_ERROR("    debug render text.\n");
			break;
		case SVG_ELEMENT_TYPE_IMAGE:
			SVG_ERROR("    debug render image.\n");
			break;
		case SVG_ELEMENT_TYPE_DEFS:
			SVG_ERROR("    debug render defs.\n");
			break;
		case SVG_ELEMENT_TYPE_GRADIENT:
			SVG_ERROR("    debug render gradient.\n");
			break; /* Gradients are applied as paint, not rendered directly */
		case SVG_ELEMENT_TYPE_PATTERN:
			SVG_ERROR("    debug render pattern.\n");
			break; /* Patterns are applied as paint, not rendered directly */
		case SVG_ELEMENT_TYPE_SYMBOL:
			SVG_ERROR("    debug render symbol.\n");
			break;
		default:
			SVG_ERROR("    debug render unknown.\n");
			break;
		}
	} else {
		SVG_ERROR("element visibility property disabled.\n");
	}
}

svg_status_t
_svg_element_get_nearest_viewport (svg_element_t *element, svg_element_t **viewport)
{
    svg_element_t *elem = element;
    *viewport = NULL;

    while (elem && !*viewport)
    {
	if (elem->type == SVG_ELEMENT_TYPE_SVG_GROUP)
	    *viewport = elem;
	elem = elem->parent;
    }
    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_parse_aspect_ratio (const char *aspect_ratio_str,
				 svg_view_box_t *view_box)
{
	const char *start;//, *end;
    if (strlen (aspect_ratio_str) < 8)
	return SVG_STATUS_SUCCESS;

    if (strncmp(aspect_ratio_str, "xMinYMin", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMINYMIN;
    else if (strncmp (aspect_ratio_str, "xMidYMin", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN;
    else if (strncmp (aspect_ratio_str, "xMaxYMin", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN;
    else if (strncmp (aspect_ratio_str, "xMinYMid", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMINYMID;
    else if (strncmp (aspect_ratio_str, "xMidYMid", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMIDYMID;
    else if (strncmp (aspect_ratio_str, "xMaxYMid", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMAXYMID;
    else if (strncmp (aspect_ratio_str, "xMinYMax", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMINYMAX;
    else if (strncmp (aspect_ratio_str, "xMidYMax", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX;
    else if (strncmp (aspect_ratio_str, "xMaxYMax", 8) == 0)
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMAXYMAX;
    else
	view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_NONE;

    start = aspect_ratio_str + 8;
//    end = aspect_ratio_str + strlen (aspect_ratio_str) + 1;

	_svg_str_skip_space (&start);

    if (strcmp (start, "meet") == 0)
	view_box->meet_or_slice = SVG_MEET_OR_SLICE_MEET;
    else if (strcmp (start, "slice") == 0)
	view_box->meet_or_slice = SVG_MEET_OR_SLICE_SLICE;
    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_parse_view_box (const char	*view_box_str,
			     double	*x,
			     double	*y,
			     double	*width,
			     double	*height)
{
    const char *s;
    const char *end;

    s = view_box_str;
    *x = _svg_ascii_strtod (s, &end);
    if (end == s)
	return SVG_STATUS_PARSE_ERROR;

    s = end;
    _svg_str_skip_space_or_char (&s, ',');
    *y = _svg_ascii_strtod (s, &end);
    if (end == s)
	return SVG_STATUS_PARSE_ERROR;

    s = end;
    _svg_str_skip_space_or_char (&s, ',');
    *width = _svg_ascii_strtod (s, &end);
    if (end == s)
	return SVG_STATUS_PARSE_ERROR;

    s = end;
    _svg_str_skip_space_or_char (&s, ',');
    *height = _svg_ascii_strtod (s, &end);
    if (end == s)
	return SVG_STATUS_PARSE_ERROR;

    return SVG_STATUS_SUCCESS;
}

static inline int count_segments(const char *str, const char *delimiters) {
	int k, kount = 0;
	int skipping_delimiter = 1;
	for(k = 0; str[k] != '\0'; k++) {
		if(strchr(delimiters, str[k]) == NULL) {
			if(skipping_delimiter) {
				kount++;
				skipping_delimiter = 0;
			}
		} else {
			skipping_delimiter = -1;
		}
	}
	return kount;
}

void _svg_attribute_apply_class(svg_element_t *element, const char *_class_string) {
	if(element->classes) {
		int l;
		for(l = 0; element->classes[l] != NULL; l++) {
			free(element->classes[l]);
		}
		free(element->classes);
	}
	char *class_string_head = strdup(_class_string); // only used temporary

	int k_max = count_segments(class_string_head, " \t");
	if(k_max) {
		element->classes = (char **)calloc(k_max + 1, sizeof(char *));
		if(element->classes) {
			char *segment;
			int k = 0;

			char *class_string = class_string_head;
			segment = strsep(&class_string, " \t");
			while(segment) {
				element->classes[k++] = strdup(segment);
				segment = strsep(&class_string, " \t");
			}
		}
	}

	free(class_string_head);
}

svg_status_t
_svg_element_apply_attributes (svg_element_t	*element,
			       const char	**attributes)
{
    svg_status_t status = 0;
    const char *id, *overflow, *class_string;

    status = _svg_transform_apply_attributes (&element->transform, attributes);
    if (status)
	return status;

    status = _svg_style_apply_attributes (&element->style, attributes);
    if (status)
	return status;

    _svg_attribute_get_string (attributes, "id", &id, NULL);
    if (id)
	element->id = strdup (id);

    _svg_attribute_get_string (attributes, "overflow", &overflow, NULL);
    if (overflow) {
	    if(strcmp("visible", overflow) == 0)
		    element->overflow = SVG_OVERFLOW_VISIBLE;
	    if(strcmp("hidden", overflow) == 0)
		    element->overflow = SVG_OVERFLOW_HIDDEN;
	    if(strcmp("scroll", overflow) == 0)
		    element->overflow = SVG_OVERFLOW_SCROLL;
	    if(strcmp("auto", overflow) == 0)
		    element->overflow = SVG_OVERFLOW_AUTO;
	    if(strcmp("inherit", overflow) == 0)
		    element->overflow = SVG_OVERFLOW_INHERIT;
    }

    _svg_attribute_get_string (attributes, "class", &class_string, NULL);
    if (class_string) {
	    _svg_attribute_apply_class(element, class_string);
    } else {
	    element->classes = NULL;
    }

    switch (element->type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
	status = _svg_group_apply_svg_attributes (&element->e.group, attributes);
	if (status)
	    return status;
	/* fall-through */
    case SVG_ELEMENT_TYPE_GROUP:
	status = _svg_group_apply_group_attributes (&element->e.group, attributes);
	break;
    case SVG_ELEMENT_TYPE_SYMBOL:
	status = _svg_group_apply_svg_attributes (&element->e.group, attributes);
	break;
    case SVG_ELEMENT_TYPE_USE:
	status = _svg_group_apply_use_attributes (element, attributes);
	break;
    case SVG_ELEMENT_TYPE_PATH:
	status = _svg_path_apply_attributes (&element->e.path, attributes);
	break;
    case SVG_ELEMENT_TYPE_RECT:
    case SVG_ELEMENT_TYPE_CIRCLE:
    case SVG_ELEMENT_TYPE_ELLIPSE:
    case SVG_ELEMENT_TYPE_LINE:
	break;
    case SVG_ELEMENT_TYPE_TEXT:
	status = _svg_text_apply_attributes (&element->e.text, attributes);
	break;
    case SVG_ELEMENT_TYPE_IMAGE:
	status = _svg_image_apply_attributes (&element->e.image, attributes);
	break;
    case SVG_ELEMENT_TYPE_GRADIENT:
	status = _svg_gradient_apply_attributes (&element->e.gradient,
						 element->doc, attributes);
	break;
    case SVG_ELEMENT_TYPE_PATTERN:
	status = _svg_pattern_apply_attributes (&element->e.pattern, attributes);
	break;
    case SVG_ELEMENT_TYPE_FILTER:
	status = SVG_STATUS_SUCCESS;
	break;
    default:
	status = SVGINT_STATUS_UNKNOWN_ELEMENT;
	break;
    }

    if (status)
	return status;

    return SVG_STATUS_SUCCESS;
}

void _svg_element_set_display(svg_element_t *element, const char *value) {
	if(element == NULL || value == NULL) return;

	_svg_style_parse_display(&(element->style), value);
}

void _svg_element_set_style(svg_element_t *element, const char *value) {
	if(element == NULL || value == NULL) return;

	_svg_style_parse_style_str(&(element->style), value);
}

svg_pattern_t *
svg_element_pattern (svg_element_t *element)
{
    if (element->type != SVG_ELEMENT_TYPE_PATTERN)
	return NULL;

    return &element->e.pattern;
}

svg_status_t _svg_element_init_copy (
	const char *new_id,
	svg_element_t   *element,
	svg_element_t   *other) {
	svg_status_t status;

	memcpy(element, other, sizeof(svg_element_t));

	element->ref_count = 0;
	element->type   = other->type;
	element->parent = NULL;
	if(new_id) {
		element->id = strdup(new_id);
	} else {
		element->id = NULL;
	}

	if(other->classes) {
		size_t kount = 1;
		int l;

		for(l = 0; other->classes[l] != NULL; l++) {
			kount++;
		}

		element->classes = (char **)calloc(kount, sizeof(char *));

		for(l = 0; other->classes[l] != NULL; l++) {
			element->classes[l] = strdup(other->classes[l]);
		}
	}

	element->transform = other->transform;

	status = _svg_style_init_copy (&element->style, &other->style);
	if (status)
		return status;

	switch (other->type) {
	case SVG_ELEMENT_TYPE_SVG_GROUP:
	case SVG_ELEMENT_TYPE_GROUP:
	case SVG_ELEMENT_TYPE_DEFS:
	case SVG_ELEMENT_TYPE_USE:
	case SVG_ELEMENT_TYPE_SYMBOL:
		status = _svg_group_init_copy (&element->e.group, &other->e.group);
		break;
	case SVG_ELEMENT_TYPE_PATH:
		status = _svg_path_init_copy (&element->e.path, &other->e.path);
		break;
	case SVG_ELEMENT_TYPE_CIRCLE:
	case SVG_ELEMENT_TYPE_ELLIPSE:
		status = _svg_ellipse_init_copy (&element->e.ellipse, &other->e.ellipse);
		break;
	case SVG_ELEMENT_TYPE_LINE:
		status = _svg_line_init_copy (&element->e.line, &other->e.line);
		break;
	case SVG_ELEMENT_TYPE_RECT:
		status = _svg_rect_init_copy (&element->e.rect, &other->e.rect);
		break;
	case SVG_ELEMENT_TYPE_TEXT:
		status = _svg_text_init_copy (&element->e.text, &other->e.text);
		status = _svg_group_init_copy (&element->e.group, &other->e.group);
		break;
	case SVG_ELEMENT_TYPE_GRADIENT:
		status = _svg_gradient_init_copy (&element->e.gradient, &other->e.gradient);
		break;
	case SVG_ELEMENT_TYPE_PATTERN:
		status = _svg_pattern_init_copy (&element->e.pattern, &other->e.pattern);
		break;
	case SVG_ELEMENT_TYPE_IMAGE:
		status = _svg_image_init_copy (&element->e.image, &other->e.image);
		break;
	default:
		status = SVGINT_STATUS_UNKNOWN_ELEMENT;
		break;
	}
	if (status)
		return status;

	return SVG_STATUS_SUCCESS;
}

svg_status_t _svg_element_clone(
	const char *new_id,
	svg_element_t       **element,
	svg_element_t       *other) {
	*element = calloc(1, sizeof (svg_element_t));
	if (*element == NULL) {
		return SVG_STATUS_NO_MEMORY;
	}

	if(_svg_element_init_copy (new_id, *element, other) != SVG_STATUS_SUCCESS) {
		free(*element);
		return SVG_STATUS_INVALID_CALL;
	}

	if ((*element)->id)
		_svg_store_element_by_id ((*element)->doc, *element);

	return  SVG_STATUS_SUCCESS;
}

svg_status_t _svg_inject_clone(const char *new_id, svg_element_t *group, svg_element_t *element_to_clone) {
	svg_element_t *clone;

	// check that the ID is unique - otherwise fail
	(void) _svg_fetch_element_by_id(group->doc, new_id, &clone);
	if(clone) {
		SVG_ERROR("could not create unique ID for clone (%s).\n", new_id);
		return SVG_STATUS_INVALID_CALL;
	}

	// OK, all good so far.. proceed with cloning and injection
	if(_svg_element_clone(new_id, &clone, element_to_clone) == SVG_STATUS_SUCCESS) {
		SVG_DEBUG("   inject clone %p (%s) into group %p (clone ref count: %d, parent ref count: %d)\n", clone, new_id, group, clone->ref_count, group->ref_count);

		clone->parent = group;
		_svg_group_add_element(&(group->e.group), clone);

		return SVG_STATUS_SUCCESS;
	}

	return SVG_STATUS_INVALID_CALL;
}
