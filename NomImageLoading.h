

#pragma once

#include "ATF/NomRenderingHeaders.h"
#include "ATLUtil/math2d.h"
#include "ATLUtil/debug_break.h"

#include <string>

namespace atl_graphics_namespace_config
{
    unsigned int getNextPow2(unsigned int in_src);
    
    struct BMPPixel
    {
        unsigned char & r;
        unsigned char & g;
        unsigned char & b;
        unsigned char & a;
        BMPPixel(unsigned char * in_pixelAddress) :
            r(in_pixelAddress[0]),
            g(in_pixelAddress[1]),
            b(in_pixelAddress[2]),
            a(in_pixelAddress[3])
        {}
    };
    
    template <int BPP>
    const unsigned char * getImagePixelPtr(const unsigned char * in_data, unsigned int in_x, unsigned int in_y, unsigned int in_width, unsigned int in_height) {
        SGDebugBreakIf(in_x >= in_width && in_y >= in_height, "Out of bounds access to image");
        return &in_data[in_y * in_width * BPP + in_x * BPP];
    };

    template <int BPP, int ByteIdx>
    unsigned char getImagePixelVal(const unsigned char * in_data, unsigned int in_x, unsigned int in_y, unsigned int in_width, unsigned int in_height) {
        SGDebugBreakIf(in_x >= in_width && in_y >= in_height, "Out of bounds access to image");
        return in_data[in_y * in_width * BPP + in_x * BPP + ByteIdx];
    };
    
    template <int BPP>
    unsigned char * getEditableImagePixelPtr(unsigned char * in_data, unsigned int in_x, unsigned int in_y, unsigned int in_width, unsigned int in_height) {
        SGDebugBreakIf(in_x >= in_width && in_y >= in_height, "Out of bounds access to image");
        return &in_data[in_y * in_width * BPP + in_x * BPP];
    };
    
    template <int BPP, int ByteIdx>
    unsigned char & getEditableImagePixelVal(unsigned char * in_data, unsigned int in_x, unsigned int in_y, unsigned int in_width, unsigned int in_height) {
        SGDebugBreakIf(in_x >= in_width && in_y >= in_height, "Out of bounds access to image");
        return in_data[in_y * in_width * BPP + in_x * BPP + ByteIdx];
    };
    
    template <int BPP>
    BMPPixel getEditableImagePixel(unsigned char * in_data, unsigned int in_x, unsigned int in_y, unsigned int in_width, unsigned int in_height) {
        SGDebugBreakIf(in_x >= in_width && in_y >= in_height, "Out of bounds access to image");
        return BMPPixel(getEditableImagePixelPtr<BPP>(in_data, in_x, in_y, in_width, in_height));
    };
    
    void findImageNonEmptyArea(const unsigned char * in_data, const atl::size2f & in_imageDim, atl::box2f & out_nonEmptyArea); // Trim image down to minimum size needed
    
    void diag(const unsigned char * in_data, unsigned int in_width, unsigned int in_height, const std::string & in_ident);
    void diagReset();
}
