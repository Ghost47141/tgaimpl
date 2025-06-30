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
} TGA_Header;
#pragma pack(pop)

typedef struct
{
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
} TGA_Color;

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

int tga_create_image(const char* _filename, uint16_t _width, uint16_t _height, TGA_Color _blankColor) 
{
    FILE* file = fopen(_filename, "wb");
    if(file == NULL) { 
        fprintf(stderr,"Error: Failed to open %s\n",_filename);
        return -1; 
    }

    TGA_Header header = {0};
    header.imageType = TGA_TYPE_TRUE_COLOR;
    header.width = _width;
    header.height = _height;
    header.pixelDepth = TGA_FORMAT_BGRA32;

    // Write header into the file binary.
    if(fwrite(&header,sizeof(header),1,file) != 1){
        fprintf(stderr, "Error: Failed to write TGA header into %s\n", _filename);
        fclose(file);
        return -1;
    }
    
    // Allocate memory for pixelArray.
    size_t pixelArraySize = _width * _height * (header.pixelDepth/8);
    uint8_t* pixelArray = (uint8_t*)malloc(pixelArraySize);
    if(pixelArray == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for pixel array");
        fclose(file);
        return -1;
    }
    
    // fill them with blankColor
    for (size_t i = 0; i < pixelArraySize; i += (header.pixelDepth/8)) {
        pixelArray[i]     = _blankColor.blue;
        pixelArray[i + 1] = _blankColor.green;
        pixelArray[i + 2] = _blankColor.red;
        pixelArray[i + 3] = _blankColor.alpha;
    }

    // Write pixel data into file binary.
    if(fwrite(pixelArray,pixelArraySize,1,file) != 1) {
        fprintf(stderr, "Error: Failed to write pixel data into %s\n",_filename);
        free(pixelArray);
        fclose(file);
        return -1;
    }
    free(pixelArray);
    fclose(file);
    return 0;
}
