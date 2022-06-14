// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "e2ee/qolmutility.h"

#include <olm/olm.h>

using namespace Quotient;

// Convert olm error to enum
QOlmError lastError(OlmUtility *utility) {
    return fromString(olm_utility_last_error(utility));
}

QOlmUtility::QOlmUtility()
{
    auto utility = new uint8_t[olm_utility_size()];
    m_utility = olm_utility(utility);
}

QOlmUtility::~QOlmUtility()
{
    olm_clear_utility(m_utility);
    delete[](reinterpret_cast<uint8_t *>(m_utility));
}

QOlmExpected<bool> QOlmUtility::ed25519Verify(const QByteArray& key,
                                              const QByteArray& message,
                                              const QByteArray& signature)
{
    QByteArray signatureBuf(signature.length(), '0');
    std::copy(signature.begin(), signature.end(), signatureBuf.begin());

    const auto ret = olm_ed25519_verify(m_utility, key.data(), key.size(),
            message.data(), message.size(), (void *)signatureBuf.data(), signatureBuf.size());

    if (ret == olm_error()) {
        auto error = lastError(m_utility);
        if (error == QOlmError::BadMessageMac) {
            return false;
        }
        return error;
    }

    return !ret; // ret == 0 means success
}
