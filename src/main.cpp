
#include "Application.h"

int main(void)
{

    rengine::Application app(800, 600);

    if (app.init()) {
        app.run();
    }

    return 0;

}