// SPDX-FileCopyrightText: 2021 Alexey Andreyev <aa13q@ya.ru>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "qolmsession.h"

#include "e2ee/qolmutils.h"
#include "logging.h"
#include "vodozemac/src/lib.rs.h"

#include <cstring>
#include <olm/olm.h>

using namespace Quotient;

struct QOlmAccount::Account {
    rust::Box<olm::Account> value;
};

struct QOlmSession::Session {
    rust::Box<olm::Session> value;
};

using PicklingKey = std::array<std::uint8_t, 32>;

QByteArray rustStrToByteArr(const rust::String& str);
rust::String qStrToStr(const QString& str);
rust::Slice<const uint8_t> byteArrToByteSlice(const QByteArray& arr);
rust::Box<olm::OlmMessage> toOlmMessage(const QOlmMessage& message);
PicklingKey picklingModeToKey(const PicklingMode& mode);
QOlmError toQOlmError(const std::exception& e);

QOlmError lastError(OlmSession* session) {
    return fromString(olm_session_last_error(session));
}

QOlmSession::QOlmSession() = default;

QOlmSession::~QOlmSession() = default;

QOlmExpected<QOlmSessionPtr> QOlmSession::createInbound(
    QOlmAccount* account, const QOlmMessage& preKeyMessage, bool from,
    const QString& theirIdentityKey)
{
    if (preKeyMessage.type() != QOlmMessage::PreKey) {
        qCCritical(E2EE) << "The message is not a pre-key in when creating inbound session" << BadMessageFormat;
    }

    rust::Str keyStr;
    if (from) {
        keyStr = qStrToStr(theirIdentityKey);
    } else {
        keyStr = preKeyMessage.toCiphertext().data();
    }
    try {
        auto message = toOlmMessage(preKeyMessage);
        auto key = types::curve_key_from_base64(keyStr);
        // TODO: I don't like using bare new here, but to get rid of it, we
        // would have to make this constructor public in order to use
        // make_unique.
        auto result = std::unique_ptr<QOlmSession>(new QOlmSession());
        // TODO: This returns the decrypted message as well, what happens to
        // it?
        result->m_session = std::make_unique<Session>(Session {
            (account->m_account->value->create_inbound_session(*key, *message)
                 .session) });
        return result;
    } catch (const std::exception& e) {
        qCWarning(E2EE) << "Error when creating inbound session: " << e.what();
        return toQOlmError(e);
    }
}

QOlmExpected<QOlmSessionPtr> QOlmSession::createInboundSession(
    QOlmAccount* account, const QOlmMessage& preKeyMessage)
{
    return createInbound(account, preKeyMessage);
}

QOlmExpected<QOlmSessionPtr> QOlmSession::createInboundSessionFrom(
    QOlmAccount* account, const QString& theirIdentityKey,
    const QOlmMessage& preKeyMessage)
{
    return createInbound(account, preKeyMessage, true, theirIdentityKey);
}

QOlmExpected<QOlmSessionPtr> QOlmSession::createOutboundSession(
    QOlmAccount* account, const QString& theirIdentityKey,
    const QString& theirOneTimeKey)
{
    try {
        auto identityKey =
            types::curve_key_from_base64(qStrToStr(theirIdentityKey));
        auto oneTimeKey =
            types::curve_key_from_base64(qStrToStr(theirOneTimeKey));
        auto result = std::unique_ptr<QOlmSession>(new QOlmSession());
        result->m_session = std::make_unique<Session>(Session {
            account->m_account->value->create_outbound_session(*identityKey,
                                                               *oneTimeKey) });
        return result;
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

QOlmExpected<QByteArray> QOlmSession::pickle(const PicklingMode &mode) const
{
    return rustStrToByteArr(m_session->value->pickle(picklingModeToKey(mode)));
}

QOlmExpected<QOlmSessionPtr> QOlmSession::unpickle(const QByteArray& pickled,
                                                   const PicklingMode& mode)
{
    try {
        auto result = std::unique_ptr<QOlmSession>(new QOlmSession());
        result->m_session = std::make_unique<Session>(Session {
            olm::session_from_pickle(pickled.data(), picklingModeToKey(mode)) });
        return result;
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

QOlmExpected<QOlmSessionPtr> QOlmSession::unpickleLibOlm(
    const QByteArray& pickled, const PicklingMode& mode)
{
    try {
        auto result = std::unique_ptr<QOlmSession>(new QOlmSession());
        result->m_session = std::make_unique<Session>(
            Session { olm::session_from_libolm_pickle(pickled.data(),
                                                      byteArrToByteSlice(toKey(mode).data())) });
        return result;
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

QOlmMessage QOlmSession::encrypt(const QString &plaintext)
{
    auto encryptedParts =
        m_session->value->encrypt(qStrToStr(plaintext))->to_parts();
    return QOlmMessage(rustStrToByteArr(encryptedParts.ciphertext),
                       QOlmMessage::Type(encryptedParts.message_type));
}

QOlmExpected<QByteArray> QOlmSession::decrypt(const QOlmMessage &message) const
{
    try {
        return rustStrToByteArr(
            m_session->value->decrypt(*toOlmMessage(message)));
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

QOlmMessage::Type QOlmSession::encryptMessageType()
{
    if (hasReceivedMessage()) {
        return QOlmMessage::General;
    } else {
        return QOlmMessage::PreKey;
    }
}

QByteArray QOlmSession::sessionId() const
{
    return rustStrToByteArr(m_session->value->session_id());
}

bool QOlmSession::hasReceivedMessage() const
{
    return m_session->value->has_received_message();
}

bool QOlmSession::matchesInboundSession(const QOlmMessage& preKeyMessage) const
{
    // TODO: Should this check if the current session is inbound?
    return m_session->value->session_matches(*toOlmMessage(preKeyMessage));
}

bool QOlmSession::matchesInboundSessionFrom(
    const QString& theirIdentityKey, const QOlmMessage& preKeyMessage) const
{
    try {
        auto key = types::curve_key_from_base64(qStrToStr(theirIdentityKey));
        return m_session->value->session_matches_from(*key, *toOlmMessage(
                                                                preKeyMessage));
    } catch (const std::exception&) {
        return false;
    }
}

QOlmSession::QOlmSession(OlmSession* session) {}
