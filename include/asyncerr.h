/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_ASYNCERR_H
# define OPENSSL_ASYNCERR_H
# pragma once

# include <openssl/opensslconf.h>
# include <openssl/symhacks.h>
# include <openssl/cryptoerr_legacy.h>

 /*
  * ASYNC reason codes.
  */
# define ASYNC_R_FAILED_TO_SET_POOL                       101
# define ASYNC_R_FAILED_TO_SWAP_CONTEXT                   102
# define ASYNC_R_INIT_FAILED                              105
# define ASYNC_R_INVALID_POOL_SIZE                        103

#endif
