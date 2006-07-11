#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include <gtk/gtk.h>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <vector>
#include "s-basis.h"
#include "point.h"
#include "point-ops.h"
#include "point-fns.h"
#include "interactive-bits.h"
#include "bezier-to-sbasis.h"
#include "sbasis-to-bezier.h"
#include "path.h"
#include "path-cairo.h"
#include <iterator>
#include "multidim-sbasis.h"
#include "path-builder.h"

using std::string;
using std::vector;

static GtkWidget *canvas;


BezOrd z0(0.5,1.);

std::vector<Geom::Point> handles;
Geom::Point *selected_handle;
Geom::Point old_handle_pos;
Geom::Point old_mouse_point;

unsigned total_pieces;

void draw_sb(cairo_t *cr, multidim_sbasis<2> const &B) {
    cairo_move_to(cr, point_at(B, 0));
    for(int ti = 1; ti <= 30; ti++) {
        double t = (double(ti))/(30);
        cairo_line_to(cr, point_at(B, t));
    }
}


// mutating
void subpath_from_sbasis(Geom::SubPath &sp, multidim_sbasis<2> const &B, double tol) {
    if(B.tail_error(2) < tol || B.size() == 2) { // nearly cubic enough
        std::vector<Geom::Point> e = sbasis_to_bezier(B, 2);
        //reverse(e.begin(), e.end());
        if(sp.handles.empty())
            sp.handles.push_back(e[0]);
        else
            if(sp.handles.back() != e[0])
                std::cout << sp.handles.back() << " vs " << e[0] << std::endl;
        sp.handles.insert(sp.handles.begin(), e.begin()+1, e.end());
        sp.cmd.push_back(Geom::cubicto);
    } else {
        subpath_from_sbasis(sp, compose(B, BezOrd(0, 0.5)), tol);
        subpath_from_sbasis(sp, compose(B, BezOrd(0.5, 1)), tol);
    }
}

void draw_cb(cairo_t *cr, multidim_sbasis<2> const &B) {
    std::vector<Geom::Point> bez = sbasis2_to_bezier(B, 2);
    /*for(int i = 0; i < bez.size(); i++) {
        std::ostringstream notify;
        notify << i;
        draw_cross(cr, bez[i]);
        cairo_move_to(cr, bez[i]);
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 10;
        pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        pango_cairo_show_layout(cr, layout);
        }*/
    cairo_move_to(cr, bez[0]);
    cairo_curve_to(cr, bez[1], bez[2], bez[3]);
    //copy(bez.begin(), bez.end(), std::ostream_iterator<Geom::Point>(std::cout, ", "));
    //std::cout << std::endl;
}

void draw_offset(cairo_t *cr, multidim_sbasis<2> const &B, double dist) {
    multidim_sbasis<2> Bp;
    const int N = 2;
    total_pieces += N;
    for(int subdivi = 0; subdivi < N; subdivi++) {
        double dsubu = 1./N;
        double subu = dsubu*subdivi;
        for(int dim = 0; dim < 2; dim++) {
            Bp[dim] = compose(B[dim], BezOrd(subu, dsubu+subu));
        }
        draw_handle(cr, Geom::Point(Bp[0].point_at(1), Bp[1].point_at(1)));
    
        multidim_sbasis<2> dB;
        SBasis arc;
        dB = derivative(Bp);
        arc = dot(dB, dB);
        
        double err = 0;
        for(int i = 1; i < arc.size(); i++)
            err += fabs(Hat(arc[i]));
        double le = fabs(arc[0][0]) - err;
        double re = fabs(arc[0][1]) - err;
        err /= std::max(arc[0][0], arc[0][1]);
        if(err > 0.5) {
            draw_offset(cr, Bp, dist);
        } else {
            arc = sqrt(arc, 2);
    
            multidim_sbasis<2> offset;
    
            for(int dim = 0; dim < 2; dim++) {
                double sgn = dim?-1:1;
                offset[dim] = Bp[dim] + divide(dist*sgn*dB[1-dim],arc, 2);
            }
            //draw_sb(cr, offset);
            Geom::SubPath sp;
            sp.closed = false;
            subpath_from_sbasis(sp, offset, 0.1);
            cairo_sub_path(cr, sp);
            draw_cb(cr, offset);
        
        }
    }
}

