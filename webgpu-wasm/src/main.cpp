
#include <emscripten.h>
#include <emscripten/html5.h>

#include "Application.h"

#ifndef __EMSCRIPTEN__
#define EMSCRIPTEN_KEEPALIVE
#endif

// #define WAIT_FOR_QUEUE 1

static Application* s_Application = nullptr;

static float triangleVertices[3*5] = {-0.5f, -0.5f, 0.f, 0.0f, 0.0f,
                                    0.5, -0.5, 0.f, 1.0f, 0.0f,
                                    0.f, 0.5f, 0.f, 0.5f, 1.0f};
static uint32_t triangleIndices[3] = {0, 1, 2};

static long s_AnimationFrameId;

extern "C" void EMSCRIPTEN_KEEPALIVE changeModes(int newMode) {
    printf("New mode: %i\n", newMode);
    if(s_Application){
        // emscripten_cancel_animation_frame(s_AnimationFrameId);
        Application::State newState = Application::State::Other;
        if(newMode == 0) newState = Application::State::Rasterizer;
        else if(newMode == 1) newState = Application::State::PathTracer;

        s_Application->transition(newState);
    }
}


int onUpdate(double t, void* userData) {
    if(s_Application) s_Application->onUpdate();

    #ifndef WAIT_FOR_QUEUE
    s_AnimationFrameId = emscripten_request_animation_frame(onUpdate, nullptr);
    #endif

    return 1;
}


int main()
{
    s_Application = new Application("WebGPU Application");

    // Start main loop
    #ifdef WAIT_FOR_QUEUE
    onUpdate(0.0, nullptr);
    #else
    emscripten_request_animation_frame(onUpdate, nullptr);
    // emscripten_set_main_loop(onUpdate, 0, true);
    #endif

}