// Copyright 2023 Egill Ingi Jacobsen

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <memory>



void generateDiffuseIrradiance(float* out, int width_out, int height_out, float* pixels, int width, int height)
{
    for(int y = 0; y < height_out; ++y) {
        for(int x = 0; x < width_out; ++x) {
            float u = static_cast<float>(x + 0.5f) / static_cast<float>(width_out);
            float v = static_cast<float>(y + 0.5f) / static_cast<float>(height_out);

            // Calculate normal

            // Sample a microfacet distribution a number of times and take average results
            // or
            // Uniformly sample hemisphere oriented in the normal direction a number of times and average results



            uint32_t xx = static_cast<uint32_t>(u * static_cast<float>(width));
            uint32_t yy = static_cast<uint32_t>(v * static_cast<float>(height));
            uint32_t ind2 = xx*3 + yy * (width-1) * 3;

            int ind = x*3 + y*(width_out-1)*3;
            out[ind + 0] = pixels[ind2 + 0];
            out[ind + 1] = pixels[ind2 + 1];
            out[ind + 2] = pixels[ind2 + 2];
        }
    }
}

int main() 
{
    int width, height, elements;
    const char* filename = "sandsloot_2k.hdr";
    float* pixels = stbi_loadf(filename, &width, &height, &elements, 3);

    int diffuseWidth = 512;
    int diffuseHeight = 256;

    float* outPixels = (float*) malloc(diffuseWidth * diffuseHeight * 3 * sizeof(float));
    generateDiffuseIrradiance(outPixels, diffuseWidth, diffuseHeight, pixels, width, height);

    stbi_write_hdr("testDiff.hdr", diffuseWidth, diffuseHeight, 3, outPixels);

}