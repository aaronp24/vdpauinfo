/*
Query and display NVIDIA VDPAU capabilities, a la glxinfo

Copyright (c) 2008 Wladimir J. van der Laan

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/
/// TODO
/// - parse display/screen from command line
/// - list color table formats for queryOutputSurface

#include <stdlib.h>
#include <stdio.h>
#include <vdpau/vdpau.h>
#include <vdpau/vdpau_x11.h>

#include "VDPDeviceImpl.h"

/******************* Base ****************/

void queryBaseInfo(VDPDeviceImpl *device)
{
    uint32_t api;
    const char *info;
    if(device->GetApiVersion(&api) != VDP_STATUS_OK ||
       device->GetInformationString(&info) != VDP_STATUS_OK)
    {
        fprintf(stderr, "Error querying API version or information\n");
        exit(-1);
    }
    printf("API version: %i\n", api);
    printf("Information string: %s\n", info);
}

/* Generic description structure */
struct Desc
{
    const char *name;
    uint32_t id;
    uint32_t aux; /* optional extra parameter... */
};

/**************** Video surface ************/
Desc chroma_types[] = {
{"420", VDP_CHROMA_TYPE_420},
{"422", VDP_CHROMA_TYPE_422},
{"444", VDP_CHROMA_TYPE_444},
};
const size_t chroma_type_count = sizeof(chroma_types)/sizeof(Desc);

Desc ycbcr_types[] = {
{"NV12", VDP_YCBCR_FORMAT_NV12},
{"YV12", VDP_YCBCR_FORMAT_YV12},
{"UYVY", VDP_YCBCR_FORMAT_UYVY},
{"YUYV", VDP_YCBCR_FORMAT_YUYV},
{"Y8U8V8A8", VDP_YCBCR_FORMAT_Y8U8V8A8},
{"V8U8Y8A8", VDP_YCBCR_FORMAT_V8U8Y8A8},
};
const size_t ycbcr_type_count = sizeof(ycbcr_types)/sizeof(Desc);

Desc rgb_types[] = {
{"B8G8R8A8", VDP_RGBA_FORMAT_B8G8R8A8},
{"R8G8B8A8", VDP_RGBA_FORMAT_R8G8B8A8},
{"R10G10B10A2", VDP_RGBA_FORMAT_R10G10B10A2},
{"B10G10R10A2", VDP_RGBA_FORMAT_B10G10R10A2},
{"A8", VDP_RGBA_FORMAT_A8},
};
const size_t rgb_type_count = sizeof(rgb_types)/sizeof(Desc);

Desc indexed_types[] = {
{"A4I4", VDP_INDEXED_FORMAT_A4I4},
{"I4A4", VDP_INDEXED_FORMAT_I4A4},
{"A8I8", VDP_INDEXED_FORMAT_A8I8},
{"I8A8", VDP_INDEXED_FORMAT_I8A8},
};
const size_t indexed_type_count = sizeof(indexed_types)/sizeof(Desc);

Desc color_table_formats[] = {
{"B8G8R8X8", VDP_COLOR_TABLE_FORMAT_B8G8R8X8},
};
const size_t color_table_format_count = sizeof(color_table_formats)/sizeof(Desc);


void queryVideoSurface(VDPDeviceImpl *device)
{
    VdpStatus rv;
    printf("\nVideo surface:\n\n");
    printf("name   width height types\n");
    printf("-------------------------------------------\n");
    for(int x=0; x<chroma_type_count; ++x)
    {
        VdpBool is_supported;
        uint32_t max_width, max_height;

        rv = device->VideoSurfaceQueryCapabilities(device->device, chroma_types[x].id,
            &is_supported, &max_width, &max_height);
        if(rv == VDP_STATUS_OK && is_supported)
        {
            printf("%-6s %5i %5i  ", chroma_types[x].name,
                max_width, max_height);
            /* Find out supported formats */
            for(int y=0; y<ycbcr_type_count; ++y)
            {
                rv = device->VideoSurfaceQueryGetPutBitsYCbCrCapabilities(
                    device->device, chroma_types[x].id, ycbcr_types[y].id,
                    &is_supported);
                if(rv == VDP_STATUS_OK && is_supported)
                {
                    printf("%s ", ycbcr_types[y].name);
                }
            }
            printf("\n");
        }
    }
}

