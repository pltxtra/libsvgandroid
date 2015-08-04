/* libsvg - Library for parsing/rendering SVG documents

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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <zlib.h>
#include <sys/param.h>

//#define __DO_SVG_DEBUG
#include "svg_debug.h"

#include "svgint.h"

#include <android/log.h>

static svg_status_t
_svg_init (svg_t *svg);

svg_status_t
svg_create (svg_t **svg)
{
    *svg = malloc (sizeof (svg_t));
    if (*svg == NULL) {
	return SVG_STATUS_NO_MEMORY;
    }

    return _svg_init (*svg);
}

void svg_enable_path_cache(svg_t *svg) {
	svg->do_path_cache = 1;
}

static svg_status_t
_svg_init (svg_t *svg)
{
    svg->dpi = 100;

    svg->dir_name = strdup (".");

    svg->group_element = NULL;

    _svg_parser_init (&svg->parser, svg);

    svg->engine = NULL;

    svg->element_ids = StrHmapAlloc(100);

    svg->do_path_cache = 0;
    
    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_deinit (svg_t *svg)
{
    free (svg->dir_name);
    svg->dir_name = NULL;

    if (svg->group_element)
	_svg_element_dereference (svg->group_element);

    _svg_parser_deinit (&svg->parser);

    svg->engine = NULL;

    StrHmapFree(svg->element_ids);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_destroy (svg_t *svg)
{
    svg_status_t status;
    status = _svg_deinit (svg);
    free (svg);

    return status;
}

#define SVG_PARSE_BUFFER_SIZE (8 * 1024)

svg_status_t
svg_parse_file (svg_t *svg, FILE *file)
{
    svg_status_t status = SVG_STATUS_SUCCESS;
    gzFile zfile;
    char buf[SVG_PARSE_BUFFER_SIZE];
    int read;

    zfile = gzdopen (dup(fileno(file)), "r");
    if (zfile == NULL) {
	switch (errno) {
	case ENOMEM:
	    return SVG_STATUS_NO_MEMORY;
	case ENOENT:
	    return SVG_STATUS_FILE_NOT_FOUND;
	default:
	    return SVG_STATUS_IO_ERROR;
	}
    }

    status = svg_parse_chunk_begin (svg);
    if (status)
	goto CLEANUP;

    while (! gzeof (zfile)) {
	read = gzread (zfile, buf, SVG_PARSE_BUFFER_SIZE);
	if (read > -1) {
	    status = svg_parse_chunk (svg, buf, read);
	    if (status)
		goto CLEANUP;
	} else {
	    status = SVG_STATUS_IO_ERROR;
	    goto CLEANUP;
	}
    }

    status = svg_parse_chunk_end (svg);

 CLEANUP:
    gzclose (zfile);
    return status;
}

svg_status_t
svg_parse (svg_t *svg, const char *filename)
{
    svg_status_t status = SVG_STATUS_SUCCESS;
    FILE *file;
    char *tmp;

    free (svg->dir_name);
    /* awful dirname semantics require some hoops */
    tmp = strdup (filename);
    svg->dir_name = strdup (dirname (tmp));
    free (tmp);

    file = fopen (filename, "r");
    if (file == NULL) {
	switch (errno) {
	case ENOMEM:
	    return SVG_STATUS_NO_MEMORY;
	case ENOENT:
	    return SVG_STATUS_FILE_NOT_FOUND;
	default:
	    return SVG_STATUS_IO_ERROR;
	}
    }
    status = svg_parse_file (svg, file);
    fclose (file);
    return status;
}

svg_status_t
svg_parse_buffer (svg_t *svg, const char *buf, size_t count)
{
    svg_status_t status;

    status = svg_parse_chunk_begin (svg);
    if (status)
	return status;

    status = svg_parse_chunk (svg, buf, count);
    if (status)
	return status;

    status = svg_parse_chunk_end (svg);

    return status;
}

svg_status_t
svg_parse_buffer_and_inject (svg_t *svg, svg_element_t *parent, const char *buf, size_t count)
{
	svg_status_t status;

	if((parent->type == SVG_ELEMENT_TYPE_SVG_GROUP) ||
	   (parent->type == SVG_ELEMENT_TYPE_GROUP)) {
	
		status = _svg_parser_begin (&svg->parser);
		if (status)
			return status;

		status = _svg_parser_spoof_state(&svg->parser, parent);
		if(status)
			return status;
		
		status = _svg_parser_parse_chunk (&svg->parser, buf, count);

		status = _svg_parser_unspoof_state(&svg->parser);
		
		if (status)
			return status;
		
		status = _svg_parser_end (&svg->parser);
	} else {
		status = SVG_STATUS_INVALID_CALL;
	}
		
	return status;
}

