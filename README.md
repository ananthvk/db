[![Actions Status](https://github.com/ananthvk/pinedb/workflows/MacOS/badge.svg)](https://github.com/ananthvk/pinedb/actions)
[![Actions Status](https://github.com/ananthvk/pinedb/workflows/Windows/badge.svg)](https://github.com/ananthvk/pinedb/actions)
[![Actions Status](https://github.com/ananthvk/pinedb/workflows/Ubuntu/badge.svg)](https://github.com/ananthvk/pinedb/actions)
[![Actions Status](https://github.com/ananthvk/pinedb/workflows/Style/badge.svg)](https://github.com/ananthvk/pinedb/actions)
[![Actions Status](https://github.com/ananthvk/pinedb/workflows/Install/badge.svg)](https://github.com/ananthvk/pinedb/actions)
[![codecov](https://codecov.io/gh/ananthvk/pinedb/branch/master/graph/badge.svg)](https://codecov.io/gh/ananthvk/pinedb)

<p align="center">
  <img src="https://repository-images.githubusercontent.com/874815925/09c2ebf4-a127-47e2-9e48-7a6044c86923" height="200" width="auto" />
</p>

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

## Page format
All pages are of size `4096` bytes or `4KB`, integer values are always stored in little-endian format.

A page can be of three types - data page (which contains the data), internal B+ tree page, or B+ tree leaf page.

### Common Page header for all B+ Tree pages

| Offset | Size (in bytes) | Description                              |
|--------|-----------------|------------------------------------------|
| 0      | 1               | Page type                                |
| 1      | 8               | Reserved for future use                  |
| 9      | 4               | Page id of the current page              |
| 13     | 4               | Page id of parent page (current, if root)|
| 17     | 4               | Number of keys/values used in the page   |
| 21     | 4               | Max number of keys/values in the page    |
| 25     | 1               | Key type                                 |

Where key type describes the data type of the key for the B+ Tree page, which can be one of the following

| Key Type | Data type   | 
|----------|-------------|
| `'b'`    | uint8_t     |
| `'B'`    | int8_t      |
| `'s'`    | uint16_t    |
| `'S'`    | int16_t     |
| `'i'`    | uint32_t    |
| `'I'`    | int32_t     |
| `'l'`    | uint64_t    |
| `'L'`    | int64_t     |
| `'f'`    | float       |
| `'d'`    | double      |
| `'c'`    | string      |

Key can have a maximum size of `64 bits` or `8 bytes`, strings require special handling to work.

Total common header size is `26 bytes`

### Page format for B+ Tree internal page
Page type is set to `0x1`

A B+ internal page stores `n` keys and `n+1` child links, that are page ids, `n` keys are stored first, followed by the child links.

```
| Key 1 | Key 2 | ..... | Key n | [Page link 1] | [Page link 2] | [Page link n+1] |
```

### Page format for B+ Tree leaf page
Page type is set to `0x2`

| Offset | Size (in bytes) | Description                              |
|--------|-----------------|------------------------------------------|
| 26     | 4               | Page id of next page(for range queries)  |

The leaf node contains `n` keys, followed by `n` values, similar to an internal node. Values are always `64 bit` in length and represent a row id.

```
| Key 1 | Key 2 | ..... | Key n | [Value 1] | [Value 2] | [Value n] |
```

### Page format for data pages

TODO

## Tasks

- [x] Implement page representation class
- [x] Storage backend (Disk)
- [x] Memory storage backend
- [ ] Free page management (free list/bitmap) for disk storage
- [ ] Metadata reader/writer implementation
- [ ] Database lockfile
- [x] Buffer pool manager
- [ ] Extendible hash table
- [x] Cache replacer
- [ ] Simple KV store supporting only small strings 
- [ ] Support for integers, floats
- [ ] Arrays
- [ ] BTrees and indices
- [ ] Simple query parsers
- [ ] REPL
- [ ] Socket server for serving requests


## Features

## Usage

### Build and run the standalone target

Use the following command to build and run the executable target.

```bash
cmake -S standalone -B build/standalone
cmake --build build/standalone
./build/standalone/PineDB --help
```

### Build for development
```bash
cmake -S standalone -B build -DUSE_SANITIZER='Address;Undefined' -DUSE_STATIC_ANALYZER=clang-tidy

```

### Build and run test suite

Use the following commands from the project's root directory to run the test suite.

```bash
cmake -S test -B build/test
cmake --build build/test
CTEST_OUTPUT_ON_FAILURE=1 cmake --build build/test --target test

# or simply call the executable: 
./build/test/PineDBTests
```

To collect code coverage information, run CMake with the `-DENABLE_TEST_COVERAGE=1` option.

### Run clang-format

Use the following commands from the project's root directory to check and fix C++ and CMake source style.
This requires _clang-format_, _cmake-format_ and _pyyaml_ to be installed on the current system.

```bash
cmake -S test -B build/test

# view changes
cmake --build build/test --target format

# apply changes
cmake --build build/test --target fix-format
```

See [Format.cmake](https://github.com/ananthvk/Format.cmake) for details.
These dependencies can be easily installed using pip.

```bash
pip install clang-format==14.0.6 cmake_format==0.6.11 pyyaml
```

### Build the documentation

The documentation is automatically built and [published](https://ananthvk.github.io/pinedb) whenever a [GitHub Release](https://help.github.com/en/github/administering-a-repository/managing-releases-in-a-repository) is created.
To manually build documentation, call the following command.

```bash
cmake -S documentation -B build/doc
cmake --build build/doc --target GenerateDocs
# view the docs
open build/doc/doxygen/html/index.html
```

To build the documentation locally, you will need Doxygen, jinja2 and Pygments installed on your system.

### Build everything at once

The project also includes an `all` directory that allows building all targets at the same time.
This is useful during development, as it exposes all subprojects to your IDE and avoids redundant builds of the library.

```bash
cmake -S all -B build
cmake --build build

# run tests
./build/test/PineDBTests
# format code
cmake --build build --target fix-format
# run standalone
./build/standalone/PineDB --help
# build docs
cmake --build build --target GenerateDocs
```

### Additional tools

The test and standalone subprojects include the [tools.cmake](cmake/tools.cmake) file which is used to import additional tools on-demand through CMake configuration arguments.
The following are currently supported.

#### Sanitizers

Sanitizers can be enabled by configuring CMake with `-DUSE_SANITIZER=<Address | Memory | MemoryWithOrigins | Undefined | Thread | Leak | 'Address;Undefined'>`.

#### Static Analyzers

Static Analyzers can be enabled by setting `-DUSE_STATIC_ANALYZER=<clang-tidy | iwyu | cppcheck>`, or a combination of those in quotation marks, separated by semicolons.
By default, analyzers will automatically find configuration files such as `.clang-format`.
Additional arguments can be passed to the analyzers by setting the `CLANG_TIDY_ARGS`, `IWYU_ARGS` or `CPPCHECK_ARGS` variables.

#### Ccache

Ccache can be enabled by configuring with `-DUSE_CCACHE=<ON | OFF>`.

## FAQ

> Question

Answer
