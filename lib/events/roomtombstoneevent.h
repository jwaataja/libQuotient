// SPDX-FileCopyrightText: 2019 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "stateevent.h"

namespace Quotient {
class QUOTIENT_API RoomTombstoneEvent : public StateEvent {
public:
    DEFINE_EVENT_TYPEID("m.room.tombstone", RoomTombstoneEvent)

    RoomTombstoneEvent(const QJsonObject& json)
        : StateEvent(TypeId, json)
    {}

    QString serverMessage() const;
    QString successorRoomId() const;
};
} // namespace Quotient
