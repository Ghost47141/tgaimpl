#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct
{
    uint8_t idLength;          // Field 1: Number of bytes in the Image ID Field 6
    uint8_t colorMapType;      // Field 2: LUT (0 = no color map & FIELD 7 empty, 1 = has color map)
    uint8_t imageType;         // Field 3: Type of image (0 = no image data, .., 2 = uncompressed true-color, etc.)
    uint16_t colorMapOrigin;   // Field 4.1: First color map entry index (leave value 0 if Field 2 = 0)
    uint16_t colorMapLength;   // Field 4.2: Number of color map entries (leave value 0 if Field 2 = 0)
    uint8_t colorMapDepth;     // Field 4.3: Bits per color map entry (leave value 0 if Field 2 = 0)
    uint16_t xOrigin;          // Field 5.1: X-coordinate of lower-left corner
    uint16_t yOrigin;          // Field 5.2: Y-coordinate of lower-left corner
    uint16_t width;            // Field 5.3: Image width in pixels
    uint16_t height;           // Field 5.4: Image height in pixels
    uint8_t pixelDepth;        // Field 5.5: AKA : Bits per pixel (e.g., 24 for RGB, etc)
    uint8_t imageDescriptor;   // Field 5.6: Image descriptor (e.g., alpha bits, orientation, etc)
} TGAHeader;
#pragma pack(pop)

typedef struct
{
    TGAHeader header;
    uint8_t* pixelArray;
    size_t pixelArraySize;
} TGAImage;

typedef struct
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} TGAColor8;

enum TGA_Image_Type
{
    TGA_TYPE_NO_IMAGE            = 0,       // No image data included
    TGA_TYPE_COLOR_MAPPED        = 1,       // Uncompressed, Color mapped image
    TGA_TYPE_TRUE_COLOR          = 2,       // Uncompressed, True Color Image
    TGA_TYPE_BLACK_AND_WHITE     = 3,       // Uncompressed, black-and-white (grayscale) image
    TGA_TYPE_RLE_COLOR_MAPPED    = 9,       // Run-length encoded (RLE), color-mapped image
    TGA_TYPE_RLE_TRUE_COLOR      = 10,      // Run-length encoded (RLE), true-color image
    TGA_TYPE_RLE_BLACK_AND_WHITE = 11       // Run-length encoded (RLE), black-and-white (grayscale) image
};
enum TGA_Pixel_Format
{
    TGA_FORMAT_BW8    = 8,
    TGA_FORMAT_BGR15  = 15,
    TGA_FORMAT_BGR24  = 24,
    TGA_FORMAT_BGRA32 = 32
};

// Creates a TGAImage structure into memory
TGAImage tga_create_image(uint16_t _width, uint16_t _height, TGAColor8 _blankColor)
{
    // Set up TGA Header
    TGAImage image = {0};
    image.header.imageType = TGA_TYPE_TRUE_COLOR;
    image.header.width = _width;
    image.header.height = _height;
    image.header.pixelDepth = TGA_FORMAT_BGRA32;

    // Allocate memory for pixelArray
    image.pixelArraySize = _width * _height * (image.header.pixelDepth/8);
    image.pixelArray = (uint8_t*)malloc(image.pixelArraySize);
    if(image.pixelArray == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for pixel array");
        return image;
    }

    // Fill them with blankColor
    for (size_t i = 0; i < image.pixelArraySize; i += (image.header.pixelDepth/8)) {
        image.pixelArray[i]     = _blankColor.blue;
        image.pixelArray[i + 1] = _blankColor.green;
        image.pixelArray[i + 2] = _blankColor.red;
        image.pixelArray[i + 3] = _blankColor.alpha;
    }
    return image;
}

/**
 *  @param[in] _filename Make sure to include .tga after writing the desired name (i.e., "test.tga")
 *  @param[in] _image Writes TGAImage onto disc as a .tga
 */
int tga_write_image(const char* _filename, TGAImage _image)
{
    FILE* file = fopen(_filename, "wb");
    if(file == NULL) {
        fprintf(stderr,"Error: Failed to open %s\n",_filename);
        return -1;
    }
    // Write header into the file binary.
    if(fwrite(&_image.header,sizeof(_image.header),1,file) != 1){
        fprintf(stderr, "Error: Failed to write TGA header into %s\n", _filename);
        fclose(file);
        return -1;
    }

    // Write pixel data into file binary.
    if(fwrite(_image.pixelArray,_image.pixelArraySize,1,file) != 1) {
        fprintf(stderr, "Error: Failed to write pixel data into %s\n", _filename);
        fclose(file);
        return -1;
    }
    fclose(file);
    return EXIT_SUCCESS;
}

// Colors a single pixel
int tga_set_pixel(uint16_t _x, uint16_t _y, TGAImage *_image, TGAColor8 _color,bool isTopToBottom)
{
    if (_x >= _image->header.width || _y >= _image->header.height) {
        fprintf(stderr, "Error: Invalid pixel coordinates requested\n");
        return EXIT_FAILURE;
    }

    // ( width * _y ) + (_x + 1) = When counting starts from 1 i.e in a 10 x 10 grid, last is 10,10
    // ( width * _y ) + _x = When counting starts from 0 i.e in a 10 x 10 grid, last is 9,9
    if (isTopToBottom)
    {
        // new y = height -1 -_y
        _y = _image->header.height - 1 - _y;
    }
    const uint16_t pixelNum = (_image->header.width * _y) + _x;
    const uint16_t bytesPerPixel = _image->header.pixelDepth / 8;

    _image->pixelArray[pixelNum * bytesPerPixel] = _color.blue;
    _image->pixelArray[(pixelNum * bytesPerPixel) +1] =_color.green;
    _image->pixelArray[(pixelNum * bytesPerPixel) +2] =_color.red;
    _image->pixelArray[(pixelNum * bytesPerPixel) +3] =_color.alpha;

    return EXIT_SUCCESS;
}