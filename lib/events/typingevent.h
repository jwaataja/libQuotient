// SPDX-FileCopyrightText: 2017 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "event.h"

namespace Quotient {
class QUOTIENT_API TypingEvent : public EventBase<TypingEvent, Event> {
public:
    DEFINE_EVENT_TYPEID("m.typing", TypingEvent)
    using EventBase::EventBase;

    QStringList users() const;
};
} // namespace Quotient
