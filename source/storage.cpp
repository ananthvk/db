#include <fcntl.h>
#include <memory>
#include <pinedb/storage.h>
#include <spdlog/spdlog.h>
#include <unistd.h>
using namespace pinedb;

DiskStorageBackend::DiskStorageBackend(const std::string &file_path, page_size_type page_sz)
    : file_path(file_path), page_sz(page_sz), current_page_id_counter(0)
{
    fd = open(file_path.c_str(), O_CREAT | O_DIRECT | O_RDWR | O_SYNC, 0644);
    if (fd == -1)
    {
        spdlog::error("Could not open db file {}: {}", file_path, strerror(errno));
        exit(1);
    }
}

page_id_type DiskStorageBackend::create_new_page()
{
    spdlog::info("Created new page");
    if (lseek(fd, 0, SEEK_END) == -1)
    {
        spdlog::error("Could not create new page lseek: {}", strerror(errno));
        exit(1);
    }
    std::unique_ptr<char[]> buf(new char[page_sz]);
    memset(buf.get(), 0, page_sz);
    if (write(fd, buf.get(), page_sz) == -1)
    {
        spdlog::error("Could not create new page write: {}", strerror(errno));
        exit(1);
    }
    return 0;
}

bool DiskStorageBackend::read_page(page_id_type page_id, uint8_t *buffer)
{
    // Pages are stored contiguously in the file, so the n th page is
    // at location n * page_sz
    auto offset = page_id * page_sz;
    if (lseek(fd, offset, SEEK_SET) == static_cast<off_t>(-1))
        return false;
    ssize_t bytes_read;
    if ((bytes_read = read(fd, buffer, page_sz) == -1))
    {
        return false;
    }
    if (bytes_read != page_sz)
    {
        return false;
    }
    return true;
}

bool DiskStorageBackend::write_page(page_id_type page_id, uint8_t *buffer) { return true; }

bool DiskStorageBackend::delete_page(page_id_type page_id) { return true; }

page_size_type DiskStorageBackend::page_size() { return page_sz; }

void DiskStorageBackend::close() { ::close(fd); }

DiskStorageBackend::~DiskStorageBackend() { close(); }