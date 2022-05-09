// SPDX-FileCopyrightText: 2020 Ram Nad <ramnad1999@gmail.com>
// SPDX-FileCopyrightText: 2020 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later
 
#pragma once

#include "stateevent.h"

namespace Quotient {
namespace EventContent {
    struct AliasesEventContent {
        QString canonicalAlias;
        QStringList altAliases;
    };
} // namespace EventContent

template <>
struct JsonConverter<EventContent::AliasesEventContent> {
    static auto load(const QJsonValue& jv)
    {
        const auto jo = jv.toObject();
        return EventContent::AliasesEventContent {
            fromJson<QString>(jo["alias"_ls]),
            fromJson<QStringList>(jo["alt_aliases"_ls])
        };
    }
    static auto dump(const EventContent::AliasesEventContent& c)
    {
        QJsonObject jo;
        addParam<IfNotEmpty>(jo, QStringLiteral("alias"), c.canonicalAlias);
        addParam<IfNotEmpty>(jo, QStringLiteral("alt_aliases"), c.altAliases);
        return jo;
    }
};

class RoomCanonicalAliasEvent
    : public KeylessStateEventBase<RoomCanonicalAliasEvent,
                                   EventContent::AliasesEventContent> {
public:
    DEFINE_EVENT_TYPEID("m.room.canonical_alias", RoomCanonicalAliasEvent)
    using KeylessStateEventBase::KeylessStateEventBase;

    QString alias() const { return content().canonicalAlias; }
    QStringList altAliases() const { return content().altAliases; }
};
} // namespace Quotient
