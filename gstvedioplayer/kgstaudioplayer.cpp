
#include "kgstaudioplayer.h"

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

void IKGSTAudioPlayer::event_loop(GstElement * pipe)
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

//gboolean IKGSTAudioPlayer::bus_watch(GstBus *bus , GstMessage *msg , gpointer data)
//{
//    GMainLoop *loop =(GMainLoop *) data;
//    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
//    {
//        g_main_loop_quit(loop);
//    }
//    return TRUE;
//}

void IKGSTAudioPlayer::implRun()// default implRun()
{
    Stop();
    /* start playing */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
//    g_main_loop_run(mloop);
    event_loop(pipeline);
}// ...
//{ event_loop(pipeline); return S_OK; }
IKGSTAudioPlayer::IKGSTAudioPlayer()
    : filesrc(0), volume(0), audiosink(0), pipeline(0), m_bLoopPlay(false)
{
    gst_init(0, 0);//...

    filesrc = gst_element_factory_make("filesrc" , "filesrc");
    g_assert(filesrc);
    pipeline = gst_pipeline_new("audio-player");
    g_assert(pipeline);
    volume = gst_element_factory_make("volume", "volume");
    g_assert(volume);
    g_object_set(G_OBJECT(volume), "volume", 1.0, NULL);
    audiosink = gst_element_factory_make("alsasink" , "audiosink");
    g_assert(audiosink);
    // add into a bin ...

//    mloop = g_main_loop_new(NULL , TRUE);
//    GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
//    gst_bus_add_watch(bus , bus_watch , mloop);
//    g_object_unref(bus);
}

IKGSTAudioPlayer::~IKGSTAudioPlayer()
{
    if (pipeline)
    {
        gst_element_set_state(pipeline , GST_STATE_NULL);
        g_object_unref(pipeline);
//        g_main_loop_unref(mloop);
    }
//        else {} // ...
}

IKGSTAudioPlayer* IKGSTAudioPlayer::LoadFile(const char* mp3file)// const std::string&
{
    gst_element_set_state(pipeline , GST_STATE_NULL);
    g_object_set(G_OBJECT(filesrc) , "location"
                 , mp3file , NULL);
    return this;
}

STDMETHODIMP IKGSTAudioPlayer::Run()
{
    if(!pipeline)
        return E_FAIL;
    Stop();
    /* start playing */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    GstState    state;
    switch(gst_element_get_state(pipeline, &state, NULL, GST_CLOCK_TIME_NONE))
    {
    case GST_STATE_CHANGE_SUCCESS:
        std::cout << "GST_STATE_CHANGE_SUCCESS\n";
        break;
    case GST_STATE_CHANGE_ASYNC:
        std::cout << "GST_STATE_CHANGE_ASYNC\n";
        return E_FAIL;
    case GST_STATE_CHANGE_FAILURE:
        std::cout << "GST_STATE_CHANGE_FAILURE\n";
        return E_FAIL;
    default:
        std::cout << "default ...\n";
    }

    return S_OK;
}
STDMETHODIMP IKGSTAudioPlayer::Pause()
{
    if(!pipeline)
        return E_FAIL;

    gst_element_set_state(pipeline , GST_STATE_PAUSED);
    return S_OK;
}
STDMETHODIMP IKGSTAudioPlayer::Resume()
{
    if(!pipeline)
        return E_FAIL;

    gst_element_set_state(pipeline, GST_STATE_PLAYING);// ...
    return S_OK;
}
STDMETHODIMP IKGSTAudioPlayer::Stop()
{
    // Pause() ...
    if(!pipeline)
        return E_FAIL;

    gst_element_set_state(pipeline , GST_STATE_READY);
    //gst_element_set_state(pipeline , GST_STATE_NULL);
    return S_OK;
}

void IKGSTAudioPlayer::setLoopPlay(bool loop)
{
    m_bLoopPlay = loop;
}
bool IKGSTAudioPlayer::getLoopPlay() const
{
    return m_bLoopPlay;
}

