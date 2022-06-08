// SPDX-FileCopyrightText: 2018 Kitsune Ral <kitsune-ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "roomevent.h"

namespace Quotient {

class QUOTIENT_API StateEventBase : public RoomEvent {
public:
    static inline EventFactory<StateEventBase> factory { "StateEvent" };

    StateEventBase(Type type, const QJsonObject& json);
    StateEventBase(Type type, const QString& stateKey = {},
                   const QJsonObject& contentJson = {});
    ~StateEventBase() override = default;

    //! Make a minimal correct Matrix state event JSON
    static QJsonObject basicJson(const QString& matrixTypeId,
                                 const QJsonObject& content,
                                 const QString& stateKey = {})
    {
        return { { TypeKey, matrixTypeId },
                 { StateKeyKey, stateKey },
                 { ContentKey, content } };
    }

    bool isStateEvent() const override { return true; }
    QString replacedState() const;
    void dumpTo(QDebug dbg) const override;

    virtual bool repeatsState() const;
};
using StateEventPtr = event_ptr_tt<StateEventBase>;
using StateEvents = EventsArray<StateEventBase>;

[[deprecated("Use StateEventBase::basicJson() instead")]]
inline QJsonObject basicStateEventJson(const QString& matrixTypeId,
                                       const QJsonObject& content,
                                       const QString& stateKey = {})
{
    return StateEventBase::basicJson(matrixTypeId, content, stateKey);
}

//! \brief Override RoomEvent factory with that from StateEventBase if JSON has
//! stateKey
//!
//! This means in particular that an event with a type known to RoomEvent but
//! having stateKey set (even to an empty value) will be treated as a state
//! event and most likely end up as unknown (consider, e.g., m.room.message
//! that has stateKey set).
template <>
inline RoomEventPtr doLoadEvent(const QJsonObject& json,
                                const QString& matrixType)
{
    if (json.contains(StateKeyKeyL))
        return StateEventBase::factory.loadEvent(json, matrixType);
    return RoomEvent::factory.loadEvent(json, matrixType);
}

template <>
inline bool is<StateEventBase>(const Event& e)
{
    return e.isStateEvent();
}

/**
 * A combination of event type and state key uniquely identifies a piece
 * of state in Matrix.
 * \sa
 * https://matrix.org/docs/spec/client_server/unstable.html#types-of-room-events
 */
using StateEventKey = std::pair<QString, QString>;

template <typename ContentT>
struct Prev {
    explicit Prev(const QJsonObject& unsignedJson)
        : senderId(unsignedJson.value("prev_sender"_ls).toString())
        , content(fromJson<ContentT>(unsignedJson.value(PrevContentKeyL)))
    {}

    QString senderId;
    ContentT content;
};

template <typename EventT, typename ContentT>
class EventBase<EventT, StateEventBase, ContentT>
    : public StateEventBase {
public:
    using content_type = ContentT;

    explicit EventBase(const QJsonObject& fullJson)
        : StateEventBase(Quotient::typeId<EventT>, fullJson)
        , _content(fromJson<ContentT>(Event::contentJson()))
    {
        const auto& unsignedData = unsignedJson();
        if (unsignedData.contains(PrevContentKeyL))
            _prev = std::make_unique<Prev<ContentT>>(unsignedData);
    }
    template <typename... ContentParamTs>
    explicit EventBase(const QString& stateKey,
                           ContentParamTs&&... contentParams)
        : StateEventBase(typeId<EventT>, stateKey)
        , _content { std::forward<ContentParamTs>(contentParams)... }
    {
        editJson().insert(ContentKey, toJson(_content));
    }

    const ContentT& content() const { return _content; }

    template <typename VisitorT>
    void editContent(VisitorT&& visitor)
    {
        visitor(_content);
        editJson()[ContentKeyL] = toJson(_content);
    }
    const ContentT* prevContent() const
    {
        return _prev ? &_prev->content : nullptr;
    }
    QString prevSenderId() const
    {
        return _prev ? _prev->senderId : QString();
    }

private:
    ContentT _content;
    std::unique_ptr<Prev<ContentT>> _prev;
};

template <typename EventT, typename ContentT>
class KeyedStateEventBase
    : public EventBase<EventT, StateEventBase, ContentT> {
public:
    using EventBase<EventT, StateEventBase, ContentT>::EventBase;
};

template <typename EventT, typename ContentT>
class KeylessStateEventBase
    : public EventBase<EventT, StateEventBase, ContentT> {
private:
    using base_type = EventBase<EventT, StateEventBase, ContentT>;

public:
    KeylessStateEventBase(const QJsonObject& fullJson)
        : base_type(fullJson)
    {}
    template <typename... ContentParamTs>
    KeylessStateEventBase(ContentParamTs&&... contentParams)
        : base_type(QString(), std::forward<ContentParamTs>(contentParams)...)
    {}
};

} // namespace Quotient
Q_DECLARE_METATYPE(Quotient::StateEventBase*)
Q_DECLARE_METATYPE(const Quotient::StateEventBase*)
