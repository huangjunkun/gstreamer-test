#ifndef KGSTAUDIOPLAYER_H_INCLUDED
#define KGSTAUDIOPLAYER_H_INCLUDED

//#include <windows.h>
//#include <kfc/string.h>
//#include <kfc.h>
#include <string>
#include <gst/gst.h>


#include "../add_core/add_core01.h"
#include "../add_core/add_core02.h"

/// 根据原来wpp提供的接口KWavePlayer，继承实现的播放音频操作类，包括IKGSTAudioPlayer（接口），
/// 类 KGSTMp3Player ，类 KGSTWavPlayer ，类KGSTAudioPlayer。详细定义见下：

/*
@fn
@brief

@arg []
@return
@remark
@*/

/// IKGSTAudioPlayer ....

class IKGSTAudioPlayer :  public KWavePlayer
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
//    static gboolean bus_watch(GstBus *bus , GstMessage *msg , gpointer data);

public:
    IKGSTAudioPlayer();
    virtual ~IKGSTAudioPlayer();
    /// ***
    IKGSTAudioPlayer* LoadFile(const char* mp3file);
    virtual void implRun() = 0;// PRUE

    STDPROC Run();
    STDPROC Pause();
    STDPROC Resume();
    STDPROC Stop();
    int GetVolume() const;
    STDPROC SetVolume(ULONG v);//0-1000
    STDPROC_(STREAM_TIME) GetDuration();
    STDPROC_(STREAM_TIME) GetPosition();
    STDPROC SetPosition(STREAM_TIME pos);

    STDPROC_(KFILTER_STATE) GetState();

    void setLoopPlay(bool loop);
    bool getLoopPlay() const;
protected:
    GstElement  *filesrc, *volume, *audiosink, *pipeline;
    bool    m_bLoopPlay;
//    GMainLoop   *mloop;
};

/// ---

/// KGSTMp3Player ....
class KGSTMp3Player : public virtual IKGSTAudioPlayer
{
public:
    void implRun();

protected:
    KGSTMp3Player();
    KGSTMp3Player*  getSelf();
    KGSTMp3Player* Init(const char* wavfile, bool loop=false);

public:
    KGSTMp3Player(const char* mp3file, bool loop=false);
    ~KGSTMp3Player();

private:
    GstElement  *decoder, *conv, *resample;
};


/// KGSTWavPlayer ....
class KGSTWavPlayer : public virtual IKGSTAudioPlayer
{
private:
    static void add_pad(GstElement *element , GstPad *pad , gpointer data);
public:
    void implRun();

protected:
    KGSTWavPlayer();
    KGSTWavPlayer*  getSelf();
    KGSTWavPlayer*   Init(const char* wavfile, bool loop=false);

public:
    KGSTWavPlayer(const char* wavfile, bool loop=false);
    ~KGSTWavPlayer();

private:
    GstElement  *parser;
    GstBus      *bus;
};///

static void* PlayerRunOnThreadProc( void* arg)
{
    IKGSTAudioPlayer* pPlayer= reinterpret_cast<IKGSTAudioPlayer*>(arg);
    for(; ;)
        pPlayer->implRun();
    return  (void*) 0;
}

/// KGSTAudioPlayer ...
class KGSTAudioPlayer :  public IKGSTAudioPlayer
{
public:
    friend void* PlayerRunOnThreadProc( void* arg);
//private:
    void implRun();
public:
    KGSTAudioPlayer();
    KGSTAudioPlayer(const char* audiofile, bool loop=false);
    ~KGSTAudioPlayer();

    STDPROC LoadFile(const char* audiofile);
    STDPROC Run();
    STDPROC Pause();
    STDPROC Resume();
    STDPROC Stop();

    int GetVolume();
    STDPROC SetVolume(ULONG vol);
    STDPROC_(STREAM_TIME) GetDuration();
    STDPROC_(STREAM_TIME) GetPosition();
    STDPROC SetPosition(STREAM_TIME pos);
    STDPROC_(KFILTER_STATE) GetState();
//
    void setLoopPlay(bool loop);
    bool getLoopPlay() const;
private:
    KGSTMp3Player *m_pMp3player;
    KGSTWavPlayer *m_pWavplayer;
    IKGSTAudioPlayer    *m_pPlayer;
    bool           m_bLoopPlay;
};

/**                     ****/
/// KGSTAudioPlayer_ ...
class KGSTAudioPlayer_Error :  public virtual KGSTWavPlayer, public virtual KGSTMp3Player
{
//private:
public:
    void implRun()
    {
        do
        {
            this->implRun();
        }
        while(m_bLoopPlay);

    }
public:
    KGSTAudioPlayer_Error() {}
    KGSTAudioPlayer_Error(const char* audiofile, bool loop=false)
    {
        LoadFile(audiofile);
        setLoopPlay(loop);
    }

    ~KGSTAudioPlayer_Error() {}

    STDPROC LoadFile(const char* audiofile)// const std::string& //LPCWSTR
    {
        // *.wav ot *mp3
        //std::string file(audiofile);
        ks_string file(audiofile);//
        if (file.find(".mp3")!=file.npos)
            m_pPlayer = this->KGSTMp3Player::Init(audiofile);
        else if (file.find(".wav")!=file.npos)
            m_pPlayer = this->KGSTWavPlayer::Init(audiofile);
        else
            return E_FAIL;//TRACE_ERR("KGSTAudioPlayer_Error::LoadFile error", E_FAIL);
        return S_OK;
    }

private:
    IKGSTAudioPlayer    *m_pPlayer;
} ;

/// KGSTAudioPlayer_...

class KGSTAudioPlayer_ :  public IKGSTAudioPlayer
{
private:
    static void cb_newpad(GstElement *decodebin,
                          GstPad     *pad,
                          gboolean    last,
                          gpointer    data);
    static gboolean cb_print_position(GstElement *pipeline);

public:
    KGSTAudioPlayer_()
    {
        Init(NULL, false);
    }
    KGSTAudioPlayer_(const char* audiofile, bool loop=false)////const std::string&
    {
        Init(audiofile, loop);
    }
    ~KGSTAudioPlayer_() {}

public:
    KGSTAudioPlayer_*  getSelf();
    KGSTAudioPlayer_* Init(const char* audiofile, bool loop=false);
    void implRun();

private:
    GstElement  *decodebin, *audioconvert, *audioresample ;
    GstElement  *audio;//
};



/// --- end ---
#endif // KGSTAUDIOPLAYER_H_INCLUDED
