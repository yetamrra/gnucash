/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

/*
 * The Gnucash Header Canvas
 *
 * Author:
 *     Heath Martin <martinh@pegasus.cc.ucf.edu>
 */

#include "gnucash-sheet.h"
#include "gnucash-color.h"
#include "gnucash-style.h"
#include "gnucash-grid.h"
#include "gnucash-cursor.h"
#include "gnucash-item-edit.h"

#include "gnucash-header.h"

static GnomeCanvasItem *gnucash_header_parent_class;

enum {
        ARG_0,
        ARG_SHEET,  /*  the sheet this header is associated with */
        ARG_CURSOR_TYPE,    /* the type of the current cursor */
};


static void
gnucash_header_update (GnomeCanvasItem *item, double *affine,
		       ArtSVP *clip_path, int flags)
{
        if (GNOME_CANVAS_ITEM_CLASS(gnucash_header_parent_class)->update)
                (*GNOME_CANVAS_ITEM_CLASS(gnucash_header_parent_class)->update)
			(item, affine, clip_path, flags);

        item->x1 = 0;
        item->y1 = 0;
        item->x2 = INT_MAX;
        item->y2 = INT_MAX;
}


static void
gnucash_header_draw (GnomeCanvasItem *item, GdkDrawable *drawable,
                     int x, int y, int width, int height)
{
        GnucashHeader *header = GNUCASH_HEADER(item);
        SheetBlockStyle *style = header->style;
        SheetBlockStyle *header_style;
        int i, j;
        int xpaint, ypaint;
        int w = 0, h = 0;
        gchar *text;
        GdkFont *font;

        header_style = header->sheet->cursor_style[GNUCASH_CURSOR_HEADER];

        /* Assume all cells have the same color */
        gdk_gc_set_foreground(header->gc,
                              header_style->inactive_bg_color[0][0]);
        gdk_draw_rectangle(drawable, header->gc, TRUE, 0, 0, width, height);

        gdk_gc_set_line_attributes (header->gc, 1, GDK_LINE_SOLID, -1, -1);
        gdk_gc_set_foreground (header->gc, &gn_black);
        gdk_draw_rectangle (drawable, header->gc, FALSE, -x, -y,
                            style->dimensions->width,
                            style->dimensions->height);
        gdk_draw_line (drawable, header->gc, -x,
                       style->dimensions->height - 1,
                       style->dimensions->width - 1,
                       style->dimensions->height - 1);

        gdk_gc_set_line_attributes (header->gc, 1, GDK_LINE_SOLID, -1, -1);
        gdk_gc_set_background (header->gc, &gn_white);
        gdk_gc_set_foreground (header->gc, &gn_black);
        font = style->header_font;

        ypaint = -y;

        for (i = 0; i < style->nrows; i++) {
                xpaint = -x;

                /* TODO: This routine is duplicated in several places.
                   Can we abstract at least the cell drawing routine?
                   That way we'll be sure everything is drawn
                   consistently, and cut down on maintenance issues. */
                for (j = 0; j < style->ncols; j++) {
                        if (header->in_resize && (j == header->resize_col))
                                w = header->resize_col_width;
                        else
                                w = style->dimensions->pixel_widths[i][j];
                        
                        h = style->dimensions->pixel_heights[i][j];
                        
                        gdk_draw_rectangle (drawable, header->gc, FALSE,
                                            xpaint, ypaint, w, h);
                        
                        text = style->labels[i][j];
                        
                        if (text) {
                                gint x_offset, y_offset;
                                GdkRectangle rect;

                                y_offset = h - MAX(CELL_VPADDING,
                                                   font->descent + 4);

                                switch (style->alignments[i][j]) {
                                default:
                                case GTK_JUSTIFY_LEFT:
                                case GTK_JUSTIFY_FILL:
                                case GTK_JUSTIFY_CENTER:
                                        x_offset = CELL_HPADDING;
                                        break;
                                case GTK_JUSTIFY_RIGHT:
                                        x_offset = w - CELL_HPADDING
                                                - gdk_string_measure(font,
                                                                     text);
                                        break;
                                }

                                rect.x = xpaint + CELL_HPADDING;
                                rect.y = ypaint + CELL_VPADDING;
                                rect.width = MAX(0, w - 2*CELL_HPADDING);
                                rect.height = h;
                                
                                gdk_gc_set_clip_rectangle (header->gc, &rect);
                                
                                gdk_draw_string (drawable,
                                                 font,
                                                 header->gc,
                                                 xpaint + x_offset,
                                                 ypaint + y_offset,
                                                 text);
                                gdk_gc_set_clip_rectangle (header->gc, NULL);
                                
                        }
                        
                        xpaint += w;
                }
                ypaint += h;
        }
}


