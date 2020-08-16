#include "renderer2D.h"

namespace Renderer2D {

    static Buffers::Vertex quadVertices[] = {
            // Positions        // Colors
            { {-0.5f, -0.5f},   {1.0f, 1.0f, 1.0f} },
            { {0.5f, -0.5f},    {1.0f, 1.0f, 1.0f} },
            { {0.5f, 0.5f},     {1.0f, 1.0f, 1.0f} },
            { {-0.5f, 0.5f},    {1.0f, 1.0f, 1.0f} }
    };

    static uint32_t quadIndices[] = {
        0, 1, 2, 2, 3, 0
    };

}