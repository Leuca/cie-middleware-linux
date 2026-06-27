//
//  CIEEngine.h
//  CIESDK
//
//  Created by ugo chirico on 26.02.2020.
//  Updated for OpenSSL 3.x Provider support.
//

#ifndef CIEEngine_h
#define CIEEngine_h

#include <stdio.h>
#include <openssl/crypto.h>
#include <openssl/objects.h>
#include <openssl/opensslv.h>

#if OPENSSL_VERSION_NUMBER >= 0x30000000
# include <openssl/provider.h>
# include <openssl/evp.h>
# include "CIEProvider.h"
#else
# include <openssl/engine.h>
#endif

typedef short (*sign)(unsigned char* tosign, size_t len, unsigned char* signature, size_t* psiglen);

#if OPENSSL_VERSION_NUMBER >= 0x30000000
/* Provider-based API (OpenSSL >= 3.0) */
void engine_load_cie(sign sign_cb);  /* compatibility wrapper → provider_load_cie */
#else
/* Legacy engine-based API (OpenSSL < 3.0) */
void engine_load_cie(sign sign_cb);
#endif

#endif /* CIEEngine_h */