static void
gnucash_header_request_redraw (GnucashHeader *header)
{
        GnomeCanvas *canvas = GNOME_CANVAS_ITEM(header)->canvas;

        gnome_canvas_request_redraw (canvas, 0, 0, INT_MAX, INT_MAX);
}


static void
gnucash_header_realize (GnomeCanvasItem *item)
{
        GnucashHeader *header = GNUCASH_HEADER (item);
        GdkWindow *window;

        	
        if (GNOME_CANVAS_ITEM_CLASS (gnucash_header_parent_class)->realize)
                (*GNOME_CANVAS_ITEM_CLASS
		 (gnucash_header_parent_class)->realize)(item);

        window = GTK_WIDGET (item->canvas)->window;
        
        header->gc = gdk_gc_new (window);
}


static void
gnucash_header_unrealize (GnomeCanvasItem *item)
{
        GnucashHeader *header = GNUCASH_HEADER (item);

        if (header->gc != NULL) {
                gdk_gc_unref (header->gc);
                header->gc = NULL;
        }

        if (header->resize_cursor != NULL)
                gdk_cursor_destroy (header->resize_cursor);
        header->resize_cursor = NULL;

        if (header->normal_cursor != NULL)
                gdk_cursor_destroy (header->normal_cursor);
        header->normal_cursor = NULL;

        if (GNOME_CANVAS_ITEM_CLASS (gnucash_header_parent_class)->unrealize)
                (*GNOME_CANVAS_ITEM_CLASS
		 (gnucash_header_parent_class)->unrealize)(item);
}


static void
gnucash_header_destroy (GtkObject *object)
{
        if (GTK_OBJECT_CLASS (gnucash_header_parent_class)->destroy)
                (*GTK_OBJECT_CLASS
		 (gnucash_header_parent_class)->destroy)(object);
}


void
gnucash_header_reconfigure (GnucashHeader *header)
{
        int w, h;

        GnomeCanvas *canvas;
        GtkWidget *widget;
        GnucashSheet *sheet;
        SheetBlockStyle *old_style;

        g_return_if_fail (header != NULL);
        g_return_if_fail (GNUCASH_IS_HEADER (header));
        
        canvas = GNOME_CANVAS_ITEM(header)->canvas;
        widget = GTK_WIDGET (header->sheet);
        sheet = GNUCASH_SHEET(header->sheet);
        old_style = header->style;

        header->style = header->sheet->cursor_style[header->type];

        if (header->style == NULL)
                return;

        /* Check for a valid header row. This can be invalid during
           arg setting. */
        if (header->row < 0 || header->row >= header->style->nrows)
                return;

        sheet->width = header->style->dimensions->width;

        w = MAX (widget->allocation.width, header->style->dimensions->width);
        h = header->style->dimensions->height;

        if (header->height != h || header->width != w ||
            header->style != old_style) {
                header->height = h;
                header->width = w;

                gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas),
					       0, 0, w, h);
                gtk_widget_set_usize (GTK_WIDGET(canvas), -1, h);

                gnucash_header_request_redraw (header);
        }
}


static double
gnucash_header_point (GnomeCanvasItem *item,
                      double x, double y, int cx, int cy,
                      GnomeCanvasItem **actual_item)
{
        *actual_item = item;
        return 0.0;
}

/*
 *  Returns FALSE if pointer not on a resize line, else returns
 *  TRUE. Returns the index of the column to the left in the col
 *  argument.
 */
static gboolean
pointer_on_resize_line (GnucashHeader *header, int x, int y, int *col)
{
        SheetBlockStyle *style = header->style;
        gboolean on_the_line = FALSE;
        int pixels = 0; /* = style->dimensions->pixel_widths[header->row][0];*/
        int j;

        for (j = 0; j < style->ncols; j++) {
                pixels += style->dimensions->pixel_widths[header->row][j];
                if (x >= pixels - 1 && x <= pixels + 1)
                        on_the_line = TRUE;
                if (x <= pixels + 1)
                        break;
        }

        if (col != NULL)
                *col = j;

        return on_the_line;
}


static int
find_resize_col (GnucashHeader *header, int col)
{
        int start = col;
        SheetBlockStyle *style = header->style;

        if (col < 0 || col >= style->ncols)
                return -1;

        /* skip to the right over zero-width columns */
        while (col+1 < style->ncols &&
               style->dimensions->pixel_widths[0][col+1] == 0)
                col++;

        /* now go back left till we have a resizable column */
        while (col >= start) {
                if (gnucash_style_col_is_resizable (style, col))
                        return col;
                else
                        col--;
        }

        /* didn't find a resizable column to the right of col */
        return -1;
}

