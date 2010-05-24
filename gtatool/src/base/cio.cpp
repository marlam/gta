/*
 * cio.cpp
 *
 * This file is part of gtatool, a tool to manipulate Generic Tagged Arrays
 * (GTAs).
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010
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

#include "config.h"

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <string>
#include <list>

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#if HAVE_MMAP
# include <sys/mman.h>
#endif
#if W32
# include <climits>
# define WIN32_LEAN_AND_MEAN    /* do not include more than necessary */
# define _WIN32_WINNT 0x0502    /* Windows XP SP2 or later */
# include <windows.h>
# include <sys/locking.h>
#endif

#include "exc.h"
#include "msg.h"
#include "cio.h"


/* Define some POSIX functionality that is missing on W32 */

#ifndef HAVE_FTELLO
# define ftello(f) ftello64(f)
#endif

#ifndef HAVE_FSEEKO
# define fseeko(stream, offset, whence) fseeko64(stream, offset, whence)
#endif

#ifndef S_IRWXG
# define S_IRWXG 0
#endif

#ifndef S_IRWXO
# define S_IRWXO 0
#endif

#ifndef HAVE_LINK
static int link(const char *path1, const char *path2)
{
    if (CreateHardLink(path2, path1, NULL) == 0)
    {
        /* It is not documented which errors CreateHardLink() can produce.
         * The following conversions are based on tests on a Windows XP SP2
         * system. */
        DWORD err = GetLastError();
        switch (err)
        {
            case ERROR_ACCESS_DENIED:
                errno = EACCES;
                break;

            case ERROR_INVALID_FUNCTION:        /* fs does not support hard links */
                errno = EPERM;
                break;

            case ERROR_NOT_SAME_DEVICE:
                errno = EXDEV;
                break;

            case ERROR_PATH_NOT_FOUND:
            case ERROR_FILE_NOT_FOUND:
                errno = ENOENT;
                break;

            case ERROR_INVALID_PARAMETER:
                errno = ENAMETOOLONG;
                break;

            case ERROR_TOO_MANY_LINKS:
                errno = EMLINK;
                break;

            case ERROR_ALREADY_EXISTS:
                errno = EEXIST;
                break;

            default:
                errno = EIO;
        }
        return -1;
    }

    return 0;
}
#endif

#if W32
/* W32 lacks a mkdir() function with a mode argument */
static int mkdir(const char *pathname, mode_t)
{
    return _mkdir(pathname);
}
#endif

#if W32
/* W32 neither has mkstemp()/mkdtemp() nor a reliable unlink().
 * The following is a replacement:
 * - w32_mktemp(tmpl, 0) works like mkstemp(tmpl).
 * - w32_mktemp(tmpl, 1) works like mkstemp(tmpl) followed by unlink()
 *   on the generated file.
 * - w32_mktemp(tmpl, 2) works like mkdtemp(tmpl), except for the
 *   return value.
 * In the first two cases, the return value is the file descriptor ot -1 on
 * error. In the third case, the return value is 0 on success and -1 on error.
 * Alle variants will set errno if they fail.
 */
static int w32_mktemp(char *tmpl, int mode)
{
    size_t tmpllen;
    char *X;
    int i;
    int attempt;
    int ret;
    const char alnum[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    tmpllen = strlen(tmpl);
    if (tmpllen < 6)
    {
        errno = EINVAL;
        return -1;
    }
    X = tmpl + tmpllen - 6;
    if (strcmp(X, "XXXXXX") != 0)
    {
        errno = EINVAL;
        return -1;
    }

    /* We have 62^6 possible filenames. We attempt 62^2=3844 times. */
    ret = -1;
    for (attempt = 0; ret == -1 && attempt < 3844; attempt++)
    {
        for (i = 0; i < 6; i++)
        {
            X[i] = alnum[rand() % 62];
        }
        if (mode == 0)
        {
            ret = _open(tmpl, _O_CREAT | _O_EXCL | _O_RDWR
                    | _O_BINARY, _S_IREAD | _S_IWRITE);
        }
        else if (mode == 1)
        {
            ret = _open(tmpl, _O_CREAT | _O_EXCL | _O_RDWR
                    | _O_BINARY | _O_TEMPORARY, _S_IREAD | _S_IWRITE);
        }
        else
        {
            ret = _mkdir(tmpl);
        }
    }
    if (attempt == 3844)
    {
        errno = EEXIST;
    }

    return ret;
}
#endif

static const char DIRSEP
#if W32
        = '\\';
#else
        = '/';
#endif


namespace cio
{
#if W32
    // Convert sane path names to and from Windows path names.
    //
    // Sane path names have the following properties:
    // - The directory separator is '/'
    // - Absolute path names are path names that start with '/'
    //   (no drive letter BS)
    // - Trailing slashes work as expected (i.e. as defined by POSIX)
    //
    // Windows cannot handle path names that are longer than 260 characters.
    // See http://msdn.microsoft.com/en-us/library/aa365247(VS.85).aspx .
    // This is a limit for the full path name, not just for one component.
    // Additionally, Windows does not return the right error (ENAMETOOLONG)
    // when it cannot handle a long path name. Instead, it returns ENOENT,
    // which is misleading. Therefore, we catch this condition here.
    //
    // TODO: "\some\path" would be an 'absolute' path in Windows, but it is
    // only valid for the current drive, so it must be prepended with the
    // drive letter (or UNC name?) to be truly absolute. Fix this if it
    // becomes a problem.

    std::string to_sys(const std::string &pathname) throw (exc)
    {
        std::string s(pathname);
        size_t l = s.length();
        if (l >= 3 && s[0] == '/' && s[2] == ':'
                && ((s[1] >= 'a' && s[1] <= 'z')
                    || (s[1] >= 'A' && s[1] <= 'Z')))
        {
            // A drive letter was masked with '/'. Undo this.
            s.erase(0, 1);
            l--;
        }
        if (l >= 1 && s[l - 1] == '/')
        {
            // Remove a trailing slash since Windows cannot handle it.
            s.resize(--l);
        }
        for (size_t i = 0; i < l; i++)
        {
            if (s[i] == '/')
                s[i] = '\\';
        }
        if (l > PATH_MAX)
        {
            throw exc(std::string("Cannot handle ") + s, ENAMETOOLONG);
        }
        return s;
    }

    std::string from_sys(const std::string &pathname) throw (exc)
    {
        std::string s(pathname);
        size_t l = s.length();
        if (l > PATH_MAX)
        {
            throw exc(std::string("Cannot handle ") + s, ENAMETOOLONG);
        }
        if (l >= 2 && s[1] == ':'
                && ((s[0] >= 'a' && s[0] <= 'z')
                    || (s[0] >= 'A' && s[0] <= 'Z')))
        {
            // Mask a drive letter with '/'.
            s.insert(0, "\\");
        }
        for (size_t i = 0; i < l; i++)
        {
            if (s[i] == '\\')
                s[i] = '/';
        }
        return s;
    }
#endif

    FILE *open(const std::string &filename, const std::string &mode, const int flags) throw (exc)
    {
        if (flags == 0)
        {
            FILE *f;
            if (!(f = ::fopen(to_sys(filename).c_str(), mode.c_str())))
            {
                throw exc("Cannot open " + to_sys(filename), errno);
            }
            return f;
        }
        else
        {
            int fd;
            FILE *f;
            if ((fd = ::open(to_sys(filename).c_str(), flags, S_IRUSR | S_IWUSR)) == -1)
            {
                if ((flags & O_CREAT) && (flags & O_EXCL))
                {
                    throw exc("Cannot create " + to_sys(filename), errno);
                }
                else
                {
                    throw exc("Cannot open " + to_sys(filename), errno);
                }
            }
            if (!(f = fdopen(fd, mode.c_str())))
            {
                throw exc("Cannot open " + to_sys(filename), errno);
            }
            return f;
        }
    }

    void close(FILE *f, const std::string &filename) throw (exc)
    {
        if (::fclose(f) != 0)
        {
            throw exc("Cannot close " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
    }

    /* Internal function that does the real work for creating temporary files or
     * directories.
     * To create a directory, set 'file' to NULL. This function will return the
     * directory name in 'pathname'. If the result is NULL, then the function
     * failed, and errno will be set.
     * To create a file, let this function return the stream in 'file', which must
     * not be NULL. Use a NULL 'pathname' to get tempfile() behaviour (no name
     * returned, file will be deleted) and a non-NULL 'pathname' to get mktempfile()
     * behaviour (name returned, file will not be deleted). If the function fails,
     * the returned stream will be NULL, and errno will be set. */
    static void real_mktemp(const char *dir, const char *base, FILE **file, char **pathname)
    {
        FILE *f;
        size_t baselen;
        size_t dirlen;
        char *tmpl = NULL;
        size_t tmpllen;
        int fd = -1;
        int saved_errno;

        if (!base)
        {
            base = "tmp";
        }
        dirlen = strlen(dir);

        /* The name prefix */
        baselen = strlen(base);

        /* Build the template */
        tmpllen = dirlen + 1 + baselen + 6;
        tmpl = static_cast<char *>(malloc((tmpllen + 1) * sizeof(char)));
        if (!tmpl)
        {
            errno = ENOMEM;             /* Not every malloc() sets this */
            goto error_exit;
        }
        strncpy(tmpl, dir, dirlen);
        if (dirlen == 0 || tmpl[dirlen - 1] != DIRSEP)
        {
            tmpl[dirlen++] = DIRSEP;
        }
        /* tmpl is long enough */
        strncpy(tmpl + dirlen, base, baselen);
        strcpy(tmpl + dirlen + baselen, "XXXXXX");

        /* Create the file or directory */
        if (file)
        {
#if W32
            if ((fd = w32_mktemp(tmpl, pathname ? 0 : 1)) == -1)
            {
                goto error_exit;
            }
#else /* POSIX */
            if ((fd = ::mkstemp(tmpl)) == -1)
            {
                goto error_exit;
            }
            if (::fchmod(fd, S_IRUSR | S_IWUSR) == -1)
            {
                goto error_exit;
            }
            if (!pathname)
            {
                if (::unlink(tmpl) != 0)
                {
                    goto error_exit;
                }
            }
#endif
            if (!(f = ::fdopen(fd, "w+")))
            {
                goto error_exit;
            }
            if (pathname)
            {
                *pathname = tmpl;
            }
            else
            {
                free(tmpl);
            }
            *file = f;
        }
        else
        {
#if W32
            if (w32_mktemp(tmpl, 2) != 0)
            {
                goto error_exit;
            }
#else /* POSIX */
            if (!::mkdtemp(tmpl))
            {
                goto error_exit;
            }
#endif
            *pathname = tmpl;
        }
        return;

error_exit:
        saved_errno = errno;
        if (fd >= 0)
        {
            ::close(fd);
        }
        if (tmpl)
        {
            (void)::remove(tmpl);
            free(tmpl);
        }
        errno = saved_errno;
        if (file)
        {
            *file = NULL;
        }
        else
        {
            *pathname = NULL;
        }
    }

    static const char *default_tmpdir()
    {
        const char *dir;

        if (!(dir = getenv("TMPDIR")))
        {
            /* System dependent default location */
#if W32
            if (!(dir = getenv("TEMP")))
            {
                if (!(dir = getenv("TMP")))
                {
                    dir = "C:";
                }
            }
#else /* UNIX */
# ifdef P_tmpdir
            dir = P_tmpdir;
# else
            dir = "/tmp";
# endif
#endif /* UNIX */
        }

        return dir;
    }

    FILE *tempfile(const std::string &base) throw (exc)
    {
        FILE *f;

        real_mktemp(default_tmpdir(), !base.empty() ? base.c_str() : NULL, &f, NULL);
        if (!f)
        {
            throw exc("Cannot create temporary file", errno);
        }
        return f;
    }

    std::string mktempfile(FILE **f, const std::string &base, const std::string &dir) throw (exc)
    {
        char *filename;
        real_mktemp(dir.empty() ? default_tmpdir() : to_sys(dir).c_str(),
                !base.empty() ? base.c_str() : NULL, f, &filename);
        if (!(*f))
        {
            throw exc("Cannot create temporary file", errno);
        }
        std::string s(filename);
        free(filename);
        return from_sys(s);
    }

    std::string mktempdir(const std::string &base, const std::string &dir) throw (exc)
    {
        char *dirname;
        real_mktemp(dir.empty() ? default_tmpdir() : to_sys(dir).c_str(),
                !base.empty() ? base.c_str() : NULL, NULL, &dirname);
        if (!dirname)
        {
            throw exc("Cannot create temporary directory", errno);
        }
        std::string s(dirname);
        free(dirname);
        return from_sys(s);
    }

    void disable_buffering(FILE *f, const std::string &filename) throw (exc)
    {
        if (::setvbuf(f, NULL, _IONBF, 0) != 0)
        {
            throw exc("Cannot disable buffering for " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
    }

    static bool lock(int mode, FILE *f, const std::string &filename) throw (exc)
    {
        int fd = fileno(f);     // Do not use ::fileno(f); fileno might be a macro
        bool success;
#if !W32
        struct flock lock;
        lock.l_type = (mode == 0 ? F_RDLCK : F_WRLCK);
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;
#endif /* !W32 */

        errno = 0;
#if W32
        success = (::_locking(fd, _LK_NBLCK, LONG_MAX) == 0);
#else /* POSIX */
        success = (::fcntl(fd, F_SETLK, &lock) == 0);
#endif
        if (!success && errno != EACCES && errno != EAGAIN)
        {
            throw exc("Cannot try to lock " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
        return success;
    }

    bool readlock(FILE *f, const std::string &filename) throw (exc)
    {
        return lock(0, f, filename);
    }

    bool writelock(FILE *f, const std::string &filename) throw (exc)
    {
        return lock(1, f, filename);
    }

    void read(void *dest, size_t s, size_t n, FILE *f, const std::string &filename) throw (exc)
    {
        size_t r;

        if ((r = ::fread(dest, s, n, f)) != n)
        {
            if (ferror(f))
            {
                throw exc("Cannot read from " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
            }
            else
            {
                throw exc("Cannot read from " + (!filename.empty() ? to_sys(filename) : "temporary file"), "unexpected end of file");
            }
        }
    }

    void write(const void *dest, size_t s, size_t n, FILE *f, const std::string &filename) throw (exc)
    {
        if (::fwrite(dest, s, n, f) != n)
        {
            throw exc("Cannot write to " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
    }

    void flush(FILE *f, const std::string &filename) throw (exc)
    {
        if (::fflush(f) != 0)
        {
            throw exc("Cannot flush " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
    }

    bool seekable(FILE *f) throw ()
    {
        return (::ftello(f) != -1);
    }

    void seek(FILE *f, off_t offset, int whence, const std::string &filename) throw (exc)
    {
        if (::fseeko(f, offset, whence) != 0)
        {
            throw exc("Cannot seek in " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
    }

    void rewind(FILE *f, const std::string &filename) throw (exc)
    {
        seek(f, 0, SEEK_SET, filename);
    }

    off_t tell(FILE *f, const std::string &filename) throw (exc)
    {
        off_t o;

        if ((o = ::ftello(f)) == -1)
        {
            throw exc("Cannot get position in " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
        return o;
    }

    int getc(FILE *f, const std::string &filename) throw (exc)
    {
        int c = ::fgetc(f);
        if (c == EOF && ferror(f))
        {
            throw exc("Cannot read from " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
        return c;
    }

    void ungetc(int c, FILE *f, const std::string &filename) throw (exc)
    {
        if (::ungetc(c, f) == EOF)
        {
            throw exc("Cannot unget a character from " + (!filename.empty() ? to_sys(filename) : "temporary file"), errno);
        }
    }

    bool has_more(FILE *f, const std::string &filename) throw (exc)
    {
        int c = getc(f, filename);
        if (c == EOF)
        {
            return false;
        }
        else
        {
            ungetc(c, f, filename);
            return true;
        }
    }

    std::string readline(FILE *f, const std::string &filename) throw (exc)
    {
        std::string line;
        char c;

        while ((c = getc(f, filename)) != EOF)
        {
            if (c == '\n')
            {
                break;
            }
            line.push_back(c);
        }
        return line;
    }

    bool isatty(FILE *f) throw ()
    {
        return ::isatty(fileno(f));
    }

    void *map(FILE *f, off_t offset, size_t length, const std::string &filename) throw (exc)
    {
#if HAVE_MMAP

        void *retval;
        if ((retval = ::mmap(NULL, length, PROT_READ, MAP_PRIVATE, fileno(f), offset)) == MAP_FAILED)
        {
            throw exc("Cannot map " + (!filename.empty() ? to_sys(filename) : "temporary file") + " to memory", errno);
        }
        return retval;

#else

        uint8_t *data = new uint8_t[length];
        off_t old_offset = 0;
        if (offset != 0)
        {
            old_offset = tell(f, filename);
            seek(f, offset, SEEK_SET, filename);
        }
        read(data, length, 1, f, filename);
        if (offset != 0)
        {
            seek(f, old_offset, SEEK_SET, filename);
        }
        return data;

#endif
    }

    void unmap(void *start, size_t length, const std::string &filename) throw (exc)
    {
#if HAVE_MMAP

        if (::munmap(start, length) != 0)
        {
            throw exc("Cannot unmap " + (!filename.empty() ? to_sys(filename) : "temporary file") + " from memory", errno);
        }

#else

        uint8_t *data = static_cast<uint8_t *>(start);
        delete[] data;

#endif
    }

    void link(const std::string &oldfilename, const std::string &newfilename) throw (exc)
    {
        if (::link(to_sys(oldfilename).c_str(), to_sys(newfilename).c_str()) != 0)
        {
            throw exc("Cannot create link " + to_sys(newfilename) + " for " + to_sys(oldfilename), errno);
        }
    }

    void unlink(const std::string &filename) throw (exc)
    {
        if (::unlink(to_sys(filename).c_str()) != 0)
        {
            throw exc("Cannot unlink " + to_sys(filename), errno);
        }
    }

    void mkdir(const std::string &dirname) throw (exc)
    {
        if (::mkdir(to_sys(dirname).c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
        {
            throw exc("Cannot create directory " + to_sys(dirname), errno);
        }
    }

    void rmdir(const std::string &dirname) throw (exc)
    {
        if (::rmdir(to_sys(dirname).c_str()) != 0)
        {
            throw exc("Cannot remove directory " + to_sys(dirname), errno);
        }
    }

    void remove(const std::string &pathname) throw (exc)
    {
        if (test_d(pathname))
        {
            rmdir(pathname);
        }
        else
        {
            unlink(pathname);
        }
    }

    void rename(const std::string &old_path, const std::string &new_path) throw (exc)
    {
        // TODO: work around W32 bug: rename cannot replace existing files
        if (::rename(to_sys(old_path).c_str(), to_sys(new_path).c_str()) != 0)
        {
            throw exc("Cannot rename " + to_sys(old_path) + " to " + to_sys(new_path), errno);
        }
    }

    void mkdir_p(const std::string &prefix, const std::string &dirname) throw (exc)
    {
        std::string p(to_sys(prefix));
        int pl = p.length();
        if (pl > 0 && p[pl - 1] != DIRSEP)
        {
            p += DIRSEP;
            pl++;
        }
        std::string d(to_sys(dirname));
        int dl = d.length();
#if W32
        // Work around inconsistencies in the Windows file system view.
        if (pl == 0)
        {
            if (dl >= 3 && d[1] == ':' && d[2] == DIRSEP)
            {
                // Move drive letters to the existing prefix because mkdir("C:") does not lead to EEXIST.
                p = d.substr(0, 3);
                pl = 3;
                d = d.substr(3);
                dl -= 3;
            }
            else if (dl >= 3 && d[0] == DIRSEP && d[1] == DIRSEP)
            {
                // Move network names ("\\server\") to the existing prefix because they are special, too.
                size_t j = d.find(DIRSEP, 2);
                if (j == std::string::npos)
                {
                    j = dl - 1;
                }
                p = d.substr(0, j + 1);
                pl = j + 1;
                if (j == static_cast<size_t>(dl - 1))
                {
                    d.clear();
                    dl = 0;
                }
                else
                {
                    d = d.substr(j + 1);
                    dl -= j + 1;
                }
            }
        }
#endif
        std::string s = p + d + '\0';
        bool excor = false;
        for (int i = (pl == 0 ? 1 : pl); i <= pl + dl; i++)
        {
            if (s[i] == DIRSEP || s[i] == '\0')
            {
                s[i] = '\0';
                // This must be thread-safe and free of race conditions.
                // Therefore, first call mkdir(), and only check if the
                // directory already existed if mkdir() failed.
                if (::mkdir(s.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
                {
                    if (errno == EEXIST)
                    {
                        struct stat buf;
                        int r = ::stat(s.c_str(), &buf);
                        if (r != 0 || !S_ISDIR(buf.st_mode))
                        {
                            excor = true;
                            errno = EEXIST;
                        }
                    }
                    else
                    {
                        excor = true;
                    }
                }
                if (excor)
                {
                    throw exc("Cannot create directory " + s, errno);
                }
                s[i] = DIRSEP;
            }
        }
    }

    void mkdir_p(const std::string &dirname) throw (exc)
    {
        mkdir_p(std::string(), dirname);
    }

    void rm_r(const std::string &pathname) throw (exc)
    {
        std::list<std::string> pathlist;
        std::list<std::string>::iterator it;

        pathlist.push_back(to_sys(pathname));
        while (!pathlist.empty())
        {
            bool pathlist_grew = false;
            it = pathlist.begin();
            while (!pathlist_grew && it != pathlist.end())
            {
                if (test_d(*it))
                {
                    DIR *dir;
                    struct dirent *ent;
                    if (!(dir = opendir((*it).c_str())))
                    {
                        throw exc("Cannot remove " + (*it), errno);
                    }
                    for (;;)
                    {
                        errno = 0;
                        ent = readdir(dir);
                        if (!ent && errno != 0)
                        {
                            throw exc("Cannot remove " + (*it), errno);
                        }
                        else if (!ent)
                        {
                            break;
                        }
                        else if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                        {
                            continue;
                        }
                        else
                        {
                            pathlist.insert(it, (*it) + DIRSEP + std::string(ent->d_name));
                            pathlist_grew = true;
                        }
                    }
                    if (closedir(dir) != 0)
                    {
                        throw exc("Cannot remove " + (*it), errno);
                    }
                    if (!pathlist_grew)
                    {
                        rmdir(*it);
                        it = pathlist.erase(it);
                    }
                }
                else
                {
                    unlink(*it);
                    it = pathlist.erase(it);
                }
            }
        }
    }

    static bool test(int mode, const std::string &pathname) throw (exc)
    {
        struct stat buf;
        int r = ::stat(to_sys(pathname).c_str(), &buf);
        if (r == 0
                && (   (mode == 0 && S_ISREG(buf.st_mode))
                    || (mode == 1 && S_ISDIR(buf.st_mode))))
        {
            return true;
        }
        else if (r != 0 && errno != ENOENT)
        {
            throw exc("Cannot test pathname " + to_sys(pathname), errno);
        }
        else
        {
            return false;
        }
    }

    bool test_f(const std::string &pathname) throw (exc)
    {
        return test(0, pathname);
    }

    bool test_d(const std::string &pathname) throw (exc)
    {
        return test(1, pathname);
    }

    std::string basename(const std::string &name, const std::string &suffix) throw (exc)
    {
        std::string base(name);
        size_t slash = name.find_last_of('/');
        if (slash != std::string::npos)
        {
            base = base.substr(slash + 1);
        }
        if (suffix.length() > 0 && base.substr(base.length() - suffix.length()).compare(suffix) == 0)
        {
            base = base.substr(0, base.length() - suffix.length());
        }
        return base;
    }
}
