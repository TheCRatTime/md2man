TH "README" "1" "2026-10-11" "TheCRatTime"
# md2man
**md2man** - generate man pages.

## Description
This is a small generator for man-pages from
Markdown markup.

There is a changed [mdcat](https://github.com/TheCRatTime/mdcat/),
just changed escape codes to roff markup.

### Features
- Minimal memory usage: (state-machine logic).
- Fast conversion.

### Build
Needed:
- C Compiler
- CMake 3.10 and greater
- GNU Make

```bash
cmake -B build
cd build/
make
```

### Authors
TheCRatTime

### License
Project is under Apache 2.0 license.

### See also
After build, you can show this README at manual:

```bash
# in build
./md2man ../README.md | man -l -
```