/***************** Output surface ****************/
void queryOutputSurface(VDPDeviceImpl *device)
{
    VdpStatus rv;
    printf("\nOutput surface:\n\n");
    printf("name              width height nat types\n");
    printf("----------------------------------------------------\n");
    for(int x=0; x<rgb_type_count; ++x)
    {
        VdpBool is_supported, native;
        uint32_t max_width, max_height;

        rv = device->OutputSurfaceQueryCapabilities(device->device, rgb_types[x].id,
            &is_supported, &max_width, &max_height);
        device->OutputSurfaceQueryGetPutBitsNativeCapabilities(device->device, rgb_types[x].id,
            &native);
        if(rv == VDP_STATUS_OK && is_supported)
        {
            printf("%-16s %5i %5i    %c  ", rgb_types[x].name,
                max_width, max_height, native?'y':'-');
            /* Find out supported formats */
            for(int y=0; y<ycbcr_type_count; ++y)
            {
                rv = device->OutputSurfaceQueryPutBitsYCbCrCapabilities(
                    device->device, rgb_types[x].id, ycbcr_types[y].id,
                    &is_supported);
                if(rv == VDP_STATUS_OK && is_supported)
                {
                    printf("%s ", ycbcr_types[y].name);
                }
            }
            printf("\n");
        }
    }
    // OutputSurfaceQueryPutBitsIndexedCapabilities
    //   rgba, idx, colortable -> supported
}

/***************** Bitmap surface ****************/
void queryBitmapSurface(VDPDeviceImpl *device)
{
    VdpStatus rv;
    printf("\nBitmap surface:\n\n");
    printf("name              width height\n");
    printf("------------------------------\n");
    for(int x=0; x<rgb_type_count; ++x)
    {
        VdpBool is_supported;
        uint32_t max_width, max_height;

        rv = device->BitmapSurfaceQueryCapabilities(device->device, rgb_types[x].id,
            &is_supported, &max_width, &max_height);
        if(rv == VDP_STATUS_OK && is_supported)
        {
            printf("%-16s %5i %5i\n", rgb_types[x].name,
                max_width, max_height);
        }
    }
}

/******************* Video mixer ****************/

/* Type for value ranges */
enum DataType
{
    DT_NONE,
    DT_INT,
    DT_UINT,
    DT_FLOAT
};

Desc mixer_features[] = {
{"DEINTERLACE_TEMPORAL",VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL},
{"DEINTERLACE_TEMPORAL_SPATIAL",VDP_VIDEO_MIXER_FEATURE_DEINTERLACE_TEMPORAL_SPATIAL},
{"INVERSE_TELECINE",VDP_VIDEO_MIXER_FEATURE_INVERSE_TELECINE},
{"NOISE_REDUCTION",VDP_VIDEO_MIXER_FEATURE_NOISE_REDUCTION},
{"SHARPNESS",VDP_VIDEO_MIXER_FEATURE_SHARPNESS},
{"LUMA_KEY",VDP_VIDEO_MIXER_FEATURE_LUMA_KEY},
{"HIGH QUALITY SCALING - L1", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L1},
{"HIGH QUALITY SCALING - L2", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L2},
{"HIGH QUALITY SCALING - L3", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L3},
{"HIGH QUALITY SCALING - L4", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L4},
{"HIGH QUALITY SCALING - L5", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L5},
{"HIGH QUALITY SCALING - L6", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L6},
{"HIGH QUALITY SCALING - L7", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L7},
{"HIGH QUALITY SCALING - L8", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L8},
{"HIGH QUALITY SCALING - L9", VDP_VIDEO_MIXER_FEATURE_HIGH_QUALITY_SCALING_L9},
};
const size_t mixer_features_count = sizeof(mixer_features)/sizeof(Desc);

Desc mixer_parameters[] = {
{"VIDEO_SURFACE_WIDTH",VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,DT_UINT},
{"VIDEO_SURFACE_HEIGHT",VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT,DT_UINT},
{"CHROMA_TYPE",VDP_VIDEO_MIXER_PARAMETER_CHROMA_TYPE,DT_NONE},
{"LAYERS",VDP_VIDEO_MIXER_PARAMETER_LAYERS,DT_UINT},
};
const size_t mixer_parameters_count = sizeof(mixer_parameters)/sizeof(Desc);

