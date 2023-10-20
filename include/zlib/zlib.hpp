#ifndef ZLIB_H
#define ZLIB_H

#include "zconf.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#define ZLIB_VERSION "1.3"
#define ZLIB_VERNUM 0x1300
#define ZLIB_VER_MAJOR 1
#define ZLIB_VER_MINOR 3
#define ZLIB_VER_REVISION 0
#define ZLIB_VER_SUBREVISION 0

	typedef voidpf(*alloc_func)(voidpf opaque, uInt items, uInt size);
	typedef void   (*free_func)(voidpf opaque, voidpf address);

	struct internal_state;

	typedef struct z_stream_s {
		z_const Bytef* next_in;         
		uInt     avail_in;         
		uLong    total_in;           

		Bytef* next_out;        
		uInt     avail_out;       
		uLong    total_out;         

		z_const char* msg;          
		struct internal_state FAR* state;      

		alloc_func zalloc;         
		free_func  zfree;          
		voidpf     opaque;           

		int     data_type;           
		uLong   adler;               
		uLong   reserved;        
	} z_stream;

	typedef z_stream FAR* z_streamp;

	typedef struct gz_header_s {
		int     text;                
		uLong   time;          
		int     xflags;               
		int     os;            
		Bytef* extra;              
		uInt    extra_len;           
		uInt    extra_max;          
		Bytef* name;              
		uInt    name_max;           
		Bytef* comment;          
		uInt    comm_max;           
		int     hcrc;                  
		int     done;               
	} gz_header;

	typedef gz_header FAR* gz_headerp;

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
#define Z_BLOCK         5
#define Z_TREES         6
#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_RLE                 3
#define Z_FIXED               4
#define Z_DEFAULT_STRATEGY    0
#define Z_BINARY   0
#define Z_TEXT     1
#define Z_ASCII    Z_TEXT          
#define Z_UNKNOWN  2
#define Z_DEFLATED   8
#define Z_NULL  0        

#define zlib_version zlibVersion()
	const char* zlibVersion(void);
	int  deflate(z_streamp strm, int flush);
	int  deflateEnd(z_streamp strm);
	int  inflate(z_streamp strm, int flush);
	int  inflateEnd(z_streamp strm);
	int  deflateSetDictionary(z_streamp strm,
		const Bytef* dictionary,
		uInt  dictLength);
	int  deflateGetDictionary(z_streamp strm,
		Bytef* dictionary,
		uInt* dictLength);
	int  deflateCopy(z_streamp dest,
		z_streamp source);
	int  deflateReset(z_streamp strm);
	int  deflateParams(z_streamp strm,
		int level,
		int strategy);
	int  deflateTune(z_streamp strm,
		int good_length,
		int max_lazy,
		int nice_length,
		int max_chain);
	uLong  deflateBound(z_streamp strm,
		uLong sourceLen);
	int  deflatePending(z_streamp strm,
		unsigned* pending,
		int* bits);
	int  deflatePrime(z_streamp strm,
		int bits,
		int value);
	int  deflateSetHeader(z_streamp strm,
		gz_headerp head);
	int  inflateSetDictionary(z_streamp strm,
		const Bytef* dictionary,
		uInt  dictLength);
	int  inflateGetDictionary(z_streamp strm,
		Bytef* dictionary,
		uInt* dictLength);
	int  inflateSync(z_streamp strm);
	int  inflateCopy(z_streamp dest,
		z_streamp source);
	int  inflateReset(z_streamp strm);
	int  inflateReset2(z_streamp strm,
		int windowBits);
	int  inflatePrime(z_streamp strm,
		int bits,
		int value);
	long  inflateMark(z_streamp strm);
	int  inflateGetHeader(z_streamp strm,
		gz_headerp head);
	typedef unsigned (*in_func)(void FAR*,
		z_const unsigned char FAR* FAR*);
	typedef int (*out_func)(void FAR*, unsigned char FAR*, unsigned);

	int  inflateBack(z_streamp strm,
		in_func in, void FAR* in_desc,
		out_func out, void FAR* out_desc);
	int  inflateBackEnd(z_streamp strm);
	uLong  zlibCompileFlags(void);
#ifndef Z_SOLO

	int  compress(Bytef* dest, uLongf* destLen,
		const Bytef* source, uLong sourceLen);
	int  compress2(Bytef* dest, uLongf* destLen,
		const Bytef* source, uLong sourceLen,
		int level);
	uLong  compressBound(uLong sourceLen);
	int  uncompress(Bytef* dest, uLongf* destLen,
		const Bytef* source, uLong sourceLen);
	int  uncompress2(Bytef* dest, uLongf* destLen,
		const Bytef* source, uLong* sourceLen);
	typedef struct gzFile_s* gzFile;         

