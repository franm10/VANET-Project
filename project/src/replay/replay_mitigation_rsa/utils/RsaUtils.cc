//  RsaUtils.cc  ─────────────────────────────────────────────────────
//  Implementazione basata su OpenSSL (libssl, libcrypto)
//  Compilare con:  -lssl -lcrypto
//-------------------------------------------------------------------
#include "RsaUtils.h"

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdexcept>
#include <sstream>
#include <vector>

/* helper: SHA-256(data) → digest[32] */
static void sha256(const std::string& data,
                   unsigned char digest[SHA256_DIGEST_LENGTH])
{
    SHA256(reinterpret_cast<const unsigned char*>(data.data()),
           data.size(), digest);
}

/* helper: Base64 encode (OpenSSL) */
static std::string base64Encode(const unsigned char* buf, size_t len)
{
    /* EVP_EncodeBlock output = 4 * ceil(len/3)  (+1 for '\0') */
    std::string out;
    out.resize(4 * ((len + 2) / 3));
    int n = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]),
                            buf, len);
    out.resize(n);
    return out;
}

/* helper: Base64 decode → vector<unsigned char> */
static std::vector<unsigned char> base64Decode(const std::string& b64)
{
    std::vector<unsigned char> out( b64.length()*3/4 + 1 );
    int n = EVP_DecodeBlock(out.data(),
                            reinterpret_cast<const unsigned char*>(b64.data()),
                            b64.length());
    if (n < 0)
        throw std::runtime_error("Base64 decode error");
    out.resize(n);
    return out;
}

/*─────────────────────────────────────────────────────────*
 *  Firmare: SHA-256 → RSA_sign → Base64                   *
 *─────────────────────────────────────────────────────────*/
std::string RsaUtils::signPayload(const std::string& payload,
                                  const std::string& privPem)
{
    /* 1. Carica la chiave privata da stringa PEM */
    BIO* bio = BIO_new_mem_buf(privPem.data(), privPem.size());
    if (!bio) throw std::runtime_error( ERR_error_string(ERR_get_error(), nullptr) );


    RSA* rsa = PEM_read_bio_RSAPrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!rsa) throw std::runtime_error("PEM_read_bio_RSAPrivateKey failed");

    /* 2. Calcola hash SHA-256 del payload */
    unsigned char hash[SHA256_DIGEST_LENGTH];
    sha256(payload, hash);

    /* 3. Firma l'hash */
    unsigned char sig[256];         // 2048 bit = 256 B
    unsigned int  sigLen = 0;
    if (!RSA_sign(NID_sha256, hash, sizeof(hash), sig, &sigLen, rsa)) {
        RSA_free(rsa);
        throw std::runtime_error("RSA_sign failed: "
               + std::string(ERR_error_string(ERR_get_error(), nullptr)));
    }
    RSA_free(rsa);

    /* 4. Codifica la firma in Base64 e restituisci */
    return base64Encode(sig, sigLen);
}

/*─────────────────────────────────────────────────────────*
 *  Verifica: Base64 → RSA_verify                          *
 *─────────────────────────────────────────────────────────*/
bool RsaUtils::verifySignature(const std::string& payload,
                               const std::string& signature,
                               const std::string& pubPem)
{
    if (pubPem.empty()) return false;

    /* 1. firma: Base-64  →  vettore binario (256 B) */
    std::vector<unsigned char> sigVec = base64Decode(signature);
    const unsigned char *sigBin = sigVec.data();
    size_t sigLen               = sigVec.size();   // 256 con RSA-2048

    /* 2. Carica la chiave pubblica PEM */
    BIO* bio = BIO_new_mem_buf(pubPem.data(), pubPem.size());
    if (!bio) return false;

    RSA* rsa = PEM_read_bio_RSA_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!rsa) return false;
    size_t modLen = RSA_size(rsa);        // 256 B con chiave 2048 bit
    if (sigLen > modLen)                  // taglia eventuali pad 0x00
        sigLen = modLen;

    /* 3. Calcola hash SHA-256 del payload */
    unsigned char hash[SHA256_DIGEST_LENGTH];
    sha256(payload, hash);

    /* 4. Verifica firma */
    int ok = RSA_verify(NID_sha256,
                        hash, sizeof(hash),
                        sigBin, sigLen,
                        rsa);
    RSA_free(rsa);
    return ok == 1;
}
