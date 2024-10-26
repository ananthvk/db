#ifndef PINEDB_CONFIG_H
#define PINEDB_CONFIG_H
#include "common.h"
#include "config.h"

#include <stddef.h>
#include <stdint.h>

namespace pinedb
{
    namespace config
    {
        constexpr page_size_type PAGE_SIZE = 1 << 12; // Default page size of 4KiB
        constexpr int NUMBER_OF_FRAMES = 1 << 15; // Number of frames to hold in memory: 32768, i.e.
                                                  // max size of buffer pool is 128MB

    }; // namespace config
};     // namespace pinedb
#endif // PIENDB_CONFIG_H