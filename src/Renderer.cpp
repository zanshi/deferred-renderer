//
// Created by Niclas Olmenius on 2016-10-28.
//

#include "Renderer.h"

namespace rengine {

Renderer::Renderer() {
    assert(!instantiated_);
    instantiated_ = true;
}

Renderer::~Renderer() { }


}