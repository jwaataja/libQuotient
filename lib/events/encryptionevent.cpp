// SPDX-FileCopyrightText: 2017 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-FileCopyrightText: 2019 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "encryptionevent.h"

#include "e2ee/e2ee.h"

namespace Quotient {
static constexpr std::array encryptionStrings { MegolmV1AesSha2AlgoKey };

template <>
struct JsonConverter<EncryptionType> {
    static EncryptionType load(const QJsonValue& jv)
    {
        const auto& encryptionString = jv.toString();
        for (auto it = encryptionStrings.begin(); it != encryptionStrings.end();
             ++it)
            if (encryptionString == *it)
                return EncryptionType(it - encryptionStrings.begin());

        if (!encryptionString.isEmpty())
            qCWarning(EVENTS) << "Unknown EncryptionType: " << encryptionString;
        return EncryptionType::Undefined;
    }
};
} // namespace Quotient

using namespace Quotient;

EncryptionEventContent::EncryptionEventContent(const QJsonObject& json)
    : encryption(fromJson<EncryptionType>(json[AlgorithmKeyL]))
    , algorithm(sanitized(json[AlgorithmKeyL].toString()))
    , rotationPeriodMs(json[RotationPeriodMsKeyL].toInt(604800000))
    , rotationPeriodMsgs(json[RotationPeriodMsgsKeyL].toInt(100))
{}

EncryptionEventContent::EncryptionEventContent(EncryptionType et)
    : encryption(et)
{
    if(encryption != Undefined) {
        algorithm = encryptionStrings[encryption];
    }
}

QJsonObject EncryptionEventContent::toJson() const
{
    QJsonObject o;
    if (encryption != EncryptionType::Undefined)
        o.insert(AlgorithmKey, algorithm);
    o.insert(RotationPeriodMsKey, rotationPeriodMs);
    o.insert(RotationPeriodMsgsKey, rotationPeriodMsgs);
    return o;
}