static void
gnucash_header_auto_resize_column (GnucashHeader *header, gint col)
{
        GnucashSheet *sheet = header->sheet;

        int width = gnucash_sheet_col_max_width (sheet, 0, col);

        gnucash_sheet_style_set_col_width (sheet, header->style,
                                           col, width, FALSE);

        gtk_window_set_default_size(GTK_WINDOW(sheet->window), 0, 0); 

        gnucash_cursor_configure (GNUCASH_CURSOR(sheet->cursor));
        item_edit_configure (ITEM_EDIT(sheet->item_editor));

        gnucash_sheet_update_adjustments (sheet);

        gnucash_header_request_redraw (header);
        gnucash_sheet_redraw_all (sheet);
}

static gint
gnucash_header_event (GnomeCanvasItem *item, GdkEvent *event)
{
        GnucashHeader *header = GNUCASH_HEADER(item);
        GnomeCanvas *canvas = item->canvas;
        int x, y;
        int col;
        
        switch (event->type) {
        case GDK_MOTION_NOTIFY:

                gnome_canvas_w2c (canvas, event->motion.x, event->motion.y,
                                  &x, &y);

                if (header->in_resize) {
                        int change = x - header->resize_x;
                        int new_width;

                        if (!header->needs_ungrab) {
                                gnome_canvas_item_grab (item,
                                                        GDK_POINTER_MOTION_MASK |
                                                        GDK_BUTTON_RELEASE_MASK,
                                                        header->resize_cursor,
                                                        event->button.time);
                                header->needs_ungrab = TRUE;
                        }

                        new_width = header->resize_col_width + change;

                        if (new_width >= 0) {
                                header->resize_x = x;
                                header->resize_col_width = new_width;
                                gnucash_header_request_redraw (header);
                        }

                        break;
                }

                if (pointer_on_resize_line(header, x, y, &col) &&
                    gnucash_style_col_is_resizable (header->style, col))
                        gdk_window_set_cursor (GTK_WIDGET(canvas)->window,
                                               header->resize_cursor);
                else
                        gdk_window_set_cursor (GTK_WIDGET(canvas)->window,
                                               header->normal_cursor);
                break;

        case GDK_BUTTON_PRESS: 
        {
                int col;

                if (event->button.button != 1)
                        break;

                gnome_canvas_w2c (canvas, event->button.x, event->button.y,
                                  &x, &y);

                if (pointer_on_resize_line (header, x, y, &col))
                        col = find_resize_col (header, col);
                else
                        col = -1;

                if (col > -1) {
                        header->in_resize = TRUE;
                        header->resize_col = col;
                        header->resize_col_width = header->style->dimensions->pixel_widths[header->row][col];
                        header->resize_x = x;
                }

                break;
        }
        case GDK_BUTTON_RELEASE: 
        {
                GnucashSheet *sheet = header->sheet;

                if (event->button.button != 1)
                        break;

                gnome_canvas_w2c (canvas, event->button.x, event->button.y,
                                  &x, &y);

                if (header->in_resize) {
                        if (header->needs_ungrab) {
                                gnome_canvas_item_ungrab (item, event->button.time);
                                header->needs_ungrab = FALSE;

                                gnucash_sheet_style_set_col_width (sheet, header->style,
                                                                   header->resize_col, header->resize_col_width, FALSE);

                                gtk_window_set_default_size(GTK_WINDOW(sheet->window), 0, 0);

                                gnucash_cursor_configure (GNUCASH_CURSOR(sheet->cursor));
                                item_edit_configure (ITEM_EDIT(sheet->item_editor));

                                gnucash_sheet_update_adjustments (sheet);

                                gnucash_header_request_redraw (header);
                                gnucash_sheet_redraw_all (sheet);
                        }
                        header->in_resize = FALSE;
                        header->resize_col = -1;
                }

                break;
        }

        case GDK_2BUTTON_PRESS:
        {
                gboolean on_line;
                int ptr_col;
                int resize_col;

                if (event->button.button != 1)
                        break;

                gnome_canvas_w2c (canvas, event->button.x, event->button.y,
                                  &x, &y);

                on_line = pointer_on_resize_line (header, x, y, &ptr_col);
                resize_col = find_resize_col (header, ptr_col);

                if ((resize_col > -1) &&
                    (on_line || (resize_col == ptr_col))) {
                        header->in_resize = FALSE;
                        header->resize_col = -1;
                        if (header->needs_ungrab) {
                                gnome_canvas_item_ungrab (item, event->button.time);
                                header->needs_ungrab = FALSE;
                        }

                        gnucash_header_auto_resize_column (header, resize_col);
                }

        }
        break;

        default:
                break;
        }

        return TRUE;
}


