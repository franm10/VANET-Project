#include "EcdsaUtils.h"
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <iomanip>

// ============================================================================
// PRIVATE: Caricamento chiavi
// ============================================================================

EVP_PKEY* EcdsaUtils::loadPrivateKey(const std::string& pem) {
    BIO* bio = BIO_new_mem_buf(pem.c_str(), pem.size());
    if (!bio) return nullptr;

    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    return pkey;
}

EVP_PKEY* EcdsaUtils::loadPublicKey(const std::string& pem) {
    BIO* bio = BIO_new_mem_buf(pem.c_str(), pem.size());
    if (!bio) return nullptr;

    EVP_PKEY* pkey = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    return pkey;
}

// ============================================================================
// PRIVATE: Base64 encoding/decoding
// ============================================================================

std::string EcdsaUtils::base64Encode(const unsigned char* data, size_t len) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);

    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, data, len);
    BIO_flush(b64);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);

    std::string result(bptr->data, bptr->length);
    BIO_free_all(b64);

    return result;
}

std::vector<unsigned char> EcdsaUtils::base64Decode(const std::string& encoded) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(encoded.c_str(), encoded.size());
    bmem = BIO_push(b64, bmem);

    BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);

    std::vector<unsigned char> result(encoded.size());
    int len = BIO_read(bmem, result.data(), encoded.size());

    BIO_free_all(bmem);

    if (len > 0) {
        result.resize(len);
    } else {
        result.clear();
    }

    return result;
}

// ============================================================================
// PUBLIC: Firma
// ============================================================================

std::string EcdsaUtils::signPayload(const std::string& payload,
                                    const std::string& privKeyPem) {
    // Carica chiave privata
    EVP_PKEY* pkey = loadPrivateKey(privKeyPem);
    if (!pkey) {
        throw std::runtime_error("Failed to load ECDSA private key");
    }

    // Crea contesto firma
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to create MD context");
    }

    // Inizializza firma con SHA-256
    if (EVP_DigestSignInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to init digest sign");
    }

    // Aggiungi payload
    if (EVP_DigestSignUpdate(mdctx, payload.c_str(), payload.size()) != 1) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to update digest");
    }

    // Ottieni lunghezza firma
    size_t sigLen;
    if (EVP_DigestSignFinal(mdctx, nullptr, &sigLen) != 1) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to get signature length");
    }

    // Genera firma
    unsigned char* sig = new unsigned char[sigLen];
    if (EVP_DigestSignFinal(mdctx, sig, &sigLen) != 1) {
        delete[] sig;
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("Failed to finalize signature");
    }

    // Encode in base64
    std::string result = base64Encode(sig, sigLen);

    // Cleanup
    delete[] sig;
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    return result;
}

// ============================================================================
// PUBLIC: Verifica
// ============================================================================

bool EcdsaUtils::verifySignature(const std::string& payload,
                                const std::string& signature,
                                const std::string& pubKeyPem) {
    // Carica chiave pubblica
    EVP_PKEY* pkey = loadPublicKey(pubKeyPem);
    if (!pkey) {
        return false;
    }

    // Decode firma da base64
    std::vector<unsigned char> sig = base64Decode(signature);
    if (sig.empty()) {
        EVP_PKEY_free(pkey);
        return false;
    }

    // Crea contesto verifica
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        EVP_PKEY_free(pkey);
        return false;
    }

    // Inizializza verifica con SHA-256
    if (EVP_DigestVerifyInit(mdctx, nullptr, EVP_sha256(), nullptr, pkey) != 1) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    // Aggiungi payload
    if (EVP_DigestVerifyUpdate(mdctx, payload.c_str(), payload.size()) != 1) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        return false;
    }

    // Verifica
    int result = EVP_DigestVerifyFinal(mdctx, sig.data(), sig.size());

    // Cleanup
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    return (result == 1);
}
