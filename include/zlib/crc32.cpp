#ifdef MAKECRCH
#  include <stdio.h>
#  ifndef DYNAMIC_CRC_TABLE
#    define DYNAMIC_CRC_TABLE
#  endif
#endif

#include "zutil.hpp"

#ifdef Z_TESTN
#  define N Z_TESTN
#else
#  define N 5
#endif
#if N < 1 || N > 6
#  error N must be in 1..6
#endif

#ifdef Z_TESTW
#  if Z_TESTW-1 != -1
#    define W Z_TESTW
#  endif
#else
#  ifdef MAKECRCH
#    define W 8
#  else
#    if defined(__x86_64__) || defined(__aarch64__)
#      define W 8
#    else
#      define W 4
#    endif
#  endif
#endif
#ifdef W
#  if W == 8 && defined(Z_U8)
typedef Z_U8 z_word_t;
#  elif defined(Z_U4)
#    undef W
#    define W 4
typedef Z_U4 z_word_t;
#  else
#    undef W
#  endif
#endif

#if defined(__aarch64__) && defined(__ARM_FEATURE_CRC32) && W == 8
#  define ARMCRC32
#endif

#if defined(W) && (!defined(ARMCRC32) || defined(DYNAMIC_CRC_TABLE))
local z_word_t byte_swap(z_word_t word) {
#  if W == 8
	return
		(word & 0xff00000000000000) >> 56 |
		(word & 0xff000000000000) >> 40 |
		(word & 0xff0000000000) >> 24 |
		(word & 0xff00000000) >> 8 |
		(word & 0xff000000) << 8 |
		(word & 0xff0000) << 24 |
		(word & 0xff00) << 40 |
		(word & 0xff) << 56;
#  else
	return
		(word & 0xff000000) >> 24 |
		(word & 0xff0000) >> 8 |
		(word & 0xff00) << 8 |
		(word & 0xff) << 24;
#  endif
}
#endif

#ifdef DYNAMIC_CRC_TABLE
local z_crc_t FAR x2n_table[32];
#else
#  include "crc32.hpp"
#endif

#define POLY 0xedb88320

local z_crc_t multmodp(z_crc_t a, z_crc_t b) {
	z_crc_t m, p;

	m = (z_crc_t)1 << 31;
	p = 0;
	for (;;) {
		if (a & m) {
			p ^= b;
			if ((a & (m - 1)) == 0)
				break;
		}
		m >>= 1;
		b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
	}
	return p;
}

local z_crc_t x2nmodp(z_off64_t n, unsigned k) {
	z_crc_t p;

	p = (z_crc_t)1 << 31;
	while (n) {
		if (n & 1)
			p = multmodp(x2n_table[k & 31], p);
		n >>= 1;
		k++;
	}
	return p;
}

const z_crc_t FAR* get_crc_table(void) {
#ifdef DYNAMIC_CRC_TABLE
	once(&made, make_crc_table);
#endif
	return (const z_crc_t FAR*)crc_table;
}

#ifdef W

local z_crc_t crc_word(z_word_t data) {
	int k;
	for (k = 0; k < W; k++)
		data = (data >> 8) ^ crc_table[data & 0xff];
	return (z_crc_t)data;
}

local z_word_t crc_word_big(z_word_t data) {
	int k;
	for (k = 0; k < W; k++)
		data = (data << 8) ^
		crc_big_table[(data >> ((W - 1) << 3)) & 0xff];
	return data;
}

#endif

