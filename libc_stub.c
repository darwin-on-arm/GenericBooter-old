/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * The contents of this file constitute Original Code as defined in and
 * are subject to the Apple Public Source License Version 1.1 (the
 * "License").  You may not use this file except in compliance with the
 * License.  Please obtain a copy of the License at
 * http://www.apple.com/publicsource and read it before using this file.
 * 
 * This Original Code and all software distributed under the License are
 * distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#include "genboot.h"
#include <limits.h>

char tolower(char c);

char *strcpy(char *s1, const char *s2)
{
    register char *ret = s1;
    while ((*s1++ = *s2++))
        continue;
    return ret;
}

char *strncpy(char *s1, const char *s2, size_t n)
{
    register char *ret = s1;
    while (n && (*s1++ = *s2++))
        n--;
    if (!n)
        *s1 = 0;
    return ret;
}

char *strncat(char *s1, const char *s2, size_t n)
{
    register char *ret = s1;
    while (*s1)
        s1++;
    while (n-- && *s2)
        *s1++ = *s2++;
    *s1 = '\0';
    return ret;
}

char *strcat(char *s1, const char *s2)
{
    return (strncat(s1, s2, strlen(s2)));
}

/*#if STRNCASECMP*/
int strncasecmp(const char *s1, const char *s2, size_t len)
{
    register int n = len;
    while (--n >= 0 && tolower(*s1) == tolower(*s2++))
        if (*s1++ == '\0')
            return (0);
    return (n < 0 ? 0 : tolower(*s1) - tolower(*--s2));
}

/*#endif*/

int isupper(char c)
{
    return (c >= 'A' && c <= 'Z');
}

int isalpha(char c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

int isspace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\12');
}

int isdigit(char c)
{
    return (c >= '0' && c <= '9');
}

char tolower(char c)
{
    return isalpha(c) ? (c | 0x20) : c;
}

/*
 * Convert a string to a long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
long strtol(nptr, endptr, base)
const char *nptr;
char **endptr;
register int base;
{
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    /*
     * Skip white space and pick up leading +/- sign if any.
     * If base is 0, allow 0x for hex and 0 for octal, else
     * assume decimal; if base is already 16, allow 0x.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    } else if ((base == 0 || base == 2) && c == '0' && (*s == 'b' || *s == 'B')) {
        c = s[1];
        s += 2;
        base = 2;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    /*
     * Compute the cutoff value between legal numbers and illegal
     * numbers.  That is the largest legal value, divided by the
     * base.  An input number that is greater than this value, if
     * followed by a legal input character, is too big.  One that
     * is equal to this value may be valid or not; the limit
     * between valid and invalid numbers is then based on the last
     * digit.  For instance, if the range for longs is
     * [-2147483648..2147483647] and the input base is 10,
     * cutoff will be set to 214748364 and cutlim to either
     * 7 (neg==0) or 8 (neg==1), meaning that if we have accumulated
     * a value > 214748364, or equal but the next digit is > 7 (or 8),
     * the number is too big, and we will return a range error.
     *
     * Set any if any `digits' consumed; make it negative to indicate
     * overflow.
     */
    cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if ((any < 0) || (acc > cutoff)
            || ((acc == cutoff) && (c > cutlim)))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
//      errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

/*
 * Convert a string to an unsigned long integer.
 *
 * Ignores `locale' stuff.  Assumes that the upper and lower case
 * alphabets and digits are each contiguous.
 */
unsigned long strtoul(nptr, endptr, base)
const char *nptr;
char **endptr;
register int base;
{
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    /*
     * See strtol for comments as to the logic used.
     */
    do {
        c = *s++;
    } while (isspace(c));
    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    if ((base == 0 || base == 16) && c == '0' && (*s == 'x' || *s == 'X')) {
        c = s[1];
        s += 2;
        base = 16;
    } else if ((base == 0 || base == 2) && c == '0' && (*s == 'b' || *s == 'B')) {
        c = s[1];
        s += 2;
        base = 2;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;
    cutoff = (unsigned long)ULONG_MAX / (unsigned long)base;
    cutlim = (unsigned long)ULONG_MAX % (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if ((any < 0) || (acc > cutoff)
            || ((acc == cutoff) && (c > cutlim)))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = ULONG_MAX;
//      errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *)(any ? s - 1 : nptr);
    return (acc);
}

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar) (const void *, const void *))
{
    int l = 0;
    int u = nmemb - 1;
    int m;
    void *mp;
    int r;

    while (l <= u) {
        m = (l + u) / 2;
        mp = (void *)(((char *)base) + (m * size));
        if ((r = (*compar) (key, mp)) == 0) {
            return mp;
        } else if (r < 0) {
            u = m - 1;
        } else {
            l = m + 1;
        }
    }
    return NULL;
}

char *strtok(s, delim)
register char *s;
register const char *delim;
{
    register char *spanp;
    register int c, sc;
    char *tok;
    static char *last;

    if (s == NULL && (s = last) == NULL)
        return (NULL);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
 cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {               /* no non-delimiter characters */
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = (char *)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

void __assert(const char *filename, int linenumber, const char *assertion)
{
    printf("Assertion %s failed in %s, line %d.\n",
           assertion, filename, linenumber);
    while (1) ;
}

void sleep(int s)
{
    ;
}

char *gets(char *str)
{
    return NULL;
}
