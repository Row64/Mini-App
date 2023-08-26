// IMGUI bare bones, stripped down to run Row64 UI


#pragma once

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>


// ----------------------------- Row64 ----------------------------- //

// Use 32-bit indices
#define ImDrawIdx unsigned int

#define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT   \
struct ImDrawVert                               \
{                                               \
    ImVec2      pos;                            \
    ImVec2      uv;                             \
    ImU32       col;                            \
    int         texID = -1;                     \
};

// Extend ImVec2 and ImVec4 with glm::vec2 and glm::vec4
#define IM_VEC2_CLASS_EXTRA                                                     \
        ImVec2(const glm::vec2& f) { x = f.x; y = f.y; }                        \
        operator glm::vec2() const { return glm::vec2(x,y); }

#define IM_VEC4_CLASS_EXTRA                                                     \
        ImVec4(const glm::vec4& f) { x = f.x; y = f.y; z = f.z; w = f.w; }      \
        operator glm::vec4() const { return glm::vec4(x,y,z,w); }


#define IMGUI_DISABLE_DEMO_WINDOWS
#define IMGUI_DISABLE_METRICS_WINDOW
#define IMGUI_DISABLE_WIN32_FUNCTIONS


// Need the packing but maybe in the future we can turn it into a cache...
// #define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
// #define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

