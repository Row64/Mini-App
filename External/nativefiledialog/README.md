# Native File Dialog CMake

In this fork I replaced Premake and other build system with CMake

For more information look at the original [README.md](https://github.com/mlabbe/nativefiledialog/blob/5cfe5002eb0fac1e49777a17dec70134147931e2/README.md)

## Example

An example of CMakeLists.txt

```cmake
add_subdirectory(libs/nativefiledialog)
add_executable(mytarget main.cpp)
target_link_libraries(mytarget nativefiledialog)
```

and main.cpp

```C
#include <nfd.h>
#include <stdio.h>
#include <stdlib.h>

int main( void )
{
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( NULL, NULL, &outPath );
        
    if ( result == NFD_OKAY ) {
        puts("Success!");
        puts(outPath);
        free(outPath);
    }
    else if ( result == NFD_CANCEL ) {
        puts("User pressed cancel.");
    }
    else {
        printf("Error: %s\n", NFD_GetError() );
    }

    return 0;
}
```
