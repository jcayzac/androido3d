//
// Use this file to customize how the the o3d library is built.
// Alternatively, you can defined the following macros in your
// project's settings.
//


// Uncomment this line to set a custom minimum log level
// Acceptable values are those found in the LogLevel enum in base/cross/log.h
// Defaults to INFO in debug builds and to WARNING in release builds.
// #define O3D_MINIMUM_LOG_LEVEL WARNING


// Uncomment this line to set a custom tag to use when logging
// Defaults to "O3D"
// #define O3D_LOG_TAG "O3D"


// Uncomment this line to prevent O3D from specializing
// std::equal_to and std::not_equal_to for floats.
// Without this macro, O3D will provide fuzzy implementations
// of these two function objects, to compare floats in a safer
// way than just using the == and != C operators, but it may cause
// problems if you use O3D with another library that does the same.
// #define O3D_NO_FUZZY_STD_EQUALITY_SPECIALIZATION_FOR_FLOATS


// MacOS-specific: uncomment to use AGL double buffer (aglSwapBuffer())
// #define O3D_USE_AGL_DOUBLE_BUFFER


// Uncomment one of the following lines to build O3D for OpenGL or
// for OpenGL ES 2.x
// #define O3D_RENDERER_GL
// #define O3D_RENDERER_GLES2

// Uncomment this line to turn on client profiling
// #define O3D_PROFILE_CLIENT

// Uncomment to prevent OpenGL ES 2.0 renderer to use FBO
// FIXME: Is this ever needed at all??
// #define O3D_DISABLE_FBO

// #define O3D_CORE_CROSS_GPU2D_PATH_CACHE_DEBUG_INTERIOR_EDGES
// #define O3D_CORE_CROSS_GPU2D_PATH_PROCESSOR_DEBUG_ORIENTATION
// GL_ERROR_DEBUGGING
// GLES2_BACKEND_DESKTOP_GL
// GLES2_BACKEND_NATIVE_GLES2
