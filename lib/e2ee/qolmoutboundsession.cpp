// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "e2ee/qolmoutboundsession.h"
#include "e2ee/qolmutils.h"

using namespace Quotient;

QOlmError lastError(OlmOutboundGroupSession *session) {
    return fromString(olm_outbound_group_session_last_error(session));
}

QOlmOutboundGroupSession::QOlmOutboundGroupSession(OlmOutboundGroupSession *session)
    : m_groupSession(session)
{}

QOlmOutboundGroupSession::~QOlmOutboundGroupSession()
{
    olm_clear_outbound_group_session(m_groupSession);
    delete[](reinterpret_cast<uint8_t *>(m_groupSession));
}

QOlmOutboundGroupSessionPtr QOlmOutboundGroupSession::create()
{
    auto *olmOutboundGroupSession = olm_outbound_group_session(new uint8_t[olm_outbound_group_session_size()]);
    const auto randomLength = olm_init_outbound_group_session_random_length(olmOutboundGroupSession);
    QByteArray randomBuf = getRandom(randomLength);

    const auto error = olm_init_outbound_group_session(olmOutboundGroupSession,
            reinterpret_cast<uint8_t *>(randomBuf.data()), randomBuf.length());

    if (error == olm_error()) {
        throw lastError(olmOutboundGroupSession);
    }

    const auto keyMaxLength = olm_outbound_group_session_key_length(olmOutboundGroupSession);
    QByteArray keyBuffer(keyMaxLength, '0');
    olm_outbound_group_session_key(olmOutboundGroupSession, reinterpret_cast<uint8_t *>(keyBuffer.data()),
            keyMaxLength);

    randomBuf.clear();

    return std::make_unique<QOlmOutboundGroupSession>(olmOutboundGroupSession);
}

QOlmExpected<QByteArray> QOlmOutboundGroupSession::pickle(const PicklingMode &mode) const
{
    QByteArray pickledBuf(olm_pickle_outbound_group_session_length(m_groupSession), '0');
    QByteArray key = toKey(mode);
    const auto error = olm_pickle_outbound_group_session(m_groupSession, key.data(), key.length(),
            pickledBuf.data(), pickledBuf.length());

    if (error == olm_error()) {
        return lastError(m_groupSession);
    }

    key.clear();

    return pickledBuf;
}

QOlmExpected<QOlmOutboundGroupSessionPtr> QOlmOutboundGroupSession::unpickle(const QByteArray &pickled, const PicklingMode &mode)
{
    QByteArray pickledBuf = pickled;
    auto *olmOutboundGroupSession = olm_outbound_group_session(new uint8_t[olm_outbound_group_session_size()]);
    QByteArray key = toKey(mode);
    const auto error = olm_unpickle_outbound_group_session(olmOutboundGroupSession, key.data(), key.length(),
            pickledBuf.data(), pickledBuf.length());
    if (error == olm_error()) {
        return lastError(olmOutboundGroupSession);
    }
    const auto idMaxLength = olm_outbound_group_session_id_length(olmOutboundGroupSession);
    QByteArray idBuffer(idMaxLength, '0');
    olm_outbound_group_session_id(olmOutboundGroupSession, reinterpret_cast<uint8_t *>(idBuffer.data()),
            idBuffer.length());

    key.clear();
    return std::make_unique<QOlmOutboundGroupSession>(olmOutboundGroupSession);
}

QOlmExpected<QByteArray> QOlmOutboundGroupSession::encrypt(const QString &plaintext) const
{
    QByteArray plaintextBuf = plaintext.toUtf8();
    const auto messageMaxLength = olm_group_encrypt_message_length(m_groupSession, plaintextBuf.length());
    QByteArray messageBuf(messageMaxLength, '0');
    const auto error = olm_group_encrypt(m_groupSession, reinterpret_cast<uint8_t *>(plaintextBuf.data()),
            plaintextBuf.length(), reinterpret_cast<uint8_t *>(messageBuf.data()), messageBuf.length());

    if (error == olm_error()) {
        return lastError(m_groupSession);
    }

    return messageBuf;
}

uint32_t QOlmOutboundGroupSession::sessionMessageIndex() const
{
    return olm_outbound_group_session_message_index(m_groupSession);
}

QByteArray QOlmOutboundGroupSession::sessionId() const
{
    const auto idMaxLength = olm_outbound_group_session_id_length(m_groupSession);
    QByteArray idBuffer(idMaxLength, '0');
    const auto error = olm_outbound_group_session_id(m_groupSession, reinterpret_cast<uint8_t *>(idBuffer.data()),
            idBuffer.length());
    if (error == olm_error()) {
        throw lastError(m_groupSession);
    }
    return idBuffer;
}

QOlmExpected<QByteArray> QOlmOutboundGroupSession::sessionKey() const
{
    const auto keyMaxLength = olm_outbound_group_session_key_length(m_groupSession);
    QByteArray keyBuffer(keyMaxLength, '0');
    const auto error = olm_outbound_group_session_key(
        m_groupSession, reinterpret_cast<uint8_t*>(keyBuffer.data()),
        keyMaxLength);
    if (error == olm_error()) {
        return lastError(m_groupSession);
    }
    return keyBuffer;
}

int QOlmOutboundGroupSession::messageCount() const
{
    return m_messageCount;
}

void QOlmOutboundGroupSession::setMessageCount(int messageCount)
{
    m_messageCount = messageCount;
}

QDateTime QOlmOutboundGroupSession::creationTime() const
{
    return m_creationTime;
}

void QOlmOutboundGroupSession::setCreationTime(const QDateTime& creationTime)
{
    m_creationTime = creationTime;
}
