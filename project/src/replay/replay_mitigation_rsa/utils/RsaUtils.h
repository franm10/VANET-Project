//  RsaUtils.h  ──────────────────────────────────────────────────────
//  • Interfaccia minima per firmare/verificare con RSA-2048 + SHA-256
//  • Implementazione in RsaUtils.cc usa OpenSSL
//--------------------------------------------------------------------
#ifndef __UTILS_RSAUTILS_H_
#define __UTILS_RSAUTILS_H_

#include <string>

namespace RsaUtils {

/**
 * Firma 'payload' con la chiave privata (PEM in memoria)
 *
 * @param payload        dati da firmare (qualunque string)
 * @param privateKeyPem  stringa che CONTIENE l'intero file PEM
 * @return               firma codificata Base64
 */
std::string signPayload(const std::string& payload,
                        const std::string& privateKeyPem);

/**
 * Verifica la firma Base64 di 'payload' con la chiave pubblica PEM
 *
 * @param payload        dati originali
 * @param signatureB64   firma in Base64
 * @param publicKeyPem   chiave pubblica del mittente (PEM)
 * @return               true se la firma è valida
 */
bool verifySignature(const std::string& payload,
                     const std::string& signatureB64,
                     const std::string& publicKeyPem);

}   // namespace RsaUtils
#endif
