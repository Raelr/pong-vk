#ifndef PONG_VK_CORE_H
#define PONG_VK_CORE_H

#include "../logger.h"

namespace Renderer {

    enum Status {
        FAILURE = 0,
        INITIALIZATION_FAILURE = 1,
        SUCCESS = 2,
        SKIPPED_FRAME = 3
    };

}


#endif //PONG_VK_CORE_H
