// SPDX-FileCopyrightText: 2016 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "converters.h"
#include "logging.h"
#include "function_traits.h"
#include "single_key_value.h"

namespace Quotient {
// === event_ptr_tt<> and type casting facilities ===

template <typename EventT>
using event_ptr_tt = std::unique_ptr<EventT>;

template <typename EventT>
[[deprecated("Use std::to_address() instead")]]
inline EventT* rawPtr(const event_ptr_tt<EventT>& ptr)
{
    return ptr.get();
}

/// Unwrap a plain pointer and downcast it to the specified type
template <typename TargetEventT, typename EventT>
inline TargetEventT* weakPtrCast(const event_ptr_tt<EventT>& ptr)
{
    return static_cast<TargetEventT*>(std::to_address(ptr));
}

// === Standard Matrix key names and basicEventJson() ===

constexpr auto TypeKeyL = "type"_ls;
constexpr auto BodyKeyL = "body"_ls;
constexpr auto ContentKeyL = "content"_ls;
constexpr auto EventIdKeyL = "event_id"_ls;
constexpr auto SenderKeyL = "sender"_ls;
constexpr auto RoomIdKeyL = "room_id"_ls;
constexpr auto UnsignedKeyL = "unsigned"_ls;
constexpr auto RedactedCauseKeyL = "redacted_because"_ls;
constexpr auto PrevContentKeyL = "prev_content"_ls;
constexpr auto StateKeyKeyL = "state_key"_ls;
const QString TypeKey { TypeKeyL };
const QString BodyKey { BodyKeyL };
const QString ContentKey { ContentKeyL };
const QString EventIdKey { EventIdKeyL };
const QString SenderKey { SenderKeyL };
const QString RoomIdKey { RoomIdKeyL };
const QString UnsignedKey { UnsignedKeyL };
const QString StateKeyKey { StateKeyKeyL };

// === Event types ===

using event_type_t = QLatin1String;
using event_mtype_t = const char*;

class QUOTIENT_API EventTypeRegistry {
public:
    ~EventTypeRegistry() = default;

    [[deprecated("event_type_t is a string now, use it directly instead")]]
    static QString getMatrixType(event_type_t typeId);

private:
    EventTypeRegistry() = default;
    Q_DISABLE_COPY_MOVE(EventTypeRegistry)
};

template <typename EventT>
inline const event_type_t typeId = std::decay_t<EventT>::TypeId;

constexpr event_type_t UnknownEventTypeId = "?"_ls;
[[deprecated("Use UnknownEventTypeId")]]
constexpr event_type_t unknownEventTypeId() { return UnknownEventTypeId; }

// === Event creation facilities ===

//! Create an event of arbitrary type from its arguments
template <typename EventT, typename... ArgTs>
inline event_ptr_tt<EventT> makeEvent(ArgTs&&... args)
{
    return std::make_unique<EventT>(std::forward<ArgTs>(args)...);
}

namespace _impl {
    class QUOTIENT_API EventFactoryBase {
    public:
        EventFactoryBase(const EventFactoryBase&) = delete;

    protected: // This class is only to inherit from
        explicit EventFactoryBase(const char* name)
            : name(name)
        {}
        bool isMethodRegistered(event_type_t matrixType, const char* className,
                                const char* registeredClassName,
                                size_t currentSize) const;

    private:
        const char* const name;
    };
} // namespace _impl

template <class EventT, class BaseEventT>
event_ptr_tt<BaseEventT> makeIfMatches(const QJsonObject& json,
                                       const QString& matrixType)
{
    // If your matrix event type is not all ASCII, it's your problem
    // (see https://github.com/matrix-org/matrix-doc/pull/2758)
    return typeId<EventT> == matrixType ? makeEvent<EventT>(json) : nullptr;
}

//! \brief A family of event factories to create events from CS API responses
//!
//! Each of these factories, as instantiated by event base types (Event,
//! RoomEvent etc.) is capable of producing an event object derived from
//! \p BaseEventT, using the JSON payload and the event type passed to its
//! make() method. Don't use these directly to make events; use loadEvent()
//! overloads as the frontend for these. Never instantiate new factories
//! outside of base event classes.
//! \sa loadEvent, setupFactory, Event::factory, RoomEvent::factory,
//!     StateEventBase::factory
template <typename BaseEventT>
class EventFactory : public _impl::EventFactoryBase {
private:
    using method_t = event_ptr_tt<BaseEventT> (*)(const QJsonObject&,
                                                  const QString&);
    std::vector<method_t> methods {};

public:
    explicit EventFactory(const char* fName)
        : EventFactoryBase { fName }
    {}

    //! \brief Add a method to create events of a given type
    //!
    //! Adds a factory method \p m - usually it's a specialisation of
    //! makeIfMatches() - to create event objects of some type from loadEvent().
    //! The string passed in \p eventType is only used for logging; the factory
    //! method should already be wired to check for the specific event type,
    //! or delegate this checking to another factory.
    //! \note The caller is responsible for ensuring that addMethod is called
    //! only once per event type
    //! \sa makeIfMatches, loadEvent, Quotient::loadEvent
    auto tryAddMethod(method_t m, event_type_t matrixType,
                      const char* className, const char* registeredClassName)
    {
        if (Q_LIKELY(!isMethodRegistered(matrixType, className,
                                         registeredClassName, methods.size()))) {
            registeredClassName = className;
            methods.emplace_back(m);
        }
        return registeredClassName;
    }

    auto loadEvent(const QJsonObject& json, const QString& matrixType)
    {
        for (const auto& f : methods)
            if (auto e = f(json, matrixType))
                return e;
        return makeEvent<BaseEventT>(UnknownEventTypeId, json);
    }
};

//! \brief Point of customisation to dynamically load events
//!
//! The default specialisation of this calls BaseEventT::factory.loadEvent()
//! and if that fails (i.e. returns nullptr) creates an unknown event of
//! BaseEventT. Other specialisations may reuse other factories, add validations
//! common to BaseEventT events, and so on.
template <class BaseEventT>
event_ptr_tt<BaseEventT> doLoadEvent(const QJsonObject& json,
                                     const QString& matrixType)
{
    return BaseEventT::factory.loadEvent(json, matrixType);
}

// === Event ===

class QUOTIENT_API Event {
public:
    using Type = event_type_t;
    static inline EventFactory<Event> factory { "Event" };

    explicit Event(Type type, const QJsonObject& json);

    Q_DISABLE_COPY(Event)
    Event(Event&&) = default;
    Event& operator=(Event&&) = delete;
    virtual ~Event();

    /// Make a minimal correct Matrix event JSON
    static QJsonObject basicJson(const QString& matrixType,
                                 const QJsonObject& content)
    {
        return { { TypeKey, matrixType }, { ContentKey, content } };
    }

    Type type() const { return _type; }
    QString matrixType() const;
    [[deprecated("Use fullJson() and stringify it with QJsonDocument::toJson() "
                 "or by other means")]]
    QByteArray originalJson() const;
    [[deprecated("Use fullJson() instead")]] //
    QJsonObject originalJsonObject() const { return fullJson(); }

    const QJsonObject& fullJson() const { return _json; }

    // According to the CS API spec, every event also has
    // a "content" object; but since its structure is different for
    // different types, we're implementing it per-event type.

    // NB: const return types below are meant to catch accidental attempts
    //     to change event JSON (e.g., consider contentJson()["inexistentKey"]).

    const QJsonObject contentJson() const;

    template <typename T, typename KeyT>
    const T contentPart(KeyT&& key) const
    {
        return fromJson<T>(contentJson()[std::forward<KeyT>(key)]);
    }

    template <typename T>
    [[deprecated("Use contentPart() to get a part of the event content")]] //
    T content(const QString& key) const
    {
        return contentPart<T>(key);
    }

    const QJsonObject unsignedJson() const;

    template <typename T, typename KeyT>
    const T unsignedPart(KeyT&& key) const
    {
        return fromJson<T>(unsignedJson()[std::forward<KeyT>(key)]);
    }

    friend QUOTIENT_API QDebug operator<<(QDebug dbg, const Event& e)
    {
        QDebugStateSaver _dss { dbg };
        dbg.noquote().nospace() << e.matrixType() << '(' << e.type() << "): ";
        e.dumpTo(dbg);
        return dbg;
    }