Desc mixer_attributes[] = {
{"BACKGROUND_COLOR",VDP_VIDEO_MIXER_ATTRIBUTE_BACKGROUND_COLOR,DT_NONE},
{"CSC_MATRIX",VDP_VIDEO_MIXER_ATTRIBUTE_CSC_MATRIX,DT_NONE},
{"NOISE_REDUCTION_LEVEL",VDP_VIDEO_MIXER_ATTRIBUTE_NOISE_REDUCTION_LEVEL,DT_FLOAT},
{"SHARPNESS_LEVEL",VDP_VIDEO_MIXER_ATTRIBUTE_SHARPNESS_LEVEL,DT_FLOAT},
{"LUMA_KEY_MIN_LUMA",VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MIN_LUMA,DT_NONE},
{"LUMA_KEY_MAX_LUMA",VDP_VIDEO_MIXER_ATTRIBUTE_LUMA_KEY_MAX_LUMA,DT_NONE},
};
const size_t mixer_attributes_count = sizeof(mixer_attributes)/sizeof(Desc);

void display_range(uint32_t aux, uint32_t minval, uint32_t maxval)
{
    switch(aux)
    {
        case DT_INT: printf("%8i %8i", minval, maxval); break;
        case DT_UINT: printf("%8u %8u", minval, maxval); break;
        case DT_FLOAT: printf("%8.2f %8.2f", *((float*)&minval), *((float*)&maxval)); break;
        default: /* Ignore value which we don't know how to display */;
    }
}

void queryVideoMixer(VDPDeviceImpl *device)
{
    VdpStatus rv;
    printf("\nVideo mixer:\n\n");
   // Features
    printf("feature name                    sup\n");
    printf("------------------------------------\n");
    for(int x=0; x<mixer_features_count; ++x)
    {
        VdpBool is_supported;

        rv = device->VideoMixerQueryFeatureSupport(device->device, mixer_features[x].id,
            &is_supported);
        is_supported = (rv == VDP_STATUS_OK && is_supported);
        printf("%-32s %c\n", mixer_features[x].name,
            is_supported?'y':'-');

    }
    printf("\n");
    // Parameters (+range)
    printf("parameter name                  sup      min      max\n");
    printf("-----------------------------------------------------\n");
    for(int x=0; x<mixer_parameters_count; ++x)
    {
        VdpBool is_supported;

        rv = device->VideoMixerQueryParameterSupport(device->device, mixer_parameters[x].id,
            &is_supported);
        is_supported = (rv == VDP_STATUS_OK && is_supported);
        printf("%-32s %c  ", mixer_parameters[x].name,
            is_supported?'y':'-');
        /* VDPAU spec does not allow range query for DT_NONE types */
        if(is_supported && mixer_parameters[x].aux != DT_NONE)
        {
            uint32_t minval, maxval;
            rv = device->VideoMixerQueryParameterValueRange(device->device, mixer_parameters[x].id,
                (void*)&minval, (void*)&maxval);
            if(rv == VDP_STATUS_OK)
                display_range(mixer_parameters[x].aux, minval, maxval);
        }
        printf("\n");
    }
    printf("\n");

    // Attributes (+range)
    printf("attribute name                  sup      min      max\n");
    printf("-----------------------------------------------------\n");
    for(int x=0; x<mixer_attributes_count; ++x)
    {
        VdpBool is_supported;

        rv = device->VideoMixerQueryAttributeSupport(device->device, mixer_attributes[x].id,
            &is_supported);
        is_supported = (rv == VDP_STATUS_OK && is_supported);
        printf("%-32s %c  ", mixer_attributes[x].name,
            is_supported?'y':'-');
        /* VDPAU spec does not allow range query for DT_NONE types */
        if(is_supported && mixer_attributes[x].aux != DT_NONE)
        {
            uint32_t minval, maxval;
            rv = device->VideoMixerQueryAttributeValueRange(device->device, mixer_parameters[x].id,
                (void*)&minval, (void*)&maxval);
            if(rv == VDP_STATUS_OK)
                display_range(mixer_attributes[x].aux, minval, maxval);
        }
        printf("\n");
    }
    printf("\n");
}

