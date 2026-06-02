#include "Bitmap.h"

#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdint>

// Incarca fisierul BMP primit prin path si returneaza ID-ul texturii OpenGL
unsigned int loadBitmapTexture(const char* path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f)
    {
        std::cerr << "Bitmap: nu pot deschide " << path << "\n";
        return 0;
    }

    unsigned char fh[14];
    f.read(reinterpret_cast<char*>(fh), 14);
    if (!f || fh[0] != 'B' || fh[1] != 'M')
    {
        std::cerr << "Bitmap: " << path << " nu are signatura BM (nu e BMP)\n";
        return 0;
    }

    // Citeste offsetul de unde incepe zona cu pixeli in fisier
    uint32_t pixOffset =
          (uint32_t)fh[10]
        | ((uint32_t)fh[11] << 8)
        | ((uint32_t)fh[12] << 16)
        | ((uint32_t)fh[13] << 24);

    unsigned char ih[40];
    f.read(reinterpret_cast<char*>(ih), 40);
    if (!f)
    {
        std::cerr << "Bitmap: " << path << " info header incomplet\n";
        return 0;
    }

    // Extrage latimea din header in format little endian
    int32_t width =
          (int32_t)((uint32_t)ih[4]
        | ((uint32_t)ih[5] << 8)
        | ((uint32_t)ih[6] << 16)
        | ((uint32_t)ih[7] << 24));

    // Extrage inaltimea din header in format little endian
    int32_t height =
          (int32_t)((uint32_t)ih[8]
        | ((uint32_t)ih[9] << 8)
        | ((uint32_t)ih[10] << 16)
        | ((uint32_t)ih[11] << 24));

    // Citeste numarul de biti folositi pentru fiecare pixel
    uint16_t bpp =
          (uint16_t)ih[14]
        | ((uint16_t)ih[15] << 8);

    // Citeste tipul de compresie al imaginii
    uint32_t comp =
          (uint32_t)ih[16]
        | ((uint32_t)ih[17] << 8)
        | ((uint32_t)ih[18] << 16)
        | ((uint32_t)ih[19] << 24);

    // Loaderul accepta doar BMP-uri simple cu 24 de biti pe pixel
    if (bpp != 24)
    {
        std::cerr << "Bitmap: " << path
                  << " are " << bpp << " bpp; suport doar 24-bit\n";
        return 0;
    }

    // Nu sunt acceptate BMP-uri comprimate
    if (comp != 0)
    {
        std::cerr << "Bitmap: " << path
                  << " e comprimat (compression=" << comp
                  << "); suport doar BI_RGB (uncompressed)\n";
        return 0;
    }

    // Verifica daca imaginea este salvata de sus in jos
    bool topDown = false;
    if (height < 0) { height = -height; topDown = true; }

    // Opreste incarcarea daca dimensiunile nu sunt valide
    if (width <= 0 || height <= 0)
    {
        std::cerr << "Bitmap: " << path << " are dimensiuni invalide\n";
        return 0;
    }

    // Muta cursorul fisierului la inceputul datelor de pixeli
    f.seekg(pixOffset, std::ios::beg);
    if (!f)
    {
        std::cerr << "Bitmap: " << path << " nu pot face seek la pixel data\n";
        return 0;
    }

    // Calculeaza dimensiunea unei linii si paddingul impus de formatul BMP
    int rowBytes  = width * 3;
    int rowStride = (rowBytes + 3) & ~3;

    std::vector<uint8_t> raw((size_t)rowStride * height);
    f.read(reinterpret_cast<char*>(raw.data()), raw.size());
    if (!f)
    {
        std::cerr << "Bitmap: " << path << " pixel data incomplet\n";
        return 0;
    }

    // Pregateste un buffer RGB fara padding pentru OpenGL
    std::vector<uint8_t> rgb((size_t)width * height * 3);
    for (int y = 0; y < height; ++y)
    {
        // Pentru BMP top down se inverseaza liniile ca orientarea sa ramana corecta in OpenGL
        int srcRow = topDown ? (height - 1 - y) : y;

        const uint8_t* src = raw.data() + (size_t)srcRow * rowStride;
        uint8_t*       dst = rgb.data() + (size_t)y      * width * 3;

        for (int x = 0; x < width; ++x)
        {
            // BMP salveaza pixelii in ordine BGR, iar OpenGL primeste aici date RGB
            dst[x * 3 + 0] = src[x * 3 + 2];
            dst[x * 3 + 1] = src[x * 3 + 1];
            dst[x * 3 + 2] = src[x * 3 + 0];
        }
    }

    unsigned int tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Seteaza alinierea la 1 deoarece bufferul RGB nu are padding intre linii
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Trimite imaginea in memoria video ca textura 2D RGB
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 width, height, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, rgb.data());

    // Revine la alinierea implicita folosita de OpenGL
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // Genereaza mipmapuri pentru afisare mai buna la distante diferite
    glGenerateMipmap(GL_TEXTURE_2D);

    // Textura nu se repeta in afara coordonatelor normale
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    // Seteaza filtrarea texturii la micsorare si marire
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return tex;
}