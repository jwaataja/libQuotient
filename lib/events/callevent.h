// SPDX-FileCopyrightText: 2022 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "roomevent.h"

namespace Quotient {

class QUOTIENT_API CallEvent : public RoomEvent {
public:
    CallEvent(Type type, const QJsonObject& json);
    ~CallEvent() override = default;
    bool isCallEvent() const override { return true; }

    QUO_CONTENT_GETTER(QString, callId)
    QUO_CONTENT_GETTER(int, version)

protected:
    static QJsonObject basicJson(const QString& matrixType,
                                 const QString& callId, int version,
                                 QJsonObject contentJson = {});
};
using CallEventBase
    [[deprecated("CallEventBase is CallEvent now")]] = CallEvent;

template <typename EventT>
class EventBase<EventT, CallEvent> : public CallEvent {
public:
    explicit EventBase(const QJsonObject& json)
        : CallEvent(typeId<EventT>, json)
    {}
    explicit EventBase(const QString& callId,
                       const QJsonObject& contentJson = {})
        : EventBase(basicJson(typeId<EventT>, callId, 0, contentJson))
    {}
};

template <typename EventT, typename ContentT>
class EventBase<EventT, CallEvent, ContentT>
    : public EventBase<EventT, CallEvent> {
public:
    using EventBase<EventT, CallEvent>::EventBase;
    template <typename... ContentParamTs>
    explicit EventBase(const QString& callId, ContentParamTs&&... contentParams)
        : EventBase<EventT, CallEvent>(
            callId,
            toJson(ContentT { std::forward<ContentParamTs>(contentParams)... }))
    {}
};

class QUOTIENT_API CallInviteEvent
    : public EventBase<CallInviteEvent, CallEvent> {
public:
    DEFINE_EVENT_TYPEID("m.call.invite", CallInviteEvent)

    using EventBase::EventBase;

    explicit CallInviteEvent(const QString& callId, int lifetime,
                             const QString& sdp);

    QUO_CONTENT_GETTER(int, lifetime)
    QString sdp() const
    {
        return contentPart<QJsonObject>("offer"_ls).value("sdp"_ls).toString();
    }
};

DEFINE_SIMPLE_EVENT(CallCandidatesEvent, CallEvent, "m.call.candidates",
                    QJsonArray, candidates, "candidates")

class QUOTIENT_API CallAnswerEvent
    : public EventBase<CallAnswerEvent, CallEvent> {
public:
    DEFINE_EVENT_TYPEID("m.call.answer", CallAnswerEvent)

    using EventBase::EventBase;

    explicit CallAnswerEvent(const QString& callId, const QString& sdp);

    QString sdp() const
    {
        return contentPart<QJsonObject>("answer"_ls).value("sdp"_ls).toString();
    }
};

class QUOTIENT_API CallHangupEvent
    : public EventBase<CallHangupEvent, CallEvent> {
public:
    DEFINE_EVENT_TYPEID("m.call.hangup", CallHangupEvent)
    using EventBase::EventBase;
};

} // namespace Quotient
Q_DECLARE_METATYPE(Quotient::CallEvent*)
Q_DECLARE_METATYPE(const Quotient::CallEvent*)
