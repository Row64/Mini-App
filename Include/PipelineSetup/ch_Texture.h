#ifndef CH_TEXTURE_H
#define CH_TEXTURE_H

#include <string>


namespace AppCore {

    // ************************************************************ //
    // ch_Texture                                                   //
    //                                                              //
    // Struct describing texture metadata                           //
    // ************************************************************ //
    struct ch_Texture {
        static std::string          fallbackTexture;
        std::string                 path = "";
        int                         bitPerChannel = 8;
        bool                        skip_load = false;

        char*                       GetImageData( int requested_components, int *width, int *height, int *components, int *data_size );

        // Overload the == and != operators so we can check for equivalence between two ch_Texture objects
        bool operator==(const ch_Texture& rhs);
        bool operator!=(const ch_Texture& rhs);
        
    };

    

} // end namespace AppCore

#endif // CH_TEXTURE_H