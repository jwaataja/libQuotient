// SPDX-FileCopyrightText: 2017 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "stateevent.h"
#include "single_key_value.h"

namespace Quotient {
#define DEFINE_SIMPLE_STATE_EVENT(Name_, TypeId_, ValueType_, ContentKey_)    \
    constexpr auto Name_##Key = #ContentKey_##_ls;                            \
    class QUOTIENT_API Name_                                                  \
        : public KeylessStateEventBase<                                       \
              Name_, EventContent::SingleKeyValue<ValueType_, &Name_##Key>> { \
    public:                                                                   \
        using value_type = ValueType_;                                        \
        DEFINE_EVENT_TYPEID(TypeId_, Name_)                                   \
        using KeylessStateEventBase::KeylessStateEventBase;                   \
        auto ContentKey_() const { return content().value; }                  \
    };                                                                        \
// End of macro

DEFINE_SIMPLE_STATE_EVENT(RoomNameEvent, "m.room.name", QString, name)
DEFINE_SIMPLE_STATE_EVENT(RoomTopicEvent, "m.room.topic", QString, topic)
DEFINE_SIMPLE_STATE_EVENT(RoomPinnedEvent, "m.room.pinned_messages",
                          QStringList, pinnedEvents)

constexpr auto RoomAliasesEventKey = "aliases"_ls;
class [[deprecated(
    "m.room.aliases events are no more recognised by the Matrix spec; use"
    " RoomCanonicalAliasEvent::altAliases() to get non-authoritative aliases")]] //
RoomAliasesEvent
    : public KeyedStateEventBase<
          RoomAliasesEvent,
          EventContent::SingleKeyValue<QStringList, &RoomAliasesEventKey>>
{
public:
    DEFINE_EVENT_TYPEID("m.room.aliases", RoomAliasesEvent)
    using KeyedStateEventBase::KeyedStateEventBase;

    QString server() const { return stateKey(); }
    QStringList aliases() const { return content().value; }
};
} // namespace Quotient