unsigned long  crc32_z(unsigned long crc, const unsigned char FAR* buf,
	z_size_t len) {
	if (buf == Z_NULL) return 0;

#ifdef DYNAMIC_CRC_TABLE
	once(&made, make_crc_table);
#endif

	crc = (~crc) & 0xffffffff;

#ifdef W

	if (len >= N * W + W - 1) {
		z_size_t blks;
		z_word_t const* words;
		unsigned endian;
		int k;

		while (len && ((z_size_t)buf & (W - 1)) != 0) {
			len--;
			crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		}

		blks = len / (N * W);
		len -= blks * N * W;
		words = (z_word_t const*)buf;

		endian = 1;
		if (*(unsigned char*)&endian) {
			z_crc_t crc0;
			z_word_t word0;
#if N > 1
			z_crc_t crc1;
			z_word_t word1;
#if N > 2
			z_crc_t crc2;
			z_word_t word2;
#if N > 3
			z_crc_t crc3;
			z_word_t word3;
#if N > 4
			z_crc_t crc4;
			z_word_t word4;
#if N > 5
			z_crc_t crc5;
			z_word_t word5;
#endif
#endif
#endif
#endif
#endif

			crc0 = crc;
#if N > 1
			crc1 = 0;
#if N > 2
			crc2 = 0;
#if N > 3
			crc3 = 0;
#if N > 4
			crc4 = 0;
#if N > 5
			crc5 = 0;
#endif
#endif
#endif
#endif
#endif

			while (--blks) {
				word0 = crc0 ^ words[0];
#if N > 1
				word1 = crc1 ^ words[1];
#if N > 2
				word2 = crc2 ^ words[2];
#if N > 3
				word3 = crc3 ^ words[3];
#if N > 4
				word4 = crc4 ^ words[4];
#if N > 5
				word5 = crc5 ^ words[5];
#endif
#endif
#endif
#endif
#endif
				words += N;

				crc0 = crc_braid_table[0][word0 & 0xff];
#if N > 1
				crc1 = crc_braid_table[0][word1 & 0xff];
#if N > 2
				crc2 = crc_braid_table[0][word2 & 0xff];
#if N > 3
				crc3 = crc_braid_table[0][word3 & 0xff];
#if N > 4
				crc4 = crc_braid_table[0][word4 & 0xff];
#if N > 5
				crc5 = crc_braid_table[0][word5 & 0xff];
#endif
#endif
#endif
#endif
#endif
				for (k = 1; k < W; k++) {
					crc0 ^= crc_braid_table[k][(word0 >> (k << 3)) & 0xff];
#if N > 1
					crc1 ^= crc_braid_table[k][(word1 >> (k << 3)) & 0xff];
#if N > 2
					crc2 ^= crc_braid_table[k][(word2 >> (k << 3)) & 0xff];
#if N > 3
					crc3 ^= crc_braid_table[k][(word3 >> (k << 3)) & 0xff];
#if N > 4
					crc4 ^= crc_braid_table[k][(word4 >> (k << 3)) & 0xff];
#if N > 5
					crc5 ^= crc_braid_table[k][(word5 >> (k << 3)) & 0xff];
#endif
#endif
#endif
#endif
#endif
				}
			}

			crc = crc_word(crc0 ^ words[0]);
#if N > 1
			crc = crc_word(crc1 ^ words[1] ^ crc);
#if N > 2
			crc = crc_word(crc2 ^ words[2] ^ crc);
#if N > 3
			crc = crc_word(crc3 ^ words[3] ^ crc);
#if N > 4
			crc = crc_word(crc4 ^ words[4] ^ crc);
#if N > 5
			crc = crc_word(crc5 ^ words[5] ^ crc);
#endif
#endif
#endif
#endif
#endif
			words += N;
		}
		else {
			z_word_t crc0, word0, comb;
#if N > 1
			z_word_t crc1, word1;
#if N > 2
			z_word_t crc2, word2;
#if N > 3
			z_word_t crc3, word3;
#if N > 4
			z_word_t crc4, word4;
#if N > 5
			z_word_t crc5, word5;
#endif
#endif
#endif
#endif
#endif

			crc0 = byte_swap(crc);
#if N > 1
			crc1 = 0;
#if N > 2
			crc2 = 0;
#if N > 3
			crc3 = 0;
#if N > 4
			crc4 = 0;
#if N > 5
			crc5 = 0;
#endif
#endif
#endif
#endif
#endif

			while (--blks) {
				word0 = crc0 ^ words[0];
#if N > 1
				word1 = crc1 ^ words[1];
#if N > 2
				word2 = crc2 ^ words[2];
#if N > 3
				word3 = crc3 ^ words[3];
#if N > 4
				word4 = crc4 ^ words[4];
#if N > 5
				word5 = crc5 ^ words[5];
#endif
#endif
#endif
#endif
#endif
				words += N;

				crc0 = crc_braid_big_table[0][word0 & 0xff];
#if N > 1
				crc1 = crc_braid_big_table[0][word1 & 0xff];
#if N > 2
				crc2 = crc_braid_big_table[0][word2 & 0xff];
#if N > 3
				crc3 = crc_braid_big_table[0][word3 & 0xff];
#if N > 4
				crc4 = crc_braid_big_table[0][word4 & 0xff];
#if N > 5
				crc5 = crc_braid_big_table[0][word5 & 0xff];
#endif
#endif
#endif
#endif
#endif
				for (k = 1; k < W; k++) {
					crc0 ^= crc_braid_big_table[k][(word0 >> (k << 3)) & 0xff];
#if N > 1
					crc1 ^= crc_braid_big_table[k][(word1 >> (k << 3)) & 0xff];
#if N > 2
					crc2 ^= crc_braid_big_table[k][(word2 >> (k << 3)) & 0xff];
#if N > 3
					crc3 ^= crc_braid_big_table[k][(word3 >> (k << 3)) & 0xff];
#if N > 4
					crc4 ^= crc_braid_big_table[k][(word4 >> (k << 3)) & 0xff];
#if N > 5
					crc5 ^= crc_braid_big_table[k][(word5 >> (k << 3)) & 0xff];
#endif
#endif
#endif
#endif
#endif
				}
			}

			comb = crc_word_big(crc0 ^ words[0]);
#if N > 1
			comb = crc_word_big(crc1 ^ words[1] ^ comb);
#if N > 2
			comb = crc_word_big(crc2 ^ words[2] ^ comb);
#if N > 3
			comb = crc_word_big(crc3 ^ words[3] ^ comb);
#if N > 4
			comb = crc_word_big(crc4 ^ words[4] ^ comb);
#if N > 5
			comb = crc_word_big(crc5 ^ words[5] ^ comb);
#endif
#endif
#endif
#endif
#endif
			words += N;
			crc = byte_swap(comb);
		}

		buf = (unsigned char const*)words;
	}

#endif

	while (len >= 8) {
		len -= 8;
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
	}
	while (len) {
		len--;
		crc = (crc >> 8) ^ crc_table[(crc ^ *buf++) & 0xff];
	}

	return crc ^ 0xffffffff;
}

unsigned long  crc32(unsigned long crc, const unsigned char FAR* buf,
	uInt len) {
	return crc32_z(crc, buf, len);
}

uLong  crc32_combine64(uLong crc1, uLong crc2, z_off64_t len2) {
#ifdef DYNAMIC_CRC_TABLE
	once(&made, make_crc_table);
#endif
	return multmodp(x2nmodp(len2, 3), crc1) ^ (crc2 & 0xffffffff);
}

uLong  crc32_combine(uLong crc1, uLong crc2, z_off_t len2) {
	return crc32_combine64(crc1, crc2, (z_off64_t)len2);
}

uLong  crc32_combine_gen64(z_off64_t len2) {
#ifdef DYNAMIC_CRC_TABLE
	once(&made, make_crc_table);
#endif
	return x2nmodp(len2, 3);
}

uLong  crc32_combine_gen(z_off_t len2) {
	return crc32_combine_gen64((z_off64_t)len2);
}

uLong  crc32_combine_op(uLong crc1, uLong crc2, uLong op) {
	return multmodp(op, crc1) ^ (crc2 & 0xffffffff);
}