# PineDB
The goal of this project is to build an efficient KV (Key-Value) store with persistence in C++, with efficient retrieval using B Trees.

Further goals include building a Query manager, concurrency control and supporting multithreading(the current implementation is single threaded)

## Database file structure
```
/dbname
    pagedata
    metadata
    freemap
    lock
```
A database is represented on the disk as a collection of files, with `pagedata` containing the actual data, it is of size `PAGE_SIZE * number of pages`

`metadata` consists of information about the database such as the page size, version, number of pages and other details

`freemap` is a bitmap which represents free pages in the `pagedata` file

`lock` is used so that only one db process can use the database files at one time

## Classes

`StorageBackend` is an abstract class which provides persistence for the pages, `DiskStorageBackend` is a concrete implementation which writes the pages to a file.

`MemoryStorageBackend` keeps the pages in memory using a map of vectors, and is useful for testing.

`BufferPool` is an interface to access the pages, it caches the pages and handles reading and writing them

## Tasks

- [x] Implement page representation class
- [x] Storage backend (Disk)
- [x] Memory storage backend
- [ ] Free page management (free list/bitmap) for disk storage
- [ ] Metadata reader/writer implementation
- [ ] Database lockfile
- [ ] Buffer pool manager
- [ ] Extendible hash table
- [ ] Cache replacer
- [ ] Simple KV store supporting only small strings 
- [ ] Support for integers, floats
- [ ] Arrays
- [ ] BTrees and indices
- [ ] Simple query parsers
- [ ] REPL
- [ ] Socket server for serving requests
