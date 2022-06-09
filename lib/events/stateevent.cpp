// SPDX-FileCopyrightText: 2018 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "stateevent.h"

using namespace Quotient;

StateEvent::StateEvent(Type type, const QJsonObject& json)
    : RoomEvent(json.contains(StateKeyKeyL) ? type : UnknownEventTypeId, json)
{
    if (Event::type() == UnknownEventTypeId && !json.contains(StateKeyKeyL))
        qWarning(EVENTS) << "Attempt to create a state event with no stateKey -"
                            "forcing the event type to unknown to avoid damage";
}

StateEvent::StateEvent(Event::Type type, const QString& stateKey,
                               const QJsonObject& contentJson)
    : RoomEvent(type, basicJson(type, contentJson, stateKey))
{}

bool StateEvent::repeatsState() const
{
    return contentJson() == unsignedPart<QJsonObject>(PrevContentKeyL);
}

QString StateEvent::replacedState() const
{
    return unsignedPart<QString>("replaces_state"_ls);
}

void StateEvent::dumpTo(QDebug dbg) const
{
    if (!stateKey().isEmpty())
        dbg << '<' << stateKey() << "> ";
    if (const auto prevContentJson = unsignedPart<QJsonObject>(PrevContentKeyL);
        !prevContentJson.isEmpty())
        dbg << QJsonDocument(prevContentJson).toJson(QJsonDocument::Compact)
            << " -> ";
    RoomEvent::dumpTo(dbg);
}