int IKGSTAudioPlayer::GetVolume() const
{
    gdouble vol = 0;

    GstElement *_volume = gst_bin_get_by_name(GST_BIN(pipeline), "volume");
    g_object_get(G_OBJECT(_volume), "volume", &vol, NULL);
    gst_object_unref(_volume);
//    std::cout << vol << " : vol\n";
    return vol * 100;

}

STDMETHODIMP IKGSTAudioPlayer::SetVolume(ULONG v)//0-1000
{
    int res = GetVolume() ;// ....
    v = std::max((int)1, std::min((int)1000, (int)v));
    gdouble vol = 0.01 * v;

    GstElement *_volume = gst_bin_get_by_name(GST_BIN(pipeline), "volume");
    g_object_set(G_OBJECT(_volume), "volume", vol, NULL);
    gst_object_unref(_volume);

    return res ;
}
STDMETHODIMP_(STREAM_TIME) IKGSTAudioPlayer::GetDuration()
{
    if(!pipeline)
        return 0;

    GstFormat fmt = GST_FORMAT_TIME;
    gint64  len;
    if (gst_element_query_duration(pipeline, &fmt, &len))
        return len;
    else
        return 0;
}
STDMETHODIMP_(STREAM_TIME) IKGSTAudioPlayer::GetPosition()//0-100
{
    if(!pipeline)
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
        return  pos;//pos/100
    }
    else
        return 0;

}

