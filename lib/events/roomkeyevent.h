// SPDX-FileCopyrightText: 2019 Alexey Andreyev <aa13q@ya.ru>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "event.h"

namespace Quotient {
class QUOTIENT_API RoomKeyEvent : public EventBase<RoomKeyEvent, Event>
{
public:
    DEFINE_EVENT_TYPEID("m.room_key", RoomKeyEvent)

    using EventBase::EventBase;
    explicit RoomKeyEvent(const QString& algorithm, const QString& roomId,
                          const QString& sessionId,
                          const QString& sessionKey)
        : EventBase(Event::basicJson(TypeId, {
                                                 { "algorithm", algorithm },
                                                 { "room_id", roomId },
                                                 { "session_id", sessionId },
                                                 { "session_key", sessionKey },
                                             }))
    {}

    QUO_CONTENT_GETTER(QString, algorithm)
    QUO_CONTENT_GETTER(QString, roomId)
    QUO_CONTENT_GETTER(QString, sessionId)
    QByteArray sessionKey() const
    {
        return contentPart<QString>("session_key"_ls).toLatin1();
    }
};
} // namespace Quotient