#endif   

	uLong  adler32(uLong adler, const Bytef* buf, uInt len);
	uLong  adler32_z(uLong adler, const Bytef* buf,
		z_size_t len);
	uLong  crc32(uLong crc, const Bytef* buf, uInt len);
	uLong  crc32_z(uLong crc, const Bytef* buf,
		z_size_t len);
	uLong  crc32_combine_op(uLong crc1, uLong crc2, uLong op);
	int  deflateInit_(z_streamp strm, int level,
		const char* version, int stream_size);
	int  inflateInit_(z_streamp strm,
		const char* version, int stream_size);
	int  deflateInit2_(z_streamp strm, int  level, int  method,
		int windowBits, int memLevel,
		int strategy, const char* version,
		int stream_size);
	int  inflateInit2_(z_streamp strm, int  windowBits,
		const char* version, int stream_size);
	int  inflateBackInit_(z_streamp strm, int windowBits,
		unsigned char FAR* window,
		const char* version,
		int stream_size);
#ifdef Z_PREFIX_SET
#  define z_deflateInit(strm, level) \
          deflateInit_((strm), (level), ZLIB_VERSION, (int)sizeof(z_stream))
#  define z_inflateInit(strm) \
          inflateInit_((strm), ZLIB_VERSION, (int)sizeof(z_stream))
#  define z_deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
          deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
                        (strategy), ZLIB_VERSION, (int)sizeof(z_stream))
#  define z_inflateInit2(strm, windowBits) \
          inflateInit2_((strm), (windowBits), ZLIB_VERSION, \
                        (int)sizeof(z_stream))
#  define z_inflateBackInit(strm, windowBits, window) \
          inflateBackInit_((strm), (windowBits), (window), \
                           ZLIB_VERSION, (int)sizeof(z_stream))
#else
#  define deflateInit(strm, level) \
          deflateInit_((strm), (level), ZLIB_VERSION, (int)sizeof(z_stream))
#  define inflateInit(strm) \
          inflateInit_((strm), ZLIB_VERSION, (int)sizeof(z_stream))
#  define deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
          deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
                        (strategy), ZLIB_VERSION, (int)sizeof(z_stream))
#  define inflateInit2(strm, windowBits) \
          inflateInit2_((strm), (windowBits), ZLIB_VERSION, \
                        (int)sizeof(z_stream))
#  define inflateBackInit(strm, windowBits, window) \
          inflateBackInit_((strm), (windowBits), (window), \
                           ZLIB_VERSION, (int)sizeof(z_stream))
#endif

#ifndef Z_SOLO

#ifdef Z_PREFIX_SET
#  undef z_gzgetc
#  define z_gzgetc(g) \
          ((g)->have ? ((g)->have--, (g)->pos++, *((g)->next)++) : (gzgetc)(g))
#else
#  define gzgetc(g) \
          ((g)->have ? ((g)->have--, (g)->pos++, *((g)->next)++) : (gzgetc)(g))
#endif

#ifdef Z_LARGE64
	uLong  adler32_combine64(uLong, uLong, z_off64_t);
	uLong  crc32_combine64(uLong, uLong, z_off64_t);
	uLong  crc32_combine_gen64(z_off64_t);
#endif

#if defined(Z_WANT64)
#  ifdef Z_PREFIX_SET
#    define z_adler32_combine z_adler32_combine64
#    define z_crc32_combine z_crc32_combine64
#    define z_crc32_combine_gen z_crc32_combine_gen64
#  else
#    define adler32_combine adler32_combine64
#    define crc32_combine crc32_combine64
#    define crc32_combine_gen crc32_combine_gen64
#  endif
#  ifndef Z_LARGE64
	uLong  adler32_combine64(uLong, uLong, z_off_t);
	uLong  crc32_combine64(uLong, uLong, z_off_t);
	uLong  crc32_combine_gen64(z_off_t);
#  endif
#else
	uLong  adler32_combine(uLong, uLong, z_off_t);
	uLong  crc32_combine(uLong, uLong, z_off_t);
	uLong  crc32_combine_gen(z_off_t);
#endif

#else   

	uLong  adler32_combine(uLong, uLong, z_off_t);
	uLong  crc32_combine(uLong, uLong, z_off_t);
	uLong  crc32_combine_gen(z_off_t);

#endif   

	const char* zError(int);
	int             inflateSyncPoint(z_streamp);
	const z_crc_t FAR* get_crc_table(void);
	int             inflateUndermine(z_streamp, int);
	int             inflateValidate(z_streamp, int);
	unsigned long   inflateCodesUsed(z_streamp);
	int             inflateResetKeep(z_streamp);
	int             deflateResetKeep(z_streamp);
#if defined(_WIN32) && !defined(Z_SOLO)
#endif
#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#  endif
#endif

#ifdef __cplusplus
}
#endif

#endif   