static gboolean
expose_event(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    cairo_t *cr = gdk_cairo_create (widget->window);
    
    int width = 256;
    int height = 256;
    std::ostringstream notify;
    gdk_drawable_get_size(widget->window, &width, &height);
    
    cairo_set_source_rgba (cr, 0., 0.5, 0, 1);
    cairo_set_line_width (cr, 1);
    for(int i = 0; i < handles.size(); i++) {
        std::ostringstream notify;
        notify << i;
        draw_circ(cr, handles[i]);
        cairo_move_to(cr, handles[i]);
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 10;
        pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        pango_cairo_show_layout(cr, layout);
    }
    cairo_set_source_rgba (cr, 0., 0., 0, 0.8);
    cairo_set_line_width (cr, 0.5);
    for(int i = 1; i < 4; i+=2) {
        cairo_move_to(cr, 0, i*height/4);
        cairo_line_to(cr, width, i*height/4);
        cairo_move_to(cr, i*width/4, 0);
        cairo_line_to(cr, i*width/4, height);
    }
    //cairo_move_to(cr, handles[0]);
    //cairo_curve_to(cr, handles[1], handles[2], handles[3]);
    //cairo_stroke(cr);
    
    SBasis Sq = BezOrd(0, 0.03);
    Sq = multiply(Sq, Sq);
    multidim_sbasis<2> B = bezier_to_sbasis<2, 3>(handles.begin());
    //B = compose(Sq, B);
    draw_cb(cr, B);
    //draw_sb(cr, B);
    total_pieces = 0;
    for(int i = 5; i < 5; i++) {
        draw_offset(cr, B, 10*i);
        draw_offset(cr, B, -10*i);
        }
        notify << "total pieces = " << total_pieces; 
    
    cairo_set_source_rgba (cr, 0., 0.125, 0, 1);
    cairo_stroke(cr);
    
    
    cairo_set_source_rgba (cr, 0., 0.5, 0, 0.8);
    /*arc = integral(arc);
    arc = arc - BezOrd(Hat(arc.point_at(0)));
    for(int ti = 0; ti <= 30; ti++) {
        double t = (double(ti))/(30);
        double x = width*t;
        double y = height - (arc.point_at(t));
        if(ti)
            cairo_line_to(cr, x, y);
        else
            cairo_move_to(cr, x, y);
    }
    cairo_stroke(cr);
    notify << "arc length = " << arc.point_at(1) - arc.point_at(0) << std::endl;*/
    {
        PangoLayout* layout = pango_cairo_create_layout (cr);
        pango_layout_set_text(layout, 
                              notify.str().c_str(), -1);

        PangoFontDescription *font_desc = pango_font_description_new();
        pango_font_description_set_family(font_desc, "Sans");
        const int size_px = 10;
        pango_font_description_set_absolute_size(font_desc, size_px * 1024.0);
        pango_layout_set_font_description(layout, font_desc);
        PangoRectangle logical_extent;
        pango_layout_get_pixel_extents(layout,
                                       NULL,
                                       &logical_extent);
        cairo_move_to(cr, 0, height-logical_extent.height);
        pango_cairo_show_layout(cr, layout);
    }
    cairo_destroy (cr);
    
    return TRUE;
}

static void handle_mouse(GtkWidget* widget) {
    gtk_widget_queue_draw (widget);
}

static gint mouse_motion_event(GtkWidget* widget, GdkEventMotion* e, gpointer data) {
    Geom::Point mouse(e->x, e->y);
    
    if(e->state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK)) {
        if(selected_handle) {
            *selected_handle = mouse - old_handle_pos;
            
        }
        handle_mouse(widget);
    }

    if(e->state & (GDK_BUTTON2_MASK)) {
        gtk_widget_queue_draw(widget);
    }
    
    old_mouse_point = mouse;

    return FALSE;
}

static gint mouse_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
    Geom::Point mouse(e->x, e->y);
    if(e->button == 1 || e->button == 3) {
        for(int i = 0; i < handles.size(); i++) {
            if(Geom::L2(mouse - handles[i]) < 5) {
                selected_handle = &handles[i];
                old_handle_pos = mouse - handles[i];
            }
        }
        handle_mouse(window);
    } else if(e->button == 2) {
        gtk_widget_queue_draw(window);
    }
    old_mouse_point = mouse;

    return FALSE;
}

static gint mouse_release_event(GtkWidget* window, GdkEventButton* e, gpointer data) {
    selected_handle = 0;
    return FALSE;
}

static gint key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer) {
    gint ret = FALSE;
    if (event->keyval == ' ') {
        ret = TRUE;
    } else if (event->keyval == 'l') {
        ret = TRUE;
    } else if (event->keyval == 'q') {
        exit(0);
    }

    if (ret) {
        gtk_widget_queue_draw(widget);
    }

    return ret;
}

static gint
delete_event_cb(GtkWidget* window, GdkEventAny* e, gpointer data)
{
    gtk_main_quit();
    return FALSE;
}

static void
on_open_activate(GtkMenuItem *menuitem, gpointer user_data) {
    //TODO: show open dialog, get filename
    
    char const *const filename = "banana.svgd";

    FILE* f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return;
    }
    
    gtk_widget_queue_draw(canvas); // globals are probably evil
}

