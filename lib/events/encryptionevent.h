// SPDX-FileCopyrightText: 2017 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-FileCopyrightText: 2019 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "stateevent.h"
#include "quotient_common.h"

namespace Quotient {
class QUOTIENT_API EncryptionEventContent {
public:
    enum EncryptionType : size_t { MegolmV1AesSha2 = 0, Undefined };

    QUO_IMPLICIT EncryptionEventContent(EncryptionType et);
    explicit EncryptionEventContent(const QJsonObject& json);

    QJsonObject toJson() const;

    EncryptionType encryption;
    QString algorithm;
    int rotationPeriodMs;
    int rotationPeriodMsgs;
};

using EncryptionType = EncryptionEventContent::EncryptionType;

class QUOTIENT_API EncryptionEvent
    : public KeylessStateEventBase<EncryptionEvent, EncryptionEventContent> {
    Q_GADGET
public:
    DEFINE_EVENT_TYPEID("m.room.encryption", EncryptionEvent)

    using EncryptionType = EncryptionEventContent::EncryptionType;
    Q_ENUM(EncryptionType)

    using KeylessStateEventBase::KeylessStateEventBase;

    EncryptionType encryption() const { return content().encryption; }
    QString algorithm() const { return content().algorithm; }
    int rotationPeriodMs() const { return content().rotationPeriodMs; }
    int rotationPeriodMsgs() const { return content().rotationPeriodMsgs; }
};
} // namespace Quotient