svg_status_t
svg_drop_element(svg_t *svg, svg_element_t *element) {
	if(!element) {
		return SVG_STATUS_INVALID_CALL;
	}

	if(element->parent) {
		if(element->parent->type != SVG_ELEMENT_TYPE_USE
		   &&
		   element->parent->type != SVG_ELEMENT_TYPE_SVG_GROUP
		   &&
		   element->parent->type != SVG_ELEMENT_TYPE_GROUP
		   &&
		   element->parent->type != SVG_ELEMENT_TYPE_DEFS
		   &&
		   element->parent->type != SVG_ELEMENT_TYPE_SYMBOL
			) {
			SVG_ERROR("Trying to drop element where parent is not a proper group type. Parent pointing to corrupted memory? Element %p, ref count %d\n", element, element->ref_count);
			exit(0);
		}
		SVG_DEBUG("svg_drop_element %p -> ref count before: %d\n", element, element->ref_count);	
		return _svg_group_drop_element(&(element->parent->e.group), element);
	}

	return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_parse_chunk_begin (svg_t *svg)
{
    return _svg_parser_begin (&svg->parser);
}

svg_status_t
svg_parse_chunk (svg_t *svg, const char *buf, size_t count)
{
    return _svg_parser_parse_chunk (&svg->parser, buf, count);
}

svg_status_t
svg_parse_chunk_end (svg_t *svg)
{
    return _svg_parser_end (&svg->parser);
}

void
svg_element_enable_events(svg_element_t *element) {
	element->do_events = 1;
}

svg_element_t *
svg_event_coords_match(svg_t *svg, int x, int y) {
	svg_element_t *current = svg->event_stack;

	SVG_DEBUG("----> event_coords_match (%d, %d)\n", x, y);
	while(current != NULL) {
		SVG_DEBUG("     current (%p) (%d, %d) -> (%d, %d)\n",
			  current,
			  current->bounding_box.left, current->bounding_box.top,
			  current->bounding_box.right, current->bounding_box.bottom);
		if(x > current->bounding_box.left &&
		   x < current->bounding_box.right &&
		   y > current->bounding_box.top &&
		   y < current->bounding_box.bottom) {
			SVG_DEBUG("  coords matched to %p.\n", current);
			return current;
		}
		current = current->next_event;
	}
	SVG_DEBUG("     no match.\n");
	return NULL;
}
	
svg_status_t
svg_render (svg_t		*svg,
	    svg_render_engine_t	*engine,
	    void		*closure)
{
    svg_status_t status;
    char orig_dir[MAXPATHLEN];

    if (svg->group_element == NULL)
	return SVG_STATUS_SUCCESS;

    svg->event_stack = NULL; // reset the event stack
    
    /* XXX: Currently, the SVG parser doesn't resolve relative URLs
       properly, so I'll just cheese things in by changing the current
       directory -- at least I'll be nice about it and restore it
       afterwards. */

    getcwd (orig_dir, MAXPATHLEN);
    chdir (svg->dir_name);
    
    status = svg_element_render (svg->group_element, engine, closure);

    chdir (orig_dir);

    return status;
}

svg_status_t
_svg_store_element_by_id (svg_t *svg, svg_element_t *element)
{
    StrHmapInsert(svg->element_ids,
		  element->id,
		  element);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_fetch_element_by_id (svg_t *svg, const char *id, svg_element_t **element_ret)
{
	if(id != NULL) {
		*element_ret = StrHmapFind(svg->element_ids, id);
	} else {
		// get root element
		*element_ret = svg->group_element; 
	}
	return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_fetch_element_by_class (svg_t *svg, const char *class_id, svg_element_t *element, svg_element_t **element_ret) {
	if(element == NULL) {
		element = svg->group_element;
	}
	if(element == NULL)
		return SVG_STATUS_INVALID_CALL;


	switch(element->type) {
	case SVG_ELEMENT_TYPE_SVG_GROUP:
	case SVG_ELEMENT_TYPE_GROUP:
	{
		int k;
		for(k = 0; k < element->e.group.num_elements; k++) {
			svg_element_t *e = element->e.group.element[k];
			if(e->classes) {
				int l;
				for(l = 0; e->classes[l] != NULL; l++) {
					if(strcmp(e->classes[l], class_id) == 0) {
						*element_ret = e;
						return SVG_STATUS_SUCCESS;
					}
				}
				
			}
			if(_svg_fetch_element_by_class(svg, class_id, e, element_ret) == SVG_STATUS_SUCCESS) {
				return SVG_STATUS_SUCCESS;
			}
		}
	}
	break;
	case SVG_ELEMENT_TYPE_DEFS:
	case SVG_ELEMENT_TYPE_USE:
	case SVG_ELEMENT_TYPE_SYMBOL:
	case SVG_ELEMENT_TYPE_PATH:
	case SVG_ELEMENT_TYPE_CIRCLE:
	case SVG_ELEMENT_TYPE_ELLIPSE:
	case SVG_ELEMENT_TYPE_LINE:
	case SVG_ELEMENT_TYPE_RECT:
	case SVG_ELEMENT_TYPE_TEXT:
	case SVG_ELEMENT_TYPE_GRADIENT:
	case SVG_ELEMENT_TYPE_GRADIENT_STOP:
	case SVG_ELEMENT_TYPE_PATTERN:
	case SVG_ELEMENT_TYPE_IMAGE:
		break;
	}
	
	return SVG_STATUS_NO_SUCH_ELEMENT;
}

void
svg_get_size (svg_t *svg, svg_length_t *width, svg_length_t *height)
{
    if (svg->group_element) {
	_svg_group_get_size (&svg->group_element->e.group, width, height);
    } else {
	_svg_length_init (width, 0.0);
	_svg_length_init (height, 0.0);
    }
}
