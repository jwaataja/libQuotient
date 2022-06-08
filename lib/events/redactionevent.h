// SPDX-FileCopyrightText: 2017 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "roomevent.h"

namespace Quotient {
class RedactionEvent : public EventBase<RedactionEvent, RoomEvent> {
public:
    DEFINE_EVENT_TYPEID("m.room.redaction", RedactionEvent)

    using EventBase::EventBase;

    QString redactedEvent() const
    {
        return fullJson()["redacts"_ls].toString();
    }
    QUO_CONTENT_GETTER(QString, reason)
};
} // namespace Quotient
