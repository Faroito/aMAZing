
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Scene.hpp"

/*
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF);

_CrtDumpMemoryLeaks();
*/

int main(int argc, const char ** argv) {
    stbi_set_flip_vertically_on_load(true);
    scene::Scene app;

    app.start();
    return 0;
}
