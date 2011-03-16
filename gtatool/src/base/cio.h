/*
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * \file cio.h
 *
 * C-style IO for C++, with exception handling.
 */

#ifndef CIO_H
#define CIO_H

#include <cstdio>
#include <string>
#include <fcntl.h>

#include "exc.h"


/**
 * C-style IO for C++, with exception handling.
 *
 * If the filename argument is optional, it is only used for error messages in
 * the exception. If no filename is given, it is assumed that the file is a
 * temporary file.
 *
 * Exceptions are only thrown for real errors; the locking functions for example
 * return success or failure and only throw an exception if something bad or
 * unexpected happens.
 */

/* Export Linux open(2) flags to systems that lack them. */
#ifndef O_NOATIME
# define O_NOATIME 0
#endif

namespace cio
{
#if W32
    // Fix off_t
#   undef off_t
#   define off_t off64_t
    // Convert sane path names to and from Windows path names.
    std::string to_sys(const std::string &pathname);
    std::string from_sys(const std::string &pathname);
#else
    // On all other systems, these inline functions should be optimized away by the compiler:
    inline const std::string &to_sys(const std::string &pathname) { return pathname; }
    inline const std::string &from_sys(const std::string &pathname) { return pathname; }
#endif

    // fopen / fclose replacements
    FILE *open(const std::string &filename, const std::string &mode, const int flags = 0);
    void close(FILE *f, const std::string &filename = std::string(""));

    // temporary files and directories
    FILE *tempfile(const std::string &base = std::string(""));
    std::string mktempfile(FILE **f, const std::string &base = std::string(""), const std::string &dir = std::string(""));
    std::string mktempdir(const std::string &base = std::string(""), const std::string &dir = std::string(""));

    // buffering
    void disable_buffering(FILE *f, const std::string &filename = std::string(""));

    // advisory locks (for the whole file)
    bool readlock(FILE *f, const std::string &filename = std::string(""));
    bool writelock(FILE *f, const std::string &filename = std::string(""));

    // fread and fwrite replacements
    void read(void *dest, size_t s, size_t n, FILE *f, const std::string &filename = std::string(""));
    void write(const void *src, size_t s, size_t n, FILE *f, const std::string &filename = std::string(""));

    // flush
    void flush(FILE *f, const std::string &filename = std::string(""));

    // fseek and ftello replacements
    bool seekable(FILE *f) throw ();
    void seek(FILE *f, off_t offset, int whence, const std::string &filename = std::string(""));
    void rewind(FILE *f, const std::string &filename = std::string(""));
    off_t tell(FILE *f, const std::string &filename = std::string(""));

    // fgetc/ungetc replacements
    int getc(FILE *f, const std::string &filename = std::string(""));
    void ungetc(int c, FILE *f, const std::string &filename = std::string(""));
    bool has_more(FILE *f, const std::string &filename = std::string(""));

    // read lines from a textfile; removes \n
    std::string readline(FILE *f, const std::string &filename = std::string(""));

    // isatty
    bool isatty(FILE *f) throw ();

    // mmap/munmap replacements
    // These wrappers support only a very limited subset of the real mmap/munmap:
    // - Mapping happens always with MAP_PRIVATE, PROT_READ
    // - The region of the file must exist; it is not automatically created
    // - The length argument must be the same for both functions
    void *map(FILE *f, off_t offset, size_t length, const std::string &filename = std::string(""));
    void unmap(void *start, size_t length, const std::string &filename = std::string(""));

    // hard links
    void link(const std::string &oldfilename, const std::string &newfilename);
    void unlink(const std::string &filename);

    // mkdir and rmdir replacements
    void mkdir(const std::string &dirname);
    void rmdir(const std::string &dirname);

    // remove a file or directory
    void remove(const std::string &pathname);

    // rename a file
    void rename(const std::string &old_path, const std::string &new_path);

    // replacements for shell utilities:
    // mkdir -p  (with a variant that assumes that a given prefix already exists)
    void mkdir_p(const std::string &prefix, const std::string &dirname);
    void mkdir_p(const std::string &dirname);
    // rm -r
    void rm_r(const std::string &pathname);
    // test -e
    bool test_e(const std::string &pathname);
    // test -f
    bool test_f(const std::string &pathname);
    // test -d
    bool test_d(const std::string &pathname);
    // basename name [suffix]
    std::string basename(const std::string &name, const std::string &suffix = std::string(""));
};

#endif
