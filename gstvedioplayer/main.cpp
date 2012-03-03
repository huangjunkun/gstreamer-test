
#include <iostream>
using namespace std;

#include <gst/gst.h>
#include "kgstvedioplayer.h"
#include "main.h"

static void* PlayerRunOnThreadProc( void* arg)
{
    KGSTVedioPlayer* pPlayer= reinterpret_cast<KGSTVedioPlayer*>(arg);
    for(; ;)
    {
        pPlayer->implRun();
    }
    return  (void*) 0;
}
void* TestPrint(void*)
{
    for (int i=0; i<20; i++)
    {
        std::cout << i << " : testPrint() ... sleep(1) \n";
        sleep(1);
    }
    return (void*) 2;
}

int main(int argc, char* argv[])
{
    const char* mediafile = "/home/huangjunkun/desktop/movie.avi";//music.mp3
//    PlayVedioWithPlaybin("file:///home/huangjunkun/desktop/movie.avi");
    test_main(mediafile);
    /*
        {
        KGSTVedioPlayer    player;
        player.LoadFile(mediafile);
        player.implRun();
        }
    /*
        {
        int     err;
        pthread_t tid1, tid2;
        KGSTVedioPlayer player(mediafile, true);
    //    player.Run();//ready ...
    //    std::cout << (u_int64_t)player.GetDuration() << " : GetDuration() \n";
        err = pthread_create(&tid1, NULL, PlayerRunOnThreadProc, &player);
        //
        if (err != 0)
            std::cout << "pthread_create error ...\n";
        err = pthread_create(&tid2, NULL, TestPrint, NULL);
        if (err != 0)
            std::cout << "pthread_create error ...\n";
        char    ch;
        do
        {
            std::cout << "q or Q for exit :";
    //        std::cin >> ch;
            std::cin.get(ch) ;
            std::cin.ignore(100, '\n');
            switch(ch)
            {
            case 'p':
            case 'P':
                player.Pause() ;
                break;
            case 'g':
            case 'G':
                player.Run() ;
                break;
            case 'x':
            case 'X':
                player.Stop() ;
                break;
            case 'r':
            case 'R':
                player.Resume() ;
                break;
            case 'd':
            case 'D':
                std::cout << (u_int64_t)player.GetDuration() << " : GetDuration() \n";
                break;
            case 'o':
            case 'O':
                std::cout << player.GetPosition() << " : GetPosition() \n";
                break;
            case 'f':
            case 'F':
                std::cout << player.SetPosition(player.GetPosition()+5) << " : 0.05 + SetPosition() +0.05\n";
                break;
            case 's':
            case 'S':
                std::cout << player.SetPosition(player.GetPosition()-5) << " : 0.05 + SetPosition() -0.05\n";
                break;
            case '+':
                std::cout << player.SetVolume(player.GetVolume()+10) << " : SetVolume() +10\n";
                break;
            case '-':
                std::cout << player.SetVolume(player.GetVolume()-10) << " : SetVolume() -10\n";
                break;
            default:
                std::cout << " Inpution is ignored ... \n" ;
            }
        }while(ch != 'q' && ch != 'Q');
        pthread_cancel(tid1);// ...
        pthread_cancel(tid2);// ..
        }
    /*
        {
        KGSTVedioPlayer    player(mediafile);
    //    player.LoadFile(mediafile);
        player.implRun();
        }
        /** =========================*/
    cout << "Hello world!" << endl;
    return 0;
}
