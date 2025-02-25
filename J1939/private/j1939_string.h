#ifndef J1939_PRIVATE_STRING_H_
#define J1939_PRIVATE_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

static inline int jstring_copy(char *dst, const char *src, int n) {
    char *pdst = dst;
    const char *psrc = src;
    int len = 0;

    if (!src || !dst) {
        return 0;
    }

    while (*psrc && n > 0) {
        *pdst++ = *psrc++;
        ++len;
        --n;
    }

    if (len) {
        pdst[len] = '\0';
    }

    return len;
}


static inline int jstring_copy_maxmin(char *dst, int fill, const char *src, int max, int min) {
    char *pdst = dst;
    const char *psrc = src;
    int len = 0;

    if (!src || !dst) {
        return 0;
    }

    while (*psrc && max > 0) {
        *pdst++ = *psrc++;
        ++len;
        --max;
        --min;
    }

    while (min > 0) {
        *pdst++ = (char) fill;
        ++len;
        --min;
    }

    if (len) {
        pdst[len] = '\0';
    }

    return len;
}

#ifdef __cplusplus
}
#endif

#endif /* J1939_PRIVATE_STRING_H_ */
