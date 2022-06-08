// SPDX-FileCopyrightText: 2020 Ram Nad <ramnad1999@gmail.com>
// SPDX-FileCopyrightText: 2020 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later
 
#pragma once

#include "stateevent.h"

namespace Quotient {
namespace EventContent{
    class AliasesEventContent {

    public:

        template<typename T1, typename T2>
        AliasesEventContent(T1&& canonicalAlias, T2&& altAliases)
            : canonicalAlias(std::forward<T1>(canonicalAlias))
            , altAliases(std::forward<T2>(altAliases))
        { }

        AliasesEventContent(const QJsonObject& json)
            : canonicalAlias(fromJson<QString>(json["alias"]))
            , altAliases(fromJson<QStringList>(json["alt_aliases"]))
        { }

        auto toJson() const
        {
            QJsonObject jo;
            addParam<IfNotEmpty>(jo, QStringLiteral("alias"), canonicalAlias);
            addParam<IfNotEmpty>(jo, QStringLiteral("alt_aliases"), altAliases);
            return jo;
        }

        QString canonicalAlias;
        QStringList altAliases;
    };
} // namespace EventContent

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
