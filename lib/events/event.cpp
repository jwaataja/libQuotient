// SPDX-FileCopyrightText: 2016 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "event.h"

#include "logging.h"

#include <QtCore/QJsonDocument>

using namespace Quotient;

QString EventTypeRegistry::getMatrixType(event_type_t typeId) { return typeId; }

bool _impl::EventFactoryBase::isMethodRegistered(event_type_t matrixType,
                                                 const char* className,
                                                 const char* registeredClassName,
                                                 size_t currentSize) const
{
    if (Q_LIKELY(!registeredClassName)) {
        qDebug(EVENTS).nospace()
            << "Registering " << matrixType << " -> " << className << "; "
            << currentSize + 1 << " method(s) will be in the " << name
            << " chain";
        return false;
    }
    if (registeredClassName != className)
        qCritical(EVENTS) << "Attempt to register" << className << "for"
                          << matrixType << "events for which"
                          << registeredClassName << "is already registered";
    else
        qDebug(EVENTS) << "Already registered" << className << "for"
                       << matrixType;
    return true;
}

Event::Event(Type type, const QJsonObject& json)
    : _type(json[TypeKeyL] == type ? type : UnknownEventTypeId)
    , _json(json)
{
    if (!json.contains(ContentKeyL)
        && !json.value(UnsignedKeyL).toObject().contains(RedactedCauseKeyL)) {
        qCWarning(EVENTS) << "Event without 'content' node";
        qCWarning(EVENTS) << formatJson << json;
    }
}

Event::~Event() = default;

QString Event::matrixType() const { return fullJson()[TypeKeyL].toString(); }

QByteArray Event::originalJson() const { return QJsonDocument(_json).toJson(); }

const QJsonObject Event::contentJson() const
{
    return fullJson()[ContentKeyL].toObject();
}

const QJsonObject Event::unsignedJson() const
{
    return fullJson()[UnsignedKeyL].toObject();
}

void Event::dumpTo(QDebug dbg) const
{
    dbg << QJsonDocument(contentJson()).toJson(QJsonDocument::Compact);
}