/******************* Decoder ****************/

Desc decoder_profiles[] = {
{"MPEG1",              VDP_DECODER_PROFILE_MPEG1},
{"MPEG2_SIMPLE",       VDP_DECODER_PROFILE_MPEG2_SIMPLE},
{"MPEG2_MAIN",         VDP_DECODER_PROFILE_MPEG2_MAIN},
{"H264_BASELINE",      VDP_DECODER_PROFILE_H264_BASELINE},
{"H264_MAIN",          VDP_DECODER_PROFILE_H264_MAIN},
{"H264_HIGH",          VDP_DECODER_PROFILE_H264_HIGH},
{"VC1_SIMPLE",         VDP_DECODER_PROFILE_VC1_SIMPLE},
{"VC1_MAIN",           VDP_DECODER_PROFILE_VC1_MAIN},
{"VC1_ADVANCED",       VDP_DECODER_PROFILE_VC1_ADVANCED},
{"MPEG4_PART2_SP",     VDP_DECODER_PROFILE_MPEG4_PART2_SP},
{"MPEG4_PART2_ASP",    VDP_DECODER_PROFILE_MPEG4_PART2_ASP},
{"DIVX4_QMOBILE",      VDP_DECODER_PROFILE_DIVX4_QMOBILE},
{"DIVX4_MOBILE",       VDP_DECODER_PROFILE_DIVX4_MOBILE},
{"DIVX4_HOME_THEATER", VDP_DECODER_PROFILE_DIVX4_HOME_THEATER},
{"DIVX4_HD_1080P",     VDP_DECODER_PROFILE_DIVX4_HD_1080P},
{"DIVX5_QMOBILE",      VDP_DECODER_PROFILE_DIVX5_QMOBILE},
{"DIVX5_MOBILE",       VDP_DECODER_PROFILE_DIVX5_MOBILE},
{"DIVX5_HOME_THEATER", VDP_DECODER_PROFILE_DIVX5_HOME_THEATER},
{"DIVX5_HD_1080P",     VDP_DECODER_PROFILE_DIVX5_HD_1080P},
};
const size_t decoder_profile_count = sizeof(decoder_profiles)/sizeof(Desc);

void queryDecoderCaps(VDPDeviceImpl *device)
{
    VdpStatus rv;
    printf("\nDecoder capabilities:\n\n");
    printf("name               level macbs width height\n");
    printf("-------------------------------------------\n");
    for(int x=0; x<decoder_profile_count; ++x)
    {
        VdpBool is_supported;
        uint32_t max_level, max_macroblocks, max_width, max_height;

        rv = device->DecoderQueryCapabilities(device->device, decoder_profiles[x].id,
            &is_supported, &max_level, &max_macroblocks, &max_width, &max_height);
        if(rv == VDP_STATUS_OK && is_supported)
        {
            printf("%-20s %2i %5i %5i %5i\n", decoder_profiles[x].name,
                max_level, max_macroblocks, max_width, max_height);
        }
    }
}


int main(int argc, char **argv)
{
    /* Create an X Display */
    Display *display;
    int screen;
    char *display_name = XDisplayName(NULL);
    if ((display=XOpenDisplay(display_name)) == NULL)
    {
        fprintf(stderr,"vdpauinfo: cannot connect to X server %s\n",
              XDisplayName(display_name));
        exit(-1);
    }
    screen = DefaultScreen(display);
    printf("display: %s   screen: %i\n", display_name, screen);

    /* Create device */
    VdpDevice device;
    VdpGetProcAddress *get_proc_address;
    VdpStatus rv;
    rv = vdp_device_create_x11(display, screen, &device, &get_proc_address);
    if(rv != VDP_STATUS_OK)
    {
        fprintf(stderr, "Error creating VDPAU device: %i\n", rv); /* cannot use GetErrorString here */
        exit(-1);
    }

    VDPDeviceImpl *impl = new VDPDeviceImpl(device, get_proc_address);

    queryBaseInfo(impl);
    queryVideoSurface(impl);
    queryDecoderCaps(impl);
    queryOutputSurface(impl);
    queryBitmapSurface(impl);
    queryVideoMixer(impl);

    printf("\n");
}
