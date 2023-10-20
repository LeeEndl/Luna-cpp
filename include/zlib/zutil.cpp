#include "zutil.hpp"

z_const char* const z_errmsg[10] = {
	(z_const char*)"need dictionary",               
	(z_const char*)"stream end",                   
	(z_const char*)"",                                     
	(z_const char*)"file error",                     
	(z_const char*)"stream error",            
	(z_const char*)"data error",                
	(z_const char*)"insufficient memory",        
	(z_const char*)"buffer error",               
	(z_const char*)"incompatible version",   
	(z_const char*)""
};

const char* zlibVersion(void) {
	return ZLIB_VERSION;
}

uLong  zlibCompileFlags(void) {
	uLong flags;

	flags = 0;
	switch ((int)(sizeof(uInt))) {
	case 2:     break;
	case 4:     flags += 1;     break;
	case 8:     flags += 2;     break;
	default:    flags += 3;
	}
	switch ((int)(sizeof(uLong))) {
	case 2:     break;
	case 4:     flags += 1 << 2;        break;
	case 8:     flags += 2 << 2;        break;
	default:    flags += 3 << 2;
	}
	switch ((int)(sizeof(voidpf))) {
	case 2:     break;
	case 4:     flags += 1 << 4;        break;
	case 8:     flags += 2 << 4;        break;
	default:    flags += 3 << 4;
	}
	switch ((int)(sizeof(z_off_t))) {
	case 2:     break;
	case 4:     flags += 1 << 6;        break;
	case 8:     flags += 2 << 6;        break;
	default:    flags += 3 << 6;
	}
#ifdef ZLIB_DEBUG
	flags += 1 << 8;
#endif
#ifdef ZLIB_WINAPI
	flags += 1 << 10;
#endif
#ifdef BUILDFIXED
	flags += 1 << 12;
#endif
#ifdef DYNAMIC_CRC_TABLE
	flags += 1 << 13;
#endif
#ifdef NO_GZCOMPRESS
	flags += 1L << 16;
#endif
#ifdef NO_GZIP
	flags += 1L << 17;
#endif
#ifdef PKZIP_BUG_WORKAROUND
	flags += 1L << 20;
#endif
#ifdef FASTEST
	flags += 1L << 21;
#endif
#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifdef NO_vsnprintf
	flags += 1L << 25;
#    ifdef HAS_vsprintf_void
	flags += 1L << 26;
#    endif
#  else
#    ifdef HAS_vsnprintf_void
	flags += 1L << 26;
#    endif
#  endif
#else
	flags += 1L << 24;
#  ifdef NO_snprintf
	flags += 1L << 25;
#    ifdef HAS_sprintf_void
	flags += 1L << 26;
#    endif
#  else
#    ifdef HAS_snprintf_void
	flags += 1L << 26;
#    endif
#  endif
#endif
	return flags;
}

#ifdef ZLIB_DEBUG
#include <stdlib.h>
#  ifndef verbose
#    define verbose 0
#  endif
int  z_verbose = verbose;

void  z_error(char* m) {
	fprintf(stderr, "%s\n", m);
	exit(1);
}
#endif

const char* zError(int err) {
	return ERR_MSG(err);
}

#if defined(_WIN32_WCE) && _WIN32_WCE < 0x800
int errno = 0;
#endif

#ifndef HAVE_MEMCPY

void  zmemcpy(Bytef* dest, const Bytef* source, uInt len) {
	if (len == 0) return;
	do {
		*dest++ = *source++;      
	} while (--len != 0);
}

int  zmemcmp(const Bytef* s1, const Bytef* s2, uInt len) {
	uInt j;

	for (j = 0; j < len; j++) {
		if (s1[j] != s2[j]) return 2 * (s1[j] > s2[j]) - 1;
	}
	return 0;
}

void  zmemzero(Bytef* dest, uInt len) {
	if (len == 0) return;
	do {
		*dest++ = 0;       
	} while (--len != 0);
}
#endif

#ifndef MY_ZCALLOC         

#ifndef STDC
extern voidp malloc(uInt size);
extern voidp calloc(uInt items, uInt size);
extern void free(voidpf ptr);
#endif

voidpf  zcalloc(voidpf opaque, unsigned items, unsigned size) {
	(void)opaque;
	return sizeof(uInt) > 2 ? (voidpf)malloc(items * size) :
		(voidpf)calloc(items, size);
}

void  zcfree(voidpf opaque, voidpf ptr) {
	(void)opaque;
	free(ptr);
}

#endif
