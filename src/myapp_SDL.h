#include <SDL.h>

//std libraries
#include <iostream>

const unsigned int MYAPP_WINDOW_WIDTH_DEFAULT = 1280;
const unsigned int MYAPP_WINDOW_HEIGHT_DEFAULT = 720;

enum MyApp_Error {
    MyApp_success,
    MyApp_fail
};

class MyApp {
public:
    MyApp():_window(nullptr),_renderer(nullptr){}
public:
    void run() {
        MyApp_Init();
        mainLoop();
        cleanUp();
    }
private:
    MyApp_Error MyApp_Init();
    void mainLoop();
    MyApp_Error cleanUp();

    MyApp_Error MyApp_SDL_Init(unsigned int _w, unsigned int _h, const char *_title);
    MyApp_Error MyApp_SDL_Exit();
private:
    SDL_Window *_window;
    SDL_Renderer *_renderer;


};