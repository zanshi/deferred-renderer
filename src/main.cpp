
#include "Engine.h"

int main(void)
{

    rengine::Engine engine(1680, 1050);

    engine.start_up();

    engine.run();

    engine.shut_down();

    return 0;

}