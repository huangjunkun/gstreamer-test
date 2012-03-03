#ifndef KGSTVEDIOPLAYER_H_INCLUDED
#define KGSTVEDIOPLAYER_H_INCLUDED

#include "../add_core/add_core01.h"
#include "../add_core/add_core02.h"
#include <gst/gst.h>
//#include <stdio.h>

#ifndef SYNC_GST_ELEMENT_SET_STATE
#define SYNC_GST_ELEMENT_SET_STATE
#endif

#ifdef SYNC_GST_ELEMENT_SET_STATE
inline BOOL sync_gst_element_set_state(GstElement *element, GstState state)
{
    GstStateChangeReturn res;
    res = gst_element_set_state(GST_ELEMENT (element), state);
    if (GST_STATE_CHANGE_FAILURE == res)
        return FALSE;
    if (GST_STATE_CHANGE_ASYNC == res)
    {
        GstState state;
        res = gst_element_get_state(GST_ELEMENT (element), &state, NULL, 1000000000);
        if (GST_STATE_CHANGE_FAILURE == res || GST_STATE_CHANGE_ASYNC == res)
            return FALSE;
    }
    return TRUE;
}
inline void sync_gst_element_set_state2(GstElement *element, GstState state)
{
    GstStateChangeReturn res;
    res = gst_element_set_state(GST_ELEMENT (element), state);
    if (GST_STATE_CHANGE_FAILURE == res)
        return;
    if (GST_STATE_CHANGE_ASYNC == res)
    {
        GstState state;
        res = gst_element_get_state(GST_ELEMENT (element), &state, NULL, 1000000000);
        if (GST_STATE_CHANGE_FAILURE == res || GST_STATE_CHANGE_ASYNC == res)
            return;
    }
}

#else
#define sync_gst_element_set_state gst_element_set_state
#define sync_gst_element_set_state2 gst_element_set_state

#endif

/// note: 1000000000 == GST_CLOCK_TIME_NONE
/// gst_element_set_state => sync_gst_element_set_state or sync_gst_element_set_state2

gboolean my_bus_callback(GstBus *bus , GstMessage *message , gpointer data)
{
    GMainLoop *loop =(GMainLoop *) data;
    switch(message->type)
    {
    case GST_MESSAGE_EOS:
        g_main_loop_quit(loop);
        return TRUE;

    case GST_MESSAGE_WARNING:
    case GST_MESSAGE_ERROR:
    {
        GError *gerror;
        gchar *debug;
        gst_message_parse_error(message, &gerror, &debug);
        gst_object_default_error(GST_MESSAGE_SRC(message), gerror, debug);
        g_print(" gerro: %s,\n debug: %s\n", gerror->message, debug);
        g_error_free(gerror);
        g_free(debug);
        g_main_loop_quit(loop);
        return FALSE;
    }
    default:
        g_print(" video is playing ...\n");//...
    }
    return TRUE;
}

int PlayVedioWithPlaybin(const char* uri)//fullfilepath
{
    gst_init(0, 0);
    GMainLoop *loop;
    GstElement *play;
    GstBus *bus;

    /* init GStreamer */
    loop = g_main_loop_new(NULL, FALSE);
    /* set up */
    play = gst_element_factory_make("playbin", "play");
    g_assert(play);
    g_object_set(G_OBJECT(play), "uri", uri, NULL);

    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    gst_bus_add_watch(bus, my_bus_callback, loop);
    gst_object_unref(bus);

    gst_element_set_state(play, GST_STATE_PLAYING);

    /* now run */
    g_main_loop_run(loop);

    /* also clean up */
    gst_element_set_state(play, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(play));

    return 0;
}

/// KGSTVedioPlayer ....

class KGSTVedioPlayer :  public KWavePlayer
{
private:
    /*
    @fn event_loop
    @brief

    @arg [in] pipe
    @return
    @remark
    @*/
    static void event_loop(GstElement * pipe);

public:
    KGSTVedioPlayer();
    KGSTVedioPlayer(const char* vediofile, bool loop = false);//fullfilepath
    virtual ~KGSTVedioPlayer();
    /// ***
    KGSTVedioPlayer* LoadFile(const char* vediofile);
    virtual void implRun(); //= 0;// PRUE

    STDPROC Run();
    STDPROC Pause();
    STDPROC Resume();
    STDPROC Stop();
    ULONG GetVolume() const;
    STDPROC SetVolume(ULONG v);//0-1000
    STDPROC_(STREAM_TIME) GetDuration();
    STDPROC_(STREAM_TIME) GetPosition();
    STDPROC SetPosition(STREAM_TIME pos);

    STDPROC_(KFILTER_STATE) GetState();

    void setLoopPlay(bool loop);
    bool getLoopPlay() const;
protected:
    GstElement  *filesrc, *volume, *audiosink, *pipeline;//, *playbin
    bool    m_bLoopPlay;
};

//class KGSTVedioPlayer_ : public KGSTVedioPlayer
//{
//public:
//
//private:
//
//};

void KGSTVedioPlayer::event_loop(GstElement * pipe)
{
    GstBus *bus;
    GstMessage *message = NULL;
    bus = gst_element_get_bus(GST_ELEMENT(pipe));
    bool    loop = true;
    while(loop)
    {
        message = gst_bus_poll(bus, GST_MESSAGE_ANY, -1);
        g_assert(message != NULL);
        switch(message->type)
        {
        case GST_MESSAGE_EOS:
            gst_message_unref(message);
            loop = false;//return
            break;
        case GST_MESSAGE_WARNING:
        case GST_MESSAGE_ERROR:
        {
            GError *gerror;
            gchar *debug;

            gst_message_parse_error(message, &gerror, &debug);
            gst_object_default_error(GST_MESSAGE_SRC(message), gerror, debug);
            gst_message_unref(message);
            g_error_free(gerror);
            g_free(debug);
            loop = false;//return
        }
        break;
        default:
            gst_message_unref(message);
            break;
        }
    }
    gst_object_unref(bus);
}

void KGSTVedioPlayer::implRun()// default implRun()
{
    Stop();
    /* start playing */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    event_loop(pipeline);
}// ...

KGSTVedioPlayer::KGSTVedioPlayer()
    : filesrc(0), volume(0), audiosink(0), pipeline(0), m_bLoopPlay(false)
{
    gst_init(0, 0);
    pipeline = gst_element_factory_make("playbin", "pipeline");
    g_assert(pipeline);
}
KGSTVedioPlayer::KGSTVedioPlayer(const char* vediofile, bool loop)//fullfilepath
    : filesrc(0), volume(0), audiosink(0), pipeline(0), m_bLoopPlay(loop)
{
    gst_init(0, 0);//...
    pipeline = gst_element_factory_make("playbin", "pipeline");
    g_assert(pipeline);
    LoadFile(vediofile);
    // ...
    g_object_get(G_OBJECT(pipeline),
                 "source", &filesrc,
                 "volume", &volume,
                 "audio-sink", &audiosink,
                 NULL);
//    g_assert(filesrc);
//    g_assert(volume);
//    g_assert(audiosink);
}

KGSTVedioPlayer::~KGSTVedioPlayer()
{
    if (pipeline)
    {
        gst_element_set_state(pipeline , GST_STATE_NULL);
        g_object_unref(pipeline);
    }
}

KGSTVedioPlayer* KGSTVedioPlayer::LoadFile(const char* vediofile)// const std::string&
{
    gst_element_set_state(pipeline , GST_STATE_NULL);
    std::string uri = "file://";
    uri = uri + vediofile;
    g_object_set(G_OBJECT(pipeline), "uri", uri.c_str(), NULL);

    return this;
}

STDMETHODIMP KGSTVedioPlayer::Run()
{
    Stop();
    /* start playing */
    sync_gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (GST_STATE_CHANGE_FAILURE == \
            gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE))
        return E_FAIL;
    else
        return S_OK;
}

STDMETHODIMP KGSTVedioPlayer::Pause()
{
    if (KState_Paused == GetState())
        return S_OK;
    sync_gst_element_set_state(pipeline , GST_STATE_PAUSED);

    if (GST_STATE_CHANGE_FAILURE == \
            gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE))
        return E_FAIL;
    else
        return S_OK;
}

STDMETHODIMP KGSTVedioPlayer::Resume()
{
    if (KState_Running == GetState())
        return S_OK;
    sync_gst_element_set_state(pipeline, GST_STATE_PLAYING);

    if (GST_STATE_CHANGE_FAILURE == \
            gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE))
        return E_FAIL;
    else
        return S_OK;
}

STDMETHODIMP KGSTVedioPlayer::Stop()
{
    if (KState_Stopped == GetState())
        return S_OK;
    sync_gst_element_set_state2(pipeline , GST_STATE_NULL);

    if (GST_STATE_CHANGE_FAILURE == \
            gst_element_get_state(pipeline, NULL, NULL, GST_CLOCK_TIME_NONE))
        return E_FAIL;
    else
        return S_OK;
}

STDMETHODIMP_(STREAM_TIME) KGSTVedioPlayer::GetDuration()
{
    if (!pipeline)
        return 0;

    GstFormat fmt = GST_FORMAT_TIME;
    gint64  len = 0;

    Run();//预先准备数据，以查询管道播放音频的大小
    if (gst_element_query_duration(pipeline, &fmt, &len));//ok
    Stop();

    return len;
}

STDMETHODIMP_(STREAM_TIME) KGSTVedioPlayer::GetPosition()//0-100
{
    if (!pipeline)
        return 0;
    GstElement *p = pipeline;
    gint64 Glength = GetDuration(), Gstart = 0;
    gint64  pos;
    GstFormat fmt = GST_FORMAT_TIME;

    if (Glength && gst_element_query_position(p, &fmt, &pos))
    {
        pos -= Gstart;
        pos *= 100;
        pos /= Glength;
        return pos;
    }
    else
        return 0;
}

STDMETHODIMP KGSTVedioPlayer::SetPosition(STREAM_TIME pos)//0-100
{
    if (!pipeline)
        return E_FAIL;
    gint64 time;
    GstElement *p = pipeline;
    gint64 Glength = GetDuration(), Gstart = 0;
    if (Glength)
    {
        time = Glength;
        time *= (double)pos/100;
        time += Gstart;
        bool b = gst_element_seek(p, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH, GST_SEEK_TYPE_SET, time, GST_SEEK_TYPE_SET, Gstart + Glength);

        return b ? S_OK : E_FAIL;
    }
    else
        return  E_FAIL;
}

ULONG KGSTVedioPlayer::GetVolume() const//0-1000
{
    gdouble vol;
    GstElement *_volume = gst_bin_get_by_name(GST_BIN(pipeline), "volume");
    g_object_get(G_OBJECT(_volume), "volume", &vol, NULL);
    gst_object_unref(_volume);

    return ULONG(vol * 100);

}

STDMETHODIMP KGSTVedioPlayer::SetVolume(ULONG v)//0-1000
{
    int res = GetVolume();// ....
    v = std::max((ULONG)1, std::min((ULONG)1000, v));
    gdouble vol = 0.01 * v;

    GstElement *_volume = gst_bin_get_by_name(GST_BIN(pipeline), "volume");
    g_object_set(G_OBJECT(_volume), "volume", vol, NULL);
    gst_object_unref(_volume);

    return res;
}

STDMETHODIMP_(KFILTER_STATE) KGSTVedioPlayer::GetState()
{
    if (!pipeline)
        return KState_NotReady;

    GstState    state;
    if (GST_STATE_CHANGE_FAILURE != \
            gst_element_get_state(pipeline, &state, NULL, GST_CLOCK_TIME_NONE))
        switch(state)
        {
        case GST_STATE_NULL:
            return KState_Stopped;
            break;
        case GST_STATE_PLAYING:
            return KState_Running;
            break;
        case GST_STATE_READY:
            return KState_Open;
            break;
        case GST_STATE_PAUSED:
            return KState_Paused;
            break;
        case GST_STATE_VOID_PENDING:
            return KState_NotReady;
            break;
        default:
            return KState_Unknown;
        }
    else
        return KState_Unknown;
}

void KGSTVedioPlayer::setLoopPlay(bool loop)
{
    m_bLoopPlay = loop;
}
bool KGSTVedioPlayer::getLoopPlay() const
{
    return m_bLoopPlay;
}


#endif // KGSTVEDIOPLAYER_H_INCLUDED
