//
//  CIEProvider.h
//  CIESDK
//
//  OpenSSL 3.x Provider for Italian CIE (Carta d'Identità Elettronica)
//  Replaces the deprecated ENGINE-based CIEEngine implementation.
//
//  Usage:
//    1. Set cie_certificate / cie_certlen globals with the DER-encoded
//       X.509 certificate read from the CIE card.
//    2. Call provider_load_cie(sign_cb) to load the provider.
//    3. Use cie_provider_get_pkey() to obtain an EVP_PKEY that routes
//       RSA signing to the smart card callback.
//    4. Use standard OpenSSL EVP_PKEY_sign() / EVP_DigestSign() APIs.
//    5. Call provider_unload_cie() when done.
//

#ifndef CIEProvider_h
#define CIEProvider_h

#include <stdio.h>
#include <openssl/provider.h>
#include <openssl/evp.h>

// ---------------------------------------------------------------------------
// Global variables — must be set by the caller before provider_load_cie()
// ---------------------------------------------------------------------------

/** DER-encoded X.509 certificate read from the CIE card. */
extern const unsigned char* cie_certificate;

/** Length in bytes of cie_certificate. */
extern unsigned long cie_certlen;

/** PIN (optional, can be NULL). */
extern unsigned char* cie_pin;

/** PIN length. */
extern unsigned long cie_pinlen;

/** Last error code from the card (0 = success). */
extern unsigned short cie_error;

// ---------------------------------------------------------------------------
// Signing callback type
// ---------------------------------------------------------------------------

/**
 * Callback invoked to perform an RSA signature on the smart card.
 *
 * @param tosign       DigestInfo structure to sign.
 * @param len          Length of tosign in bytes.
 * @param signature    Output buffer for the signature.
 * @param psiglen      [in/out] On input: size of signature buffer.
 *                     On output: actual signature length.
 * @return             0x9000 on success, error code otherwise.
 */
typedef short (*sign_cb)(unsigned char* tosign, size_t len,
                         unsigned char* signature, size_t* psiglen);

// ---------------------------------------------------------------------------
// Provider management
// ---------------------------------------------------------------------------

/**
 * Load the CIE provider and register it with OpenSSL.
 *
 * @param sign_cb  Callback for RSA signing operations on the smart card.
 * @return         Provider handle, or NULL on failure.
 */
OSSL_PROVIDER *provider_load_cie(sign_cb sign_cb);

/**
 * Unload the CIE provider.
 *
 * @param prov  Provider handle returned by provider_load_cie().
 */
void provider_unload_cie(OSSL_PROVIDER *prov);

// ---------------------------------------------------------------------------
// Key creation
// ---------------------------------------------------------------------------

/**
 * Create an EVP_PKEY backed by the CIE provider.
 *
 * The returned key contains the RSA public key extracted from the CIE
 * certificate. When used with EVP_PKEY_sign() / EVP_DigestSign(),
 * the signing operation is delegated to the smart card via the callback
 * provided to provider_load_cie().
 *
 * Only RSA_PKCS1_PADDING is supported (CIE hardware limitation).
 *
 * @param prov  Provider handle returned by provider_load_cie().
 * @return      EVP_PKEY (caller must free with EVP_PKEY_free()), or NULL.
 */
EVP_PKEY *cie_provider_get_pkey(OSSL_PROVIDER *prov);

#endif /* CIEProvider_h */
