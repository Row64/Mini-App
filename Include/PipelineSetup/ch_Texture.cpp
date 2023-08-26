#ifndef CH_TEXTURE_CPP
#define CH_TEXTURE_CPP

#include "ch_Texture.h"
#include "ch_Tools.h"

#include "stb_image.h"
#include <iostream>

namespace AppCore {

    std::string ch_Texture::fallbackTexture = "";

    char* ch_Texture::GetImageData( int requested_components, int *width, int *height, int *components, int *data_size ) {

        if ( path == "" ) { return {}; }

        FileEngine imgfile(false);
        if ( !imgfile.OpenFile( path ) ) { 
            // failed to get the image. Open the white pixel instead.
            std::cout << "Could not open image file " << path <<". ";
            if ( !fallbackTexture.empty() ){
                std::cout << "Falling back on placeholder texture.\n";
                if ( !imgfile.OpenFile( fallbackTexture ) ){
                    throw std::runtime_error( "Could not open fallback texture image file!" );
                }
            }
            else {
                throw std::runtime_error( "Could not open texture image file! (no fallback specififed)" );
            }
        }

        int tmp_width = 0, tmp_height = 0, tmp_components = 0;
        

        unsigned char *image_data;

        if ( bitPerChannel == 32 ) {
            // Load HDR
            std::cout << "Loading HDR Image: " << path << std::endl;
            image_data = (unsigned char * ) stbi_loadf_from_memory(imgfile.Content, (int) imgfile.FileSize(), &tmp_width, &tmp_height,  &tmp_components, requested_components);
        }
        else if(bitPerChannel == 16){
            // Load 16-bit image
            image_data = (unsigned char * ) stbi_load_16_from_memory(imgfile.Content, (int) imgfile.FileSize(), &tmp_width, &tmp_height, &tmp_components, requested_components);
        }   
        else{
            // Load 8-bit image
            image_data = stbi_load_from_memory( imgfile.Content, (int) imgfile.FileSize(), &tmp_width, &tmp_height, &tmp_components, requested_components );
             
        }

        if( (image_data == nullptr) ||
            (tmp_width <= 0) ||
            (tmp_height <= 0) ||
            (tmp_components <= 0) ) {
            throw std::runtime_error( "Could not get image data!" );
        }

        int size = (tmp_width) * (tmp_height) * (requested_components <= 0 ? tmp_components : requested_components);
        if( data_size ) {
            if ( bitPerChannel == 32 ) {
                *data_size = size * 4;
            } else if ( bitPerChannel == 16 ) {
                *data_size = size * 2;
            } else {
                *data_size = size;
            }
        }
        if( width ) {
            *width = tmp_width;
        }
        if( height ) {
            *height = tmp_height;
        }
        if( components ) {
            *components = tmp_components;
        }

        return reinterpret_cast<char*>(image_data);
    }

    bool ch_Texture::operator==(const ch_Texture& rhs) {
        if( path != rhs.path ) { return false; }
        if( bitPerChannel != rhs.bitPerChannel ) { return false; }
        return true;
    }

    bool ch_Texture::operator!=(const ch_Texture& rhs) {
        if( path != rhs.path ) { return true; }
        if( bitPerChannel != rhs.bitPerChannel ) { return true; }
        return false;
    }

} // end namespace AppCore

#endif /* CH_TEXTURE_CPP */