static void
on_about_activate(GtkMenuItem *menuitem, gpointer user_data) {
    
}

double uniform() {
    return double(rand()) / RAND_MAX;
}

int main(int argc, char **argv) {
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    handles.push_back(Geom::Point(uniform()*400, uniform()*400));
    
    gtk_init (&argc, &argv);
    
    gdk_rgb_init();
    GtkWidget *menubox;
    GtkWidget *menubar;
    GtkWidget *menuitem;
    GtkWidget *menu;
    GtkWidget *open;
    GtkWidget *separatormenuitem;
    GtkWidget *quit;
    GtkWidget *menuitem2;
    GtkWidget *menu2;
    GtkWidget *about;
    GtkAccelGroup *accel_group;

    accel_group = gtk_accel_group_new ();
 
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(window), "text toy");

    menubox = gtk_vbox_new (FALSE, 0);
    gtk_widget_show (menubox);
    gtk_container_add (GTK_CONTAINER (window), menubox);

    menubar = gtk_menu_bar_new ();
    gtk_widget_show (menubar);
    gtk_box_pack_start (GTK_BOX (menubox), menubar, FALSE, FALSE, 0);

    menuitem = gtk_menu_item_new_with_mnemonic ("_File");
    gtk_widget_show (menuitem);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem);

    menu = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem), menu);

    open = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
    gtk_widget_show (open);
    gtk_container_add (GTK_CONTAINER (menu), open);

    separatormenuitem = gtk_separator_menu_item_new ();
    gtk_widget_show (separatormenuitem);
    gtk_container_add (GTK_CONTAINER (menu), separatormenuitem);
    gtk_widget_set_sensitive (separatormenuitem, FALSE);

    quit = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
    gtk_widget_show (quit);
    gtk_container_add (GTK_CONTAINER (menu), quit);

    menuitem2 = gtk_menu_item_new_with_mnemonic ("_Help");
    gtk_widget_show (menuitem2);
    gtk_container_add (GTK_CONTAINER (menubar), menuitem2);

    menu2 = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem2), menu2);

    about = gtk_menu_item_new_with_mnemonic ("_About");
    gtk_widget_show (about);
    gtk_container_add (GTK_CONTAINER (menu2), about);

    g_signal_connect ((gpointer) open, "activate",
                    G_CALLBACK (on_open_activate),
                    NULL);
    g_signal_connect ((gpointer) quit, "activate",
                    gtk_main_quit,
                    NULL);
    g_signal_connect ((gpointer) about, "activate",
                    G_CALLBACK (on_about_activate),
                    NULL);

    gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);


    gtk_window_set_policy(GTK_WINDOW(window), TRUE, TRUE, TRUE);

    gtk_signal_connect(GTK_OBJECT(window),
                       "delete_event",
                       GTK_SIGNAL_FUNC(delete_event_cb),
                       NULL);

    gtk_widget_push_visual(gdk_rgb_get_visual());
    gtk_widget_push_colormap(gdk_rgb_get_cmap());
    canvas = gtk_drawing_area_new();

    gtk_signal_connect(GTK_OBJECT (canvas),
                       "expose_event",
                       GTK_SIGNAL_FUNC(expose_event),
                       0);
    gtk_widget_add_events(canvas, (GDK_BUTTON_PRESS_MASK |
                                   GDK_BUTTON_RELEASE_MASK |
                                   GDK_KEY_PRESS_MASK    |
                                   GDK_POINTER_MOTION_MASK));
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_press_event",
                       GTK_SIGNAL_FUNC(mouse_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "button_release_event",
                       GTK_SIGNAL_FUNC(mouse_release_event),
                       0);
    gtk_signal_connect(GTK_OBJECT (canvas),
                       "motion_notify_event",
                       GTK_SIGNAL_FUNC(mouse_motion_event),
                       0);
    gtk_signal_connect(GTK_OBJECT(canvas),
                       "key_press_event",
                       GTK_SIGNAL_FUNC(key_release_event),
                       0);

    gtk_widget_pop_colormap();
    gtk_widget_pop_visual();

    //GtkWidget *vb = gtk_vbox_new(0, 0);


    //gtk_container_add(GTK_CONTAINER(window), vb);

    gtk_box_pack_start(GTK_BOX(menubox), canvas, TRUE, TRUE, 0);

    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);

    gtk_widget_show_all(window);

    /* Make sure the canvas can receive key press events. */
    GTK_WIDGET_SET_FLAGS(canvas, GTK_CAN_FOCUS);
    assert(GTK_WIDGET_CAN_FOCUS(canvas));
    gtk_widget_grab_focus(canvas);
    assert(gtk_widget_is_focus(canvas));

    //g_idle_add((GSourceFunc)idler, canvas);

    gtk_main();

    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
