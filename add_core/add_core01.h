#ifndef ADD_CORE01_H_INCLUDED
#define ADD_CORE01_H_INCLUDED

#pragma once

//  #define   STDMETHOD(method)               virtual   HRESULT   STDMETHODCALLTYPE   method
//  #define   STDMETHODCALLTYPE               __stdcall
//  这样当写一个函数STDMETHOD(op1(int   i))
//  展开后成为：     virtual   HRESULT   __stdcall   op1(int   i);
//
//  这样就这义了一个虚函数op1。当然了这个虚函数只能放在接口定义中了。
//  所以STDMETHOD宏是用于定义接口用的。放在头文件中用。
//
//  当要在CPP中实现这个方法时就用到另一个宏STDMETHOD
//  #define   STDMETHODIMP                         HRESULT   STDMETHODCALLTYPE
//  #define   STDMETHODCALLTYPE               __stdcall
//  这样CPP文件中，STDMETHODIMP(op1(int   i))就展开为：
//
//  HRESULT   __stdcall   op1(int   i);

#include <string>
#include <iostream>

#ifndef PURE
#define PURE    =0
#endif

#define HGBL HGLOBAL
#define STDPROC virtual int //__stdcall
#define STDMETHODIMP    int //__stdcall
#define STDPROC_(Type) virtual Type //__stdcall
#define STDMETHODIMP_(Type) Type //__stdcall
#define STREAM_TIME long long
#define HRESULT int


typedef std::string ks_string;

typedef enum _KFilterState
{
    KState_NotReady,
    KState_Open,
    KState_Stopped,
    KState_Paused,
    KState_Running,
    KState_Record,
    KState_Seek,
    KState_Unknown,
} KFILTER_STATE;

// functions
#define XGlobalUnlock   GlobalUnlock
#define XGlobalLock   GlobalLock
#define XGlobalSize GlobalSize
namespace
{
class TException : public std::exception
{
public:
    TException() :_what("a TException ...") {}
    TException(const std::string& str) :_what(str.c_str()) {}
    ~TException() throw() {}
    const char* what() const throw()
    {
        return _what;
    }
private:
    const char* _what;
};

#define THROW_ERROR 0
#define PRINT_ERROR_EXIT    1
#define PRINT_ERROR_RETURN    2
#define HANDLE_ERROR_MODE   THROW_ERROR

#if (HANDLE_ERROR_MODE==THROW_ERROR)
#define HANDLE_ERROR(what)  throw TException(what)
#elif (HANDLE_ERROR_MODE==PRINT_ERROR_EXIT)
#define HANDLE_ERROR(what)  { perror(what); exit(1);}
#else
#define HANDLE_ERROR(what)  { perror(what); return 1;}
#endif
}

//#define TRACE_ERR(errtext, result) TRACEW(TempStr, L##errtext), result
static int print(const std::string& str)
{
    std::cout << str << "\n";
    return 1;
}
#define SAFE_DELETE_ARRAY(p) if ((p)) { delete [] (p); (p) = NULL; }

#define SAFE_DELETE(p) if ((p)) { delete (p); (p) = NULL; }
#define _WCH(X) (X)

#define TRACE_ERR(errtext, result)  (print(errtext), result)


#endif // ADDITIONAL_H_INCLUDED
