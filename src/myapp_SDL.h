#include <SDL.h>

class MyApp {
public:
    void run() {
        initAll();
        mainLoop();
        cleanUp();
    }
private:
    void initAll();
    void mainLoop();
    void cleanUp();
};