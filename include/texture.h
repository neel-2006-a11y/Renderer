#pragma once
#include <string>
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include <stb_image.h>

struct Texture {
    unsigned int id = 0;
    std::string type;
    std::string path;
};

struct GPUTexture{
    unsigned int id = 0;
};

unsigned int TextureFromFile(const std::string& path);