STDMETHODIMP IKGSTAudioPlayer::SetPosition(STREAM_TIME pos)//0-100
{
    if(!pipeline)
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

STDMETHODIMP_(KFILTER_STATE) IKGSTAudioPlayer::GetState()
{
    GstState    state;
    gst_element_get_state(pipeline, &state, NULL, GST_CLOCK_TIME_NONE );
    switch(state)
    {
    case GST_STATE_NULL:
        return KState_Stopped;
        break;
    case GST_STATE_PLAYING:
        return KState_Running;
        break;
    case GST_STATE_READY:
    case GST_STATE_PAUSED:
        return  KState_Paused;
        break;
    default:
        return KState_Unknown;
    }
}
///

void KGSTMp3Player::implRun()
{
    IKGSTAudioPlayer::implRun();
}

KGSTMp3Player::KGSTMp3Player() {}
KGSTMp3Player*  KGSTMp3Player::getSelf()
{
    return this;
}

KGSTMp3Player* KGSTMp3Player::Init(const char* wavfile, bool loop)
{
    setLoopPlay(loop);

    g_object_set(G_OBJECT(filesrc), "location", wavfile, NULL);
    /* now it's time to get the decoder */
    decoder = gst_element_factory_make("mad", "decode");
    if(!decoder)
    {
        g_print("could not find plugin \"mad\"");
    }
    /* also, we need to add some converters to make sure the audio stream
     * from the decoder is converted into a format the audio sink can
     * understand(if necessary) */
    conv = gst_element_factory_make("audioconvert", "audioconvert");
    if(!conv)
    {
        g_print("could not create \"audioconvert\" element!");
    }
    resample = gst_element_factory_make("audioresample", "audioresample");
    if(!resample)
    {
        g_print("could not create \"audioresample\" element!");
    }
    /* add objects to the main pipeline */

    gst_bin_add_many(GST_BIN(pipeline), filesrc, decoder, conv,
                     resample, volume, audiosink, NULL);
    /* link the elements */
    gst_element_link_many(filesrc, decoder, conv, resample, volume, audiosink, NULL);
    return this;
}

KGSTMp3Player::KGSTMp3Player(const char* mp3file, bool loop)////const std::string&
{
    Init(mp3file, loop);
}
KGSTMp3Player::~KGSTMp3Player() {}

///

void KGSTWavPlayer::add_pad(GstElement *element , GstPad *pad , gpointer data)
{
    gchar *name;
    GstElement *sink =(GstElement*)data;

    name = gst_pad_get_name(pad);
    gst_element_link_pads(element , name , sink , "sink");
    //gst_element_link_pads(element , name , sink , "volume");
    g_free(name);
}

void KGSTWavPlayer::implRun()
{
    IKGSTAudioPlayer::implRun();
}

KGSTWavPlayer::KGSTWavPlayer() {}
KGSTWavPlayer*  KGSTWavPlayer::getSelf()
{
    return this;
}

KGSTWavPlayer*   KGSTWavPlayer::Init(const char* wavfile, bool loop)
{
    setLoopPlay(loop);

    parser = gst_element_factory_make("wavparse" , "parser");
    g_object_set(G_OBJECT(filesrc) , "location"
                 , wavfile , NULL);

    gst_bin_add_many(GST_BIN(pipeline)
                     , filesrc , parser , volume, audiosink , NULL);
//        gst_element_link_many(filesrc, parser, volume, audiosink, NULL);//linke filesrc to parser failed !!!

    g_signal_connect(parser, "pad-added" , G_CALLBACK(add_pad) , volume);// solve ...
    if(! gst_element_link(filesrc , parser))
    {
        g_warning("linke filesrc to parser failed");
    }
    gst_element_link(volume , audiosink);
    return this;
}

KGSTWavPlayer::KGSTWavPlayer(const char* wavfile, bool loop)//
{
    Init(wavfile, loop);
}
KGSTWavPlayer::~KGSTWavPlayer() {}

///
///

void KGSTAudioPlayer::implRun()
{
    do
    {
        m_pPlayer->implRun();
    }
    while(m_bLoopPlay); //getLoopPlay()

}

KGSTAudioPlayer::KGSTAudioPlayer()
    : m_pMp3player(0), m_pWavplayer(0), m_pPlayer(0)//
{}
KGSTAudioPlayer::KGSTAudioPlayer(const char* audiofile, bool loop)
    : m_pMp3player(0), m_pWavplayer(0), m_pPlayer(0)//
{
    LoadFile(audiofile);
    setLoopPlay(loop);
}
KGSTAudioPlayer::~KGSTAudioPlayer()
{
    if (m_pMp3player)    delete m_pMp3player;
    if (m_pWavplayer)    delete m_pWavplayer;
}
STDMETHODIMP KGSTAudioPlayer::LoadFile(const char* audiofile)// const std::string& //LPCWSTR
{
    // *.wav ot *mp3
    //std::string file(audiofile);
    ks_string file(audiofile);//
    if (file.find(".mp3")!=file.npos)
        if(!m_pMp3player)//==NULL
            m_pPlayer = m_pMp3player = new KGSTMp3Player(file.c_str());
        else
            m_pPlayer = m_pMp3player->LoadFile(file.c_str());
    else if (file.find(".wav")!=file.npos)
        if(!m_pWavplayer)//==NULL
            m_pPlayer = m_pWavplayer = new KGSTWavPlayer(file.c_str());
        else
            m_pPlayer = m_pWavplayer->LoadFile(file.c_str());
    else
        return E_FAIL;//TRACE_ERR("KGSTAudioPlayer::LoadFile error", E_FAIL);

    return S_OK;
}
STDMETHODIMP KGSTAudioPlayer::Run()
{
    return m_pPlayer->Run();
//        m_pPlayer->implRun();
//        return S_OK;
}
STDMETHODIMP KGSTAudioPlayer::Pause()
{
    return m_pPlayer->Pause();
}
STDMETHODIMP KGSTAudioPlayer::Resume()
{
    return m_pPlayer->Resume();
}
STDMETHODIMP KGSTAudioPlayer::Stop()
{
    return m_pPlayer->Stop();
}
int KGSTAudioPlayer::GetVolume()
{
    return m_pPlayer->GetVolume();
}
STDMETHODIMP KGSTAudioPlayer::SetVolume(ULONG vol)
{
    return m_pPlayer->SetVolume(vol);
}
STDMETHODIMP_(STREAM_TIME) KGSTAudioPlayer::GetDuration()
{
    return m_pPlayer->GetDuration();
}
STDMETHODIMP_(STREAM_TIME) KGSTAudioPlayer::GetPosition()
{
    return m_pPlayer->GetPosition();
}
STDMETHODIMP KGSTAudioPlayer::SetPosition(STREAM_TIME pos)
{
    return m_pPlayer->SetPosition(pos);
}
STDMETHODIMP_(KFILTER_STATE) KGSTAudioPlayer::GetState()
{
    return m_pPlayer->GetState();
}
//
void KGSTAudioPlayer::setLoopPlay(bool loop)
{
    KGSTAudioPlayer::m_bLoopPlay = loop;
}
bool KGSTAudioPlayer::getLoopPlay() const
{
    return KGSTAudioPlayer::m_bLoopPlay;
}

///
void KGSTAudioPlayer_::cb_newpad(GstElement *decodebin,
                                 GstPad     *pad,
                                 gboolean    last,
                                 gpointer    data)
{
    GstCaps *caps;
    GstStructure *str;
    GstPad *audiopad;

    /* only link once */
    GstElement  *audio = static_cast<GstElement*>(data);
    audiopad = gst_element_get_static_pad(audio, "sink");
    if (GST_PAD_IS_LINKED(audiopad))
    {
        g_object_unref(audiopad);
        return;
    }

    /* check media type */
    caps = gst_pad_get_caps(pad);
    str = gst_caps_get_structure(caps, 0);
    if(!g_strrstr(gst_structure_get_name(str), "audio"))
    {
        gst_caps_unref(caps);
        gst_object_unref(audiopad);
        return;
    }
    gst_caps_unref(caps);

    /* link'n'play */
    gst_pad_link(pad, audiopad);

    g_object_unref(audiopad);
}
gboolean KGSTAudioPlayer_::cb_print_position(GstElement *pipeline)
{
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 pos, len;

    if (gst_element_query_position(pipeline, &fmt, &pos)
            && gst_element_query_duration(pipeline, &fmt, &len))
    {
        g_print("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
                GST_TIME_ARGS(pos), GST_TIME_ARGS(len));
    }
    /* call me again */
    return TRUE;
}

KGSTAudioPlayer_*  KGSTAudioPlayer_::getSelf()
{
    return this;
}

KGSTAudioPlayer_* KGSTAudioPlayer_::Init(const char* audiofile, bool loop)
{
    setLoopPlay(loop);

    g_object_set(G_OBJECT(filesrc), "location", audiofile, NULL);
    /* get the decoder */
    decodebin = gst_element_factory_make("decodebin", "decodebin");
    if(!decodebin)
    {
        g_print("could not find plugin \"mad\"");
    }
    g_assert(decodebin);
    /* create audio output */
    audio = gst_bin_new("audiobin");
    g_assert(audio);

    g_signal_connect(decodebin, "new-decoded-pad", G_CALLBACK(cb_newpad), audio);
    gst_bin_add_many(GST_BIN(pipeline), filesrc, decodebin, NULL);
    gst_element_link(filesrc, decodebin);

    audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
    if(!audioconvert)
    {
        g_print("could not create \"audioconvert\" element!");
    }
    g_assert(audioconvert);

    audioresample = gst_element_factory_make("audioresample", "audioresample");
    if(!audioresample)
    {
        g_print("could not create \"audioresample\" element!");
    }
    g_assert(audioresample);

    /* add ... into audio output */
    audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
    GstPad  *audiopad = gst_element_get_static_pad(audioconvert, "sink");

    gst_bin_add_many(GST_BIN(audio), audioconvert, audioresample, volume, audiosink, NULL);
    gst_element_link_many(audioconvert, audioresample, volume, audiosink, NULL);
    gst_element_add_pad(audio,
                        gst_ghost_pad_new("sink", audiopad));
    gst_object_unref(audiopad);
    gst_bin_add(GST_BIN(pipeline), audio);

///     error ...
//        /* add objects to the main pipeline *///
//        gst_bin_add_many(GST_BIN(pipeline), filesrc, decodebin, audioconvert,
//                          audioresample, volume, audiosink, NULL);
//        /* link the elements */
//        gst_element_link_many(filesrc, decodebin, audioconvert, audioresample, volume, audiosink, NULL);

//        g_timeout_add(200,(GSourceFunc) cb_print_position, pipeline);

    return this;
}
void KGSTAudioPlayer_::implRun()
{
    IKGSTAudioPlayer::implRun();
}
