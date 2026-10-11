// Compiles the implementation of the stb single-header libraries.
// Kept in its own translation unit so it is defined exactly once and so
// that third-party warnings can be silenced without affecting our code.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