static void
gnucash_header_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
        GnucashHeader *header;
        GtkLayout *layout;
        gint needs_update = FALSE;

        header = GNUCASH_HEADER (o);
        layout = GTK_LAYOUT(GNOME_CANVAS_ITEM(header)->canvas);

        switch (arg_id){
        case ARG_SHEET:
                header->sheet = GTK_VALUE_POINTER (*arg);
                gtk_layout_set_hadjustment (layout, header->sheet->hadj);
                needs_update = TRUE;
                break;
        case ARG_CURSOR_TYPE: 
        {
                gint old_type = header->type;
                
                header->type = GTK_VALUE_INT (*arg);
                needs_update = (old_type != header->type);
                break;
        }
        default:
                break;
        }

        if ((header->sheet != NULL) && needs_update)
                gnucash_header_reconfigure (header);
}


static void
gnucash_header_init (GnucashHeader *header)
{
        header->sheet = NULL;
        header->in_resize = FALSE;
        header->resize_col = -1;
        header->resize_cursor = gdk_cursor_new (GDK_SB_H_DOUBLE_ARROW);
        header->normal_cursor = NULL;
        header->height = 20;
}


static void
gnucash_header_class_init (GnucashHeaderClass *header_class)
{
        GtkObjectClass  *object_class;
        GnomeCanvasItemClass *item_class;

        gnucash_header_parent_class =
		gtk_type_class(gnome_canvas_item_get_type());

        object_class = (GtkObjectClass *) header_class;
        item_class = (GnomeCanvasItemClass *) header_class;

        gtk_object_add_arg_type ("GnucashHeader::sheet", GTK_TYPE_POINTER,
                                 GTK_ARG_WRITABLE, ARG_SHEET);
        gtk_object_add_arg_type ("GnucashHeader::cursor_type", GTK_TYPE_INT,
                                 GTK_ARG_WRITABLE, ARG_CURSOR_TYPE);

        object_class->set_arg = gnucash_header_set_arg;
        object_class->destroy = gnucash_header_destroy;

        item_class->realize   = gnucash_header_realize;
        item_class->unrealize = gnucash_header_unrealize;
        item_class->update    = gnucash_header_update;
        item_class->draw      = gnucash_header_draw;
        item_class->event     = gnucash_header_event;
        item_class->point     = gnucash_header_point;
}


GtkType
gnucash_header_get_type (void)
{
        static GtkType gnucash_header_type = 0;

        if (!gnucash_header_type) {
                GtkTypeInfo gnucash_header_info = {
                        "GnucashHeader",
                        sizeof (GnucashHeader),
                        sizeof (GnucashHeaderClass),
                        (GtkClassInitFunc) gnucash_header_class_init,
                        (GtkObjectInitFunc) gnucash_header_init,
                        NULL, /* reserved_1 */
                        NULL, /* reserved_2 */
                        (GtkClassInitFunc) NULL
                };

                gnucash_header_type =
			gtk_type_unique(gnome_canvas_item_get_type (),
					&gnucash_header_info);
        }

        return gnucash_header_type;
}


static void
gnucash_header_realized (GtkWidget *widget, gpointer data)
{
       	gdk_window_set_back_pixmap (GTK_LAYOUT (widget)->bin_window,
				    NULL, FALSE);
}


GtkWidget *
gnucash_header_new (GnucashSheet *sheet)
{
        GnomeCanvasGroup *group;
        GnomeCanvasItem *item;
        GtkWidget *canvas;

        canvas = gnome_canvas_new ();

        gtk_signal_connect (GTK_OBJECT (canvas), "realize",
                            (GtkSignalFunc) gnucash_header_realized,
                            NULL);

        group = GNOME_CANVAS_GROUP (GNOME_CANVAS (canvas)->root);

        item = gnome_canvas_item_new (group,
                                      gnucash_header_get_type (),
                                      "GnucashHeader::sheet", sheet,
                                      "GnucashHeader::cursor_type",
                                      GNUCASH_CURSOR_HEADER,
                                      NULL);

        sheet->header_item = item;

        gtk_widget_show (canvas);
        return canvas;
}


/*
  Local Variables:
  c-basic-offset: 8
  End:
*/