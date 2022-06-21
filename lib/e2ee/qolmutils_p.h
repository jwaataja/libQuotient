// SPDX-FileCopyrightText: 2022 Jason Waataja <jasonswaataja@gmail.com>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "e2ee/e2ee.h"
#include "e2ee/qolmerrors.h"
#include "e2ee/qolmmessage.h"
#include "vodozemac/src/lib.rs.h"

#include <QByteArray>
#include <QString>
#include <stdexcept>

namespace Quotient {

using PicklingKey = std::array<std::uint8_t, 32>;

QByteArray rustStrToByteArr(const rust::String& str);
QString rustStrToQStr(const rust::String& str);
rust::String qStrToStr(const QString& str);
rust::Slice<const uint8_t> byteArrToByteSlice(const QByteArray& arr);
rust::Box<olm::OlmMessage> toOlmMessage(const QOlmMessage& message);
PicklingKey picklingModeToKey(const PicklingMode& mode);
std::logic_error notImplemented(std::string_view functionName);
// Converts an exception thrown by vodozemac into a QOlmError.
QOlmError toQOlmError(const std::exception& e);

} // namespace Quotient
