
#include "Engine.h"

int main(void)
{

    rengine::Engine engine(800, 600);

    engine.start_up();

    engine.run();

    engine.shut_down();

    return 0;

}