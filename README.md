# webgpu-wasm
This is a hobby project and the goal of it is to learn and experiment with WebGPU and its shading language WGSL.
The code is predominantly written in C++ and is compiled to WebAssembly and javascript using the Emscripten compiler toolchain.

As version one of WebGPU is still under development, browser support has not been released in stable browser versions. The project has been tested on Google Chromes experimental nighlt builds, Chrome Canary. To run it in Chrome Canary the experimental flag: *Unsafe WebGPU* needs to be enabled.

As stated this is a hobby project and is only meant for learning. This means decisions about architecture, algorithms and other aspects of the code are not necessarily final and are often made the "What takes the shortest time to implement?" way.

To build this project you should use CMake to generate build files. You need to make sure it uses the Emscripten CMake toolchain file and a compatible generator (Unix Makefiles are known to be working)

## Features
 + Real-Time rasterized rendering using the WebGPU API
 + Deferred pipeline
 + Physically based shading
   + Based on shaders from https://learnopengl.com/
 + Simple camera animation
 + Simple Ray Tracing of sphere primitives in a compute shader
 
 ## Possible future features
  + Full HDR pipeline with Image Based Lighting
  + Reflections
  + Shadows
  + Post Processing
    + Ambient Occlusion
    + Bloom
  + Mesh animations
  + Procedurally generated geometry using compute shaders
