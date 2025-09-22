# Lab work 2
## Author
Журавлев Александр Антонович 24.Б81-мм
## Contacts
st129760@student.spbu.ru

# Skip List Container

A C++17 implementation of a skip list container with STL-style interface.

## Features

- **STL-compatible** API design
- **O(log n)** average complexity for search/insert/delete
- Custom **comparators** and **allocators** support
- **Move-aware** operations
- Comprehensive **unit tests**
- Minimal dependencies

## Building & Testing

### Requirements
- C++17 compiler (GCC 7+/Clang 5+/MSVC 19.20+)
- GNU Make
- Google Test (development headers)

### Using Makefile

```bash
# Build tests
make

# Run all tests
make test

# Clean build artifacts
make clean

# Format code (requires clang-format)
make format
```

## API Highlights

| Operation          | Method                      | Complexity  |
|--------------------|-----------------------------|-------------|
| Insert element     | `insert(const T&)`          | O(log n)    |
| Remove element     | `erase(const T&)`           | O(log n)    |
| Find element       | `find(const T&)`            | O(log n)    |
| Check size         | `size()`                    | O(1)        |
| Iterate elements   | `begin()`, `end()`          | O(n)        |

## CI Integration

Preconfigured GitHub Actions workflow (`ci.yml`) tests:
- Linux
- GCC/Clang
- C++17/C++20 standards
