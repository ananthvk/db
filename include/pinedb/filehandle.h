#ifndef PINEDB_FILEHANDLE_H
#define PINEDB_FILEHANDLE_H
#ifdef _WIN32
#    include <fcntl.h>
#    include <io.h>
#    include <sys/stat.h>
#    include <sys/types.h>
#else
#    include <fcntl.h>
#    include <sys/types.h>
#    include <unistd.h>
#endif
#include <string>

class FileHandle
{
  private:
    int fd = -1;

  public:
    bool open(const std::string &file_path, int flags, int mode = 0)
    {
#ifdef _WIN32
        fd = _open(file_path.c_str(), flags, mode);
#else
        fd = ::open(file_path.c_str(), flags, mode);
#endif
        return fd != -1;
    }

    ~FileHandle()
    {
        if (fd != -1)
        {
            close();
        }
    }

    int get() const { return fd; }

    off_t seek(off_t offset, int whence)
    {
#ifdef _WIN32
        return _lseek(fd, offset, whence);
#else
        return lseek(fd, offset, whence);
#endif
    }

    ssize_t read(void *buffer, size_t size)
    {
#ifdef _WIN32
        return _read(fd, buffer, size);
#else
        return ::read(fd, buffer, size);
#endif
    }

    ssize_t write(const void *buffer, size_t size)
    {
#ifdef _WIN32
        return _write(fd, buffer, size);
#else
        return ::write(fd, buffer, size);
#endif
    }

    bool close()
    {
        bool status;
#ifdef _WIN32
        status = _close(fd) != -1;
#else
        status = ::close(fd) != -1;
#endif
        fd = -1;
        return status;
    }

    // Returns true if the file handle is closed
    bool closed() { return fd == -1; }
};
#endif // PINEDB_FILEHANDLE_H