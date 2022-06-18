// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "e2ee/qolminboundsession.h"

#include "vodozemac/src/lib.rs.h"

#include <cstring>
#include <iostream>

using namespace Quotient;

struct QOlmInboundGroupSession::InboundGroupSession {
    rust::Box<megolm::InboundGroupSession> value;
};

using PicklingKey = std::array<std::uint8_t, 32>;

QByteArray rustStrToByteArr(const rust::String& str);
PicklingKey picklingModeToKey(const PicklingMode& mode);
QOlmError toQOlmError(const std::exception& e);
std::logic_error notImplemented(std::string_view functionName);

QOlmError lastError(OlmInboundGroupSession *session) {
    return fromString(olm_inbound_group_session_last_error(session));
}

QOlmInboundGroupSession::QOlmInboundGroupSession(OlmInboundGroupSession* session)
    : QOlmInboundGroupSession()
{}

QOlmInboundGroupSession::QOlmInboundGroupSession()
    : m_groupSession()
{
}

QOlmInboundGroupSession::~QOlmInboundGroupSession() {}

std::unique_ptr<QOlmInboundGroupSession> QOlmInboundGroupSession::create(const QByteArray &key)
{
    try {
        auto sessionKey = megolm::session_key_from_base64(key.data());
        auto result = std::unique_ptr<QOlmInboundGroupSession>(
            new QOlmInboundGroupSession());
        result->m_groupSession =
            std::make_unique<InboundGroupSession>(InboundGroupSession {
                megolm::new_inbound_group_session(*sessionKey) });
        return result;
    } catch (const std::exception& e) {
        throw toQOlmError(e);
    }
}

std::unique_ptr<QOlmInboundGroupSession> QOlmInboundGroupSession::import(const QByteArray &key)
{
    try {
        auto sessionKey = megolm::exported_session_key_from_base64(key.data());
        auto result = std::unique_ptr<QOlmInboundGroupSession>(
            new QOlmInboundGroupSession());
        result->m_groupSession =
            std::make_unique<InboundGroupSession>(InboundGroupSession {
                megolm::import_inbound_group_session(*sessionKey) });
        return result;
    } catch (const std::exception& e) {
        throw toQOlmError(e);
    }
}

QByteArray toKey(const PicklingMode &mode)
{
    if (std::holds_alternative<Unencrypted>(mode)) {
        return "";
    }
    return std::get<Encrypted>(mode).key;
}

QByteArray QOlmInboundGroupSession::pickle(const PicklingMode &mode) const
{
    return rustStrToByteArr(
        m_groupSession->value->pickle(picklingModeToKey(mode)));
}

QOlmExpected<QOlmInboundGroupSessionPtr> QOlmInboundGroupSession::unpickle(
    const QByteArray& pickled, const PicklingMode& mode)
{
    try {
        auto result = std::unique_ptr<QOlmInboundGroupSession>(
            new QOlmInboundGroupSession());
        result->m_groupSession = std::make_unique<InboundGroupSession>(
            InboundGroupSession { megolm::inbound_group_session_from_pickle(
                pickled.data(), picklingModeToKey(mode)) });
        return result;
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

QOlmExpected<std::pair<QByteArray, uint32_t>> QOlmInboundGroupSession::decrypt(
    const QByteArray& message)
{
    try {
        auto megolmMessage = megolm::megolm_message_from_base64(message.data());
        auto result = m_groupSession->value->decrypt(*megolmMessage);
        return std::pair<QByteArray, uint32_t> {
            rustStrToByteArr(result.plaintext), result.message_index
        };
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

QOlmExpected<QByteArray> QOlmInboundGroupSession::exportSession(uint32_t messageIndex)
{
    try {
        return rustStrToByteArr(
            m_groupSession->value->export_at(messageIndex)->to_base64());
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

uint32_t QOlmInboundGroupSession::firstKnownIndex() const
{
    return m_groupSession->value->first_known_index();
}

QByteArray QOlmInboundGroupSession::sessionId() const
{
    return rustStrToByteArr(m_groupSession->value->session_id());
}

bool QOlmInboundGroupSession::isVerified() const
{
    // This function has not apparent vodozemac equivalent, and according to
    // the libolm documentation it is mainly used in unit tests.
    throw notImplemented("QOlmInboundGroupSession::isVerified");
}

QString QOlmInboundGroupSession::olmSessionId() const
{
    return m_olmSessionId;
}
void QOlmInboundGroupSession::setOlmSessionId(const QString& newOlmSessionId)
{
    m_olmSessionId = newOlmSessionId;
}

QString QOlmInboundGroupSession::senderId() const
{
    return m_senderId;
}
void QOlmInboundGroupSession::setSenderId(const QString& senderId)
{
    m_senderId = senderId;
}
