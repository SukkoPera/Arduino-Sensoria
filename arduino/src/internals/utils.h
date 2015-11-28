#ifndef _UTILS_H_INCLUDED
#define _UTILS_H_INCLUDED

#define _UNUSED __attribute__ ((unused))

/* Remove trailing whitespace from a string.
 * String is modified in-place.
 * Returns pointer to stripped string.
 */
char *strstrip (char *s);
  
/* Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 * 
 * From: http://mirror.fsf.org/pmon2000/3.x/src/sdk/libc/string/strlcpy.c
 */
size_t strlcpy (char *dst, const char *src, size_t siz);

/* Convert a float to a string.
 *
 * NOTE: Precision is fixed to two digits.
 */
char *floatToString (double val, char *outstr);

// WARNING: str is updated in-place
int splitString (char *str, char **parts, size_t n);

#endif
