#include <iostream>
#include <pinedb/config.h>
#include <pinedb/pinedb.h>
#include <pinedb/storage.h>
#include <pinedb/version.h>
#include <string>
#include <unordered_map>

auto main(int argc, char **argv) -> int
{
    pinedb::DiskStorageBackend b("db_file_1", pinedb::config::PAGE_SIZE);
    uint8_t buf[pinedb::config::PAGE_SIZE];
    buf[0] = 0xA;
    buf[1] = 0xB;
    buf[2] = 0xC;
    buf[3] = 0xD;
    b.write_page(3, buf);
    b.read_page(3, buf);

    return 0;
}
