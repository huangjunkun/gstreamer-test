#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>  // for GDK_WINDOW_XID
#endif
#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>

//#include "kgstvedioplayer.h"

// ...
static gulong video_window_xid = 0;
// ...
static GstBusSyncReply bus_sync_handler(GstBus * bus, GstMessage * message, gpointer user_data)
{
// ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ELEMENT)
        return GST_BUS_PASS;
    if(!gst_structure_has_name(message->structure, "prepare-xwindow-id"))
        return GST_BUS_PASS;

    if (video_window_xid != 0)
    {
        GstXOverlay *xoverlay;

        // GST_MESSAGE_SRC(message) will be the video sink element
        xoverlay = GST_X_OVERLAY(GST_MESSAGE_SRC(message));
        gst_x_overlay_set_xwindow_id(xoverlay, video_window_xid);
    }
    else
    {
        g_warning("Should have obtained video_window_xid by now!");
    }

    gst_message_unref(message);
    return GST_BUS_DROP;
}
// ...
static void
video_widget_realize_cb(GtkWidget * widget, gpointer data)
{
#if GTK_CHECK_VERSION(2,18,0)
    // This is here just for pedagogical purposes, GDK_WINDOW_XID will call
    // it as well in newer Gtk versions
    if(!gdk_window_ensure_native(widget->window))
        g_error("Couldn't create native window needed for GstXOverlay!");
#endif

#ifdef GDK_WINDOWING_X11
    GtkWidget *video_window = widget;
    video_window_xid = GDK_WINDOW_XID(video_window->window);
#endif
}
// ...
int test_main(const char* file)//fullfilepath
{
    GtkWidget *video_window;
    GtkWidget *app_window;
    // ...
    app_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    // ...
    video_window = gtk_drawing_area_new();
    g_signal_connect(video_window, "realize",
                     G_CALLBACK(video_widget_realize_cb), NULL);
    gtk_widget_set_double_buffered(video_window, FALSE);
    // ...
    // usually the video_window will not be directly embedded into the
    // application window like this, but there will be many other widgets
    // and the video window will be embedded in one of them instead
    gtk_container_add(GTK_CONTAINER(app_window), video_window);
    // ...
    // show the GUI
    gtk_widget_show_all(app_window);

    // realize window now so that the video window gets created and we can
    // obtain its XID before the pipeline is started up and the videosink
    // asks for the XID of the window to render onto
    gtk_widget_realize(app_window);//window

    // we should have the XID now
    g_assert(video_window_xid != 0);
    // ...
    /* init GStreamer */
    gst_init(0, 0);
    GMainLoop *loop;
    GstElement *play;
    GstBus *bus;
    loop = g_main_loop_new(NULL, FALSE);
    /* set up */
    play = gst_element_factory_make("playbin", "playbin");
    g_assert(play);
    std::string uri = "file://";
    uri = uri + file;
    g_object_set(G_OBJECT(play), "uri", uri, NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    gst_bus_add_watch(bus, my_bus_callback, loop);
    gst_object_unref(bus);

    GstElement  *pipeline = gst_bin_get_by_name(GST_BIN(pipeline), "playbin");
    // set up sync handler for setting the xid once the pipeline is started
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_set_sync_handler(bus,(GstBusSyncHandler) bus_sync_handler, NULL);
    gst_object_unref(bus);
    // ...
//    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    gst_element_set_state(play, GST_STATE_PLAYING);
    /* now run */
    g_main_loop_run(loop);

    /* also clean up */
    gst_element_set_state(play, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(play));
    // ...
}

#endif // MAIN_H_INCLUDED
