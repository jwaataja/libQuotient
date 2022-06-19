// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "e2ee/qolmutility.h"

#include <vodozemac/src/lib.rs.h>
#include <openssl/sha.h>
#include <olm/olm.h>

using namespace Quotient;

rust::Slice<const uint8_t> byteArrToByteSlice(const QByteArray& arr);

// Convert olm error to enum
QOlmError lastError(OlmUtility *utility) {
    return fromString(olm_utility_last_error(utility));
}

QString QOlmUtility::sha256Bytes(const QByteArray &inputBuf) const
{
    std::array<uint8_t, SHA256_DIGEST_LENGTH> outputBuf{};
    SHA256(reinterpret_cast<const uint8_t *>(inputBuf.data()), inputBuf.length(), outputBuf.data());
    return QString::fromLocal8Bit(reinterpret_cast<const char *>(outputBuf.data()), outputBuf.size());
}

QString QOlmUtility::sha256Utf8Msg(const QString &message) const
{
    return sha256Bytes(message.toUtf8());
}

QOlmExpected<bool> QOlmUtility::ed25519Verify(const QByteArray& key,
                                              const QByteArray& message,
                                              const QByteArray& signature)
{

    try {
        auto edKey = types::ed25519_key_from_base64(key.data());
        edKey->verify(byteArrToByteSlice(message), *types::ed25519_signature_from_base64(signature.data()));
        return true;
    } catch (const std::exception&) {
        return false;
    }
}
