#ifndef __ECDSA_UTILS_H_
#define __ECDSA_UTILS_H_

#include <string>
#include <vector>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bio.h>

class EcdsaUtils {
public:
    // Firma payload con chiave privata ECDSA P-256
    static std::string signPayload(const std::string& payload,
                                   const std::string& privKeyPem);

    // Verifica firma ECDSA
    static bool verifySignature(const std::string& payload,
                               const std::string& signature,
                               const std::string& pubKeyPem);

private:
    // Carica chiave privata da PEM
    static EVP_PKEY* loadPrivateKey(const std::string& pem);

    // Carica chiave pubblica da PEM
    static EVP_PKEY* loadPublicKey(const std::string& pem);

    // Base64 encoding/decoding
    static std::string base64Encode(const unsigned char* data, size_t len);
    static std::vector<unsigned char> base64Decode(const std::string& encoded);
};

#endif
