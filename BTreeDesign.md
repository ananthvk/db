# TODO: Change later
### Common Page header for all B+ Tree pages

| Offset | Size (in bytes) | Description                              |
|--------|-----------------|------------------------------------------|
| 0      | 1               | Page type                                |
| 1      | 8               | Reserved for future use                  |
| 9      | 4               | Page id of the current page              |
| 13     | 4               | Reserved                                 |
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
| 13     | 4               | Page id of next page(for range queries)  |

The reserved field is used as link to next page in leaf nodes

The leaf node contains `n` keys, followed by `n` values, similar to an internal node. Values are always `64 bit` in length and represent a row id.

```
| Key 1 | Key 2 | ..... | Key n | [Value 1] | [Value 2] | [Value n] |
```

### Page format for data pages

TODO
