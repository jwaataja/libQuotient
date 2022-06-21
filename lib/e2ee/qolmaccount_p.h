// SPDX-FileCopyrightText: 2022 Jason Waataja <jasonswaataja@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "e2ee/qolmaccount.h"
#include "vodozemac/src/lib.rs.h"

namespace Quotient {
struct QOlmAccount::Account {
    rust::Box<olm::Account> value;
};
} // namespace Quotient
