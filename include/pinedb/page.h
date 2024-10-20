#ifndef PINEDB_PAGE_H
#define PINEDB_PAGE_H

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace pinedb {
  class Page {
  private:
    uint8_t *buffer;
    int32_t page_id;
    std::shared_mutex rw_latch;
    std::atomic<int> pin_count;
    std::atomic<bool> is_dirty;

  public:
    Page();
  };
}  // namespace pinedb
#endif  // PINEDB_PAGE_H