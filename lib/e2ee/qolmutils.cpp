// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "e2ee/qolmutils.h"

#include "e2ee/qolmmessage.h"
#include "e2ee/qolmutils_p.h"
#include "vodozemac/src/lib.rs.h"

#include <QtCore/QRandomGenerator>

using namespace Quotient;

QByteArray Quotient::toKey(const Quotient::PicklingMode &mode)
{
    if (std::holds_alternative<Quotient::Unencrypted>(mode)) {
        return {};
    }
    return std::get<Quotient::Encrypted>(mode).key;
}

QByteArray Quotient::getRandom(size_t bufferSize)
{
    QByteArray buffer(bufferSize, '0');
    QRandomGenerator::system()->generate(buffer.begin(), buffer.end());
    return buffer;
}

QByteArray Quotient::rustStrToByteArr(const rust::String& str)
{
    return {str.data(), static_cast<int>(str.length())};
}

QString Quotient::rustStrToQStr(const rust::String& str)
{
    return QString::fromUtf8(str.data(), str.length());
}

rust::String Quotient::qStrToStr(const QString& str)
{
    // TODO: Possible opportunities for optimization.
    return str.toStdString();
}

rust::Slice<const uint8_t> Quotient::byteArrToByteSlice(const QByteArray& arr)
{
    return {reinterpret_cast<const uint8_t*>(arr.data()), static_cast<size_t>(arr.size())};
}

rust::Box<olm::OlmMessage> Quotient::toOlmMessage(const QOlmMessage& message)
{
    return olm::olm_message_from_parts(
        { message.type(), message.toCiphertext().data() });
}

PicklingKey Quotient::picklingModeToKey(const PicklingMode& mode)
{
    PicklingKey result;
    result.fill(0);
    if (std::holds_alternative<Quotient::Encrypted>(mode)) {
        auto it = result.begin();
        for (auto b : std::get<Encrypted>(mode).key) {
            if (it == result.end()) {
                break;
            }

            *it++ = b;
        }
    }

    return result;
}

std::logic_error Quotient::notImplemented(std::string_view functionName)
{
    std::string message("called unimplemented function ");
    message += functionName;
    return std::logic_error(message);
}

// Converts an exception thrown by vodozemac into a QOlmError.
QOlmError Quotient::toQOlmError(const std::exception& /*e*/)
{
    // TODO: Would the fromString functoin work here? It expects strings from
    // libOlm, which might differ from vodozemac.
    return Unknown;
}
