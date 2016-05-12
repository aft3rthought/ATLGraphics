

#include "ATLUtil/debug_break.h"
#include "ATF/NomImageLoading.h"

namespace atl_graphics_namespace_config
{
    unsigned int getNextPow2(unsigned int in_src)
    {
        for(unsigned int cnt_pow = 0; cnt_pow < 13; cnt_pow++)
        {
            unsigned int l_nextPow2 = 1 << cnt_pow;
            if(in_src <= l_nextPow2)
                return l_nextPow2;
        }
        return 0;
    }


    void findImageNonEmptyArea(const unsigned char * in_data, const atl::size2f & in_imageDim, atl::box2f & out_nonEmptyArea)
    {
        out_nonEmptyArea.t = 0;
        out_nonEmptyArea.l = 0;
        out_nonEmptyArea.r = in_imageDim.w;
        out_nonEmptyArea.b = in_imageDim.h;

        bool l_continue;
        // - Trim from top Y:
        l_continue = true;
        for(unsigned int idx_searchY = out_nonEmptyArea.t; idx_searchY < out_nonEmptyArea.b; idx_searchY++)
        {
            for(unsigned int idx_searchX = out_nonEmptyArea.l; idx_searchX < out_nonEmptyArea.r; idx_searchX++)
            {
                if(getImagePixelVal<4,3>(in_data, idx_searchX, idx_searchY, in_imageDim.w, in_imageDim.h) != 0)
                {
                    l_continue = false;
                    break;
                }
            }
            if(!l_continue)
                break;
            out_nonEmptyArea.t++;
        }
        // - Trim from bottom Y:
        l_continue = true;
        for(unsigned int idx_searchY = out_nonEmptyArea.b - 1; idx_searchY > out_nonEmptyArea.t; idx_searchY--)
        {
            for(unsigned int idx_searchX = out_nonEmptyArea.l; idx_searchX < out_nonEmptyArea.r; idx_searchX++)
            {
                if(getImagePixelVal<4,3>(in_data, idx_searchX, idx_searchY, in_imageDim.w, in_imageDim.h) != 0)
                {
                    l_continue = false;
                    break;
                }
            }
            if(!l_continue)
                break;
            out_nonEmptyArea.b--;
        }
        // - Trim from left X:
        l_continue = true;
        for(unsigned int idx_searchX = out_nonEmptyArea.l; idx_searchX < out_nonEmptyArea.r; idx_searchX++)
        {
            for(unsigned int idx_searchY = out_nonEmptyArea.t; idx_searchY < out_nonEmptyArea.b; idx_searchY++)
            {
                if(getImagePixelVal<4,3>(in_data, idx_searchX, idx_searchY, in_imageDim.w, in_imageDim.h) != 0)
                {
                    l_continue = false;
                    break;
                }
            }
            if(!l_continue)
                break;
            out_nonEmptyArea.l++;
        }
        // - Trim from bottom Y:
        l_continue = true;
        for(unsigned int idx_searchX = out_nonEmptyArea.r - 1; idx_searchX > out_nonEmptyArea.l; idx_searchX--)
        {
            for(unsigned int idx_searchY = out_nonEmptyArea.t; idx_searchY < out_nonEmptyArea.b; idx_searchY++)
            {
                if(getImagePixelVal<4,3>(in_data, idx_searchX, idx_searchY, in_imageDim.w, in_imageDim.h) != 0)
                {
                    l_continue = false;
                    break;
                }
            }
            if(!l_continue)
                break;
            out_nonEmptyArea.r--;
        }
    }

    #ifdef DEBUG
    unsigned long g_bytesAllocated = 0;
    unsigned long g_bytesEmpty = 0;
    #endif

    void diagReset()
    {
    #ifdef DEBUG
        g_bytesAllocated = 0;
        g_bytesEmpty = 0;
    #endif
    }

    void diag(const unsigned char * in_data, unsigned int in_width, unsigned int in_height, const std::string & in_ident)
    {
    #ifdef DEBUG
        unsigned long l_newBytes = in_width * in_height * 4;
        unsigned long l_emptyBytes = 0;

        for(unsigned int idx_y = 0; idx_y < in_height; idx_y++)
        {
            for(unsigned int idx_x = 0; idx_x < in_width; idx_x++)
            {
                const unsigned char * l_ptr = getImagePixelPtr<4>(in_data, idx_x, idx_y, in_width, in_height);

                if(l_ptr[0] == 0 &&
                   l_ptr[1] == 0 &&
                   l_ptr[2] == 0 &&
                   l_ptr[3] == 0)
                    l_emptyBytes += 4;
            }
        }

        // Increase global sum:
        g_bytesAllocated += l_newBytes;
        g_bytesEmpty += l_emptyBytes;

        // Calculate this image's metrics:
        float l_pctEmpty = float(l_emptyBytes) / float(l_newBytes) * 100.f;
        float l_totalPctEmpty = float(g_bytesEmpty) / float(g_bytesAllocated) * 100.f;
        float l_totalMb = float(g_bytesAllocated) / 1024.f / 1024.f;

        printf("TEX: %s: %.1f%% empty pixels, TOTAL: %.3f MB, %.1f%% empty pixels\n", in_ident.c_str(), l_pctEmpty, l_totalMb, l_totalPctEmpty);
    #endif
    }
}