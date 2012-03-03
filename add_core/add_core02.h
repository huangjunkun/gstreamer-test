#ifndef ADD_CORE02_H_INCLUDED
#define ADD_CORE02_H_INCLUDED

#include <windows.h>

namespace
{
#ifndef _WIN32

#define S_OK    0
#define S_FALSE 1
#define E_FAIL  80004005
typedef int DWORD;
typedef u_long  ULONG;

#endif
}

///
class KWavePlayer
{
public:
    virtual ~KWavePlayer()	{}//				PURE;

    STDPROC Run()							PURE;
    STDPROC Pause()							PURE;
    STDPROC Resume()						PURE;
    STDPROC Stop()							PURE;

    STDPROC SetVolume(ULONG)				PURE;

    STDPROC_(STREAM_TIME) GetDuration()		PURE;
    STDPROC_(STREAM_TIME) GetPosition()		PURE;
    STDPROC SetPosition(STREAM_TIME)		PURE;
    STDPROC_(KFILTER_STATE) GetState()		PURE;
};

//KWavePlayer::~KWavePlayer() {}


#endif // ADDITIONAL_H_INCLUDED