    virtual bool isStateEvent() const { return false; }
    virtual bool isCallEvent() const { return false; }

protected:
    QJsonObject& editJson() { return _json; }
    virtual void dumpTo(QDebug dbg) const;

private:
    const Type _type;
    QJsonObject _json;
};
using EventPtr = event_ptr_tt<Event>;

template <typename EventT>
using EventsArray = std::vector<event_ptr_tt<EventT>>;
using Events = EventsArray<Event>;

#define QUO_CONTENT_GETTER_IMPL(PartType_, PartName_, JsonKey_) \
    PartType_ PartName_() const                                 \
    {                                                           \
        static const auto PartName_##JsonKey = JsonKey_;        \
        return contentPart<PartType_>(PartName_##JsonKey);      \
    }

//! \brief Define an inline method obtaining a content part
//!
//! This macro adds a const method that extracts a JSON value at the key
//! <tt>toSnakeCase(PartName_)</tt> (sic) and converts it to the type
//! \p PartType_. Effectively, the generated method is an equivalent of
//! \code
//! contentPart<PartType_>(Quotient::toSnakeCase(#PartName_##_ls));
//! \endcode
#define QUO_CONTENT_GETTER(PartType_, PartName_) \
    QUO_CONTENT_GETTER_IMPL(PartType_, PartName_, toSnakeCase(#PartName_##_ls))

// === Facilities for event class definitions ===

//! \brief Adds an event factory to its default factory chain
//!
//! Adds a standard factory method (via makeIfMatches) for a given event type
//! to EventT::factory so that loadEvent<>() could create objects of this class
//! dynamically from JSON.
//!
//! \tparam EventT the type to enable dynamic creation of
//! \sa loadEvent, Event::type
template <class EventT>
event_type_t registerEventType(event_type_t matrixType,
                               const char* className = nullptr)
{
    // A simple way to ensure the same factory is only added once.
    // This won't really protect against double registration across
    // dynamically loaded libraries but each of those factory methods is
    // interchangeable and self-contained anyway so two entries won't really
    // hurt.
    static const char* registeredClassName = nullptr;
    registeredClassName = EventT::factory.tryAddMethod(&makeIfMatches<EventT>,
                                                       matrixType, className,
                                                       registeredClassName);
    return { matrixType };
}

// This macro should be used in a public section of an event class to
// provide matrixTypeId() and typeId().
#define DEFINE_EVENT_TYPEID(Id_, Type_)                                   \
    static inline const auto TypeId { registerEventType<Type_>(Id_##_ls,  \
                                                               #Type_) }; \
    [[deprecated("Use " #Type_ "::TypeId directly instead")]]             \
    static constexpr event_mtype_t matrixTypeId() { return Id_; }         \
    [[deprecated("Use " #Type_ "::TypeId directly instead")]]             \
    static event_type_t typeId() { return TypeId; }                       \
    // End of macro

// === EventBase<> and other facilities to create event classes ===

template <class EventT, class EventBaseT, typename ContentT = void>
class EventBase : public EventBaseT {
public:
    using content_type = ContentT;

    explicit EventBase(const QJsonObject& json)
        : EventBaseT(typeId<EventT>, json)
    {}
    explicit EventBase(const ContentT& c)
        : EventBaseT(typeId<EventT>,
                     EventBaseT::basicJson(typeId<EventT>, toJson(c)))
    {}

    ContentT content() const { return fromJson<ContentT>(this->contentJson()); }
};

// Specialisation for an event that doesn't use ContentT parameter
template <typename EventT, typename EventBaseT>
class EventBase<EventT, EventBaseT> : public EventBaseT {
public:
    explicit EventBase(const QJsonObject& json)
        : EventBaseT(Quotient::typeId<EventT>, json)
    {}
};

/// \brief Define a new event class with a single key-value pair in the content
///
/// This macro defines a new event class \p Name_ derived from \p Base_,
/// with Matrix event type \p TypeId_, providing a getter named \p GetterName_
/// for a single value of type \p ValueType_ inside the event content.
/// To retrieve the value the getter uses a JSON key name that corresponds to
/// its own (getter's) name but written in snake_case. \p GetterName_ must be
/// in camelCase, no quotes (an identifier, not a literal).
#define DEFINE_SIMPLE_EVENT(Name_, Base_, TypeId_, ValueType_, GetterName_,   \
                            JsonKey_)                                         \
    constexpr auto Name_##ContentKey = JsonKey_##_ls;                         \
    class QUOTIENT_API Name_                                                  \
        : public EventBase<                                                   \
              Name_, Base_,                                                   \
              EventContent::SingleKeyValue<ValueType_, &Name_##ContentKey>> { \
    public:                                                                   \
        DEFINE_EVENT_TYPEID(TypeId_, Name_)                                   \
        using value_type = ValueType_;                                        \
        using EventBase::EventBase;                                           \
        QUO_CONTENT_GETTER_IMPL(ValueType_, GetterName_, Name_##ContentKey)   \
    };                                                                        \
    // End of macro

// === is<>(), eventCast<>() and switchOnType<>() ===

template <class EventT>
inline bool is(const Event& e)
{
    return e.type() == typeId<EventT>;
}

inline bool isUnknown(const Event& e)
{
    return e.type() == UnknownEventTypeId;
}

template <class EventT, typename BasePtrT>
inline auto eventCast(const BasePtrT& eptr)
    -> decltype(static_cast<EventT*>(&*eptr))
{
    Q_ASSERT(eptr);
    return is<std::decay_t<EventT>>(*eptr) ? static_cast<EventT*>(&*eptr)
                                           : nullptr;
}

// A trivial generic catch-all "switch"
template <class BaseEventT, typename FnT>
inline auto switchOnType(const BaseEventT& event, FnT&& fn)
    -> decltype(fn(event))
{
    return fn(event);
}

namespace _impl {
    // Using bool instead of auto below because auto apparently upsets MSVC
    template <class BaseT, typename FnT>
    constexpr bool needs_downcast =
        std::is_base_of_v<BaseT, std::decay_t<fn_arg_t<FnT>>>
        && !std::is_same_v<BaseT, std::decay_t<fn_arg_t<FnT>>>;
}

// A trivial type-specific "switch" for a void function
template <class BaseT, typename FnT>
inline auto switchOnType(const BaseT& event, FnT&& fn)
    -> std::enable_if_t<_impl::needs_downcast<BaseT, FnT>
                        && std::is_void_v<fn_return_t<FnT>>>
{
    using event_type = fn_arg_t<FnT>;
    if (is<std::decay_t<event_type>>(event))
        fn(static_cast<event_type>(event));
}

// A trivial type-specific "switch" for non-void functions with an optional
// default value; non-voidness is guarded by defaultValue type
template <class BaseT, typename FnT>
inline auto switchOnType(const BaseT& event, FnT&& fn,
                         fn_return_t<FnT>&& defaultValue = {})
    -> std::enable_if_t<_impl::needs_downcast<BaseT, FnT>, fn_return_t<FnT>>
{
    using event_type = fn_arg_t<FnT>;
    if (is<std::decay_t<event_type>>(event))
        return fn(static_cast<event_type>(event));
    return std::move(defaultValue);
}

// A switch for a chain of 2 or more functions
template <class BaseT, typename FnT1, typename FnT2, typename... FnTs>
inline std::common_type_t<fn_return_t<FnT1>, fn_return_t<FnT2>>
switchOnType(const BaseT& event, FnT1&& fn1, FnT2&& fn2, FnTs&&... fns)
{
    using event_type1 = fn_arg_t<FnT1>;
    if (is<std::decay_t<event_type1>>(event))
        return fn1(static_cast<event_type1&>(event));
    return switchOnType(event, std::forward<FnT2>(fn2),
                        std::forward<FnTs>(fns)...);
}

template <class BaseT, typename... FnTs>
[[deprecated("The new name for visit() is switchOnType()")]] //
inline auto visit(const BaseT& event, FnTs&&... fns)
{
    return switchOnType(event, std::forward<FnTs>(fns)...);
}

    // A facility overload that calls void-returning switchOnType() on each event
// over a range of event pointers
// TODO: replace with ranges::for_each once all standard libraries have it
template <typename RangeT, typename... FnTs>
inline auto visitEach(RangeT&& events, FnTs&&... fns)
    -> std::enable_if_t<std::is_void_v<
        decltype(switchOnType(**begin(events), std::forward<FnTs>(fns)...))>>
{
    for (auto&& evtPtr: events)
        switchOnType(*evtPtr, std::forward<FnTs>(fns)...);
}
} // namespace Quotient
Q_DECLARE_METATYPE(Quotient::Event*)
Q_DECLARE_METATYPE(const Quotient::Event*)
