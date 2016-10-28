
#include "Engine.h"

int main(void)
{

    rengine::Engine engine(800, 600);

    if (engine.init()) {
        engine.run();
    }

    return 0;

}