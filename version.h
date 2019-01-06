#ifndef VERSION_H
#define VERSION_H

#define xstr(a) str(a)
#define str(a) #a

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#ifndef COMMIT_HASH
#warning "Missing git commit hash, compiling with incomplete version string."
#define COMMIT_STRING ""
#else
#define COMMIT_STRING " (" xstr(COMMIT_HASH) ")"
#endif

#define CONCAT(a, b, c, d) \
    xstr(a) "." xstr(b) "." xstr(c) d

#define VERSION_STRING CONCAT(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, COMMIT_STRING)

#endif /* VERSION_H */
