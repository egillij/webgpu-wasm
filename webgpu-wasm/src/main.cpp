
#include <emscripten.h>

#include "Application.h"

static Application* s_Application = nullptr;

static float triangleVertices[3*5] = {-0.5f, -0.5f, 0.f, 0.0f, 0.0f,
                                    0.5, -0.5, 0.f, 1.0f, 0.0f,
                                    0.f, 0.5f, 0.f, 0.5f, 1.0f};
static uint32_t triangleIndices[3] = {0, 1, 2};

void onUpdate() {
    if(s_Application) s_Application->onUpdate();
}


int main()
{
    s_Application = new Application("WebGPU Application");

    // Start main loop
    emscripten_set_main_loop(onUpdate, 0, true);

}