
#include "Skybox.h"
#include "Shader.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>

// Starea interna
static unsigned int s_cubemapTex  = 0;
static unsigned int s_vao         = 0;
static unsigned int s_vbo         = 0;
static unsigned int s_shader      = 0;
static int          s_viewLoc     = -1;
static int          s_projLoc     = -1;
static int          s_texLoc      = -1;

// Varfurile cubului folosit pentru skybox
// Pozitia vertexului este folosita ca directie de sampling
static const float kSkyboxVerts[] =
{
    // fata -Z (back in world)
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    // fata -X (left)
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    // fata +X (right)
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    // fata +Z (front in world)
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    // fata +Y (top)
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    // fata -Y (bottom)
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
};

// incarca BMP 24-bit si returneaza pixelii RGB raw
static bool loadBMPRaw(
    const char*            path,
    std::vector<uint8_t>&  out,
    int&                   w,
    int&                   h)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        std::cerr << "Skybox: nu pot deschide " << path << "\n";
        return false;
    }

    unsigned char fh[14];
    f.read(reinterpret_cast<char*>(fh), 14);
    if (!f || fh[0] != 'B' || fh[1] != 'M')
    {
        std::cerr << "Skybox: " << path << " nu e BMP valid\n";
        return false;
    }
    uint32_t pixOffset =
          (uint32_t)fh[10]
        | ((uint32_t)fh[11] << 8)
        | ((uint32_t)fh[12] << 16)
        | ((uint32_t)fh[13] << 24);

    unsigned char ih[40];
    f.read(reinterpret_cast<char*>(ih), 40);
    if (!f) { std::cerr << "Skybox: " << path << " header incomplet\n"; return false; }

    int32_t width =
          (int32_t)((uint32_t)ih[4]
        | ((uint32_t)ih[5] << 8)
        | ((uint32_t)ih[6] << 16)
        | ((uint32_t)ih[7] << 24));
    int32_t height =
          (int32_t)((uint32_t)ih[8]
        | ((uint32_t)ih[9] << 8)
        | ((uint32_t)ih[10] << 16)
        | ((uint32_t)ih[11] << 24));
    uint16_t bpp   = (uint16_t)ih[14] | ((uint16_t)ih[15] << 8);
    uint32_t comp  = (uint32_t)ih[16] | ((uint32_t)ih[17] << 8)
                   | ((uint32_t)ih[18] << 16) | ((uint32_t)ih[19] << 24);

    if (bpp != 24 || comp != 0)
    {
        std::cerr << "Skybox: " << path << " trebuie sa fie BMP 24-bit uncompressed\n";
        return false;
    }

    bool topDown = false;
    if (height < 0) { height = -height; topDown = true; }
    if (width <= 0 || height <= 0) { return false; }

    f.seekg(pixOffset, std::ios::beg);

    int rowStride = (width * 3 + 3) & ~3;
    std::vector<uint8_t> raw((size_t)rowStride * height);
    f.read(reinterpret_cast<char*>(raw.data()), raw.size());
    if (!f) { std::cerr << "Skybox: " << path << " date incomplete\n"; return false; }

    out.resize((size_t)width * height * 3);

    // Converteste pixelii din BGR in RGB
    for (int y = 0; y < height; ++y)
    {
        // Inverseaza randurile pentru orientarea corecta
        int bmpRow = topDown ? y : (height - 1 - y);
        const uint8_t* src = raw.data() + (size_t)bmpRow * rowStride;
        uint8_t*       dst = out.data()  + (size_t)y      * width * 3;

        for (int x = 0; x < width; ++x)
        {
            dst[x * 3 + 0] = src[x * 3 + 2]; // R
            dst[x * 3 + 1] = src[x * 3 + 1]; // G
            dst[x * 3 + 2] = src[x * 3 + 0]; // B
        }
    }

    w = (int)width;
    h = (int)height;
    return true;
}

// Initializeaza skybox-ul
void initSkybox()
{
    // Creeaza shader-ul skybox-ului
    s_shader = createShaderProgram("shaders/skybox.vert", "shaders/skybox.frag");
    if (s_shader == 0)
    {
        std::cerr << "Skybox: shader invalid\n";
        return;
    }
    s_viewLoc = glGetUniformLocation(s_shader, "view");
    s_projLoc = glGetUniformLocation(s_shader, "projection");
    s_texLoc  = glGetUniformLocation(s_shader, "skybox");

    // Creeaza VAO si VBO pentru cub
    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);

    glBindVertexArray(s_vao);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kSkyboxVerts), kSkyboxVerts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Creeaza textura cubemap
    // Ordinea fetelor pentru cubemap
    // POSITIVE_X = right,  NEGATIVE_X = left
    // POSITIVE_Y = top,    NEGATIVE_Y = bottom
    // POSITIVE_Z = front,  NEGATIVE_Z = back
    // maparea front<->back sau activeaza flipul vertical mai jos
    struct FaceDesc
    {
        const char* path;
        GLenum      target;
    };

    // in sens orar, urmatoarea fata e NEGATIVE_X => right.bmp, etc
    const FaceDesc faces[] =
    {
        { "skybox/left.bmp",   GL_TEXTURE_CUBE_MAP_POSITIVE_X },
        { "skybox/right.bmp",  GL_TEXTURE_CUBE_MAP_NEGATIVE_X },
        { "skybox/top.bmp",    GL_TEXTURE_CUBE_MAP_POSITIVE_Y },
        { "skybox/bottom.bmp", GL_TEXTURE_CUBE_MAP_NEGATIVE_Y },
        { "skybox/front.bmp",  GL_TEXTURE_CUBE_MAP_POSITIVE_Z },
        { "skybox/back.bmp",   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z },
    };

    glGenTextures(1, &s_cubemapTex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, s_cubemapTex);

    for (const auto& fd : faces)
    {
        std::vector<uint8_t> pixels;
        int w = 0, h = 0;

        if (loadBMPRaw(fd.path, pixels, w, h))
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(
                fd.target, 0, GL_RGB,
                w, h, 0,
                GL_RGB, GL_UNSIGNED_BYTE,
                pixels.data());
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Activeaza interpolarea corecta intre fetele cubemap-ului
    // texelii peste margini in loc sa lase un gap negru. OpenGL 3.2+
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// Deseneaza skybox-ul
void drawSkybox(const float* viewNoTranslate, const float* projection)
{
    if (s_shader == 0 || s_vao == 0 || s_cubemapTex == 0) return;

    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);   // cerul nu scrie in depth buffer

    glUseProgram(s_shader);
    glUniformMatrix4fv(s_viewLoc, 1, GL_FALSE, viewNoTranslate);
    glUniformMatrix4fv(s_projLoc, 1, GL_FALSE, projection);
    glUniform1i(s_texLoc, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, s_cubemapTex);

    glBindVertexArray(s_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Restaureaza starea depth pentru restul scenei
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
}

// Elibereaza resursele skybox-ului
void cleanupSkybox()
{
    if (s_vao)        { glDeleteVertexArrays(1, &s_vao);    s_vao = 0; }
    if (s_vbo)        { glDeleteBuffers(1, &s_vbo);         s_vbo = 0; }
    if (s_cubemapTex) { glDeleteTextures(1, &s_cubemapTex); s_cubemapTex = 0; }
    if (s_shader)     { glDeleteProgram(s_shader);          s_shader = 0; }
}
