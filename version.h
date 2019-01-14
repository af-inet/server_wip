#ifndef VERSION_H
#define VERSION_H

#define xstr(s) str(s)
#define str(s) #s

#ifndef COMMIT_HASH
#warning "Missing git commit hash, compiling with incomplete version string."
#define VERSION_STRING ""
#else
#define VERSION_STRING xstr(COMMIT_HASH)
#endif

#endif /* VERSION_H */
