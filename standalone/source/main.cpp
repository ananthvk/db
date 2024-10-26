#include <pinedb/config.h>
#include <pinedb/pinedb.h>
#include <pinedb/storage.h>
#include <pinedb/version.h>

#include <iostream>
#include <string>
#include <unordered_map>

auto main(int argc, char** argv) -> int {
  pinedb::DiskStorageBackend b("db_file_1", pinedb::config::PAGE_SIZE);
  // b.create_new_page();
  uint8_t buf[pinedb::config::PAGE_SIZE];
  b.read_page(8, buf);

  return 0;
}
