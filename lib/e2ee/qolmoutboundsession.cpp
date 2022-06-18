// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "e2ee/qolmoutboundsession.h"

#include "e2ee/qolmutils.h"
#include "vodozemac/src/lib.rs.h"

using namespace Quotient;

struct QOlmOutboundGroupSession::GroupSession {
    rust::Box<megolm::GroupSession> value;
};

using PicklingKey = std::array<std::uint8_t, 32>;

QByteArray rustStrToByteArr(const rust::String& str);
rust::String qStrToStr(const QString& str);
PicklingKey picklingModeToKey(const PicklingMode& mode);
QOlmError toQOlmError(const std::exception& e);

QOlmError lastError(OlmOutboundGroupSession *session) {
    return fromString(olm_outbound_group_session_last_error(session));
}

QOlmOutboundGroupSession::QOlmOutboundGroupSession(
    OlmOutboundGroupSession* session)
    : QOlmOutboundGroupSession()
{}

QOlmOutboundGroupSession::QOlmOutboundGroupSession()
    : m_groupSession(std::make_unique<GroupSession>(
        GroupSession { megolm::new_group_session() }))
{}

QOlmOutboundGroupSession::~QOlmOutboundGroupSession() = default;

QOlmOutboundGroupSessionPtr QOlmOutboundGroupSession::create()
{
    return std::unique_ptr<QOlmOutboundGroupSession>(
        new QOlmOutboundGroupSession());
}

QOlmExpected<QByteArray> QOlmOutboundGroupSession::pickle(const PicklingMode &mode) const
{
    return rustStrToByteArr(
        m_groupSession->value->pickle(picklingModeToKey(mode)));
}

QOlmExpected<QOlmOutboundGroupSessionPtr> QOlmOutboundGroupSession::unpickle(const QByteArray &pickled, const PicklingMode &mode)
{
    try {
        auto result = std::unique_ptr<QOlmOutboundGroupSession>(
            new QOlmOutboundGroupSession());
        result->m_groupSession = std::make_unique<GroupSession>(GroupSession {
            megolm::group_session_from_pickle(pickled.data(),
                                              picklingModeToKey(mode)) });
        return result;
    } catch (const std::exception& e) {
        return toQOlmError(e);
    }
}

QOlmExpected<QByteArray> QOlmOutboundGroupSession::encrypt(const QString &plaintext) const
{
    return rustStrToByteArr(
        m_groupSession->value->encrypt(qStrToStr(plaintext))->to_base64());
}

uint32_t QOlmOutboundGroupSession::sessionMessageIndex() const
{
    return m_groupSession->value->message_index();
}

QByteArray QOlmOutboundGroupSession::sessionId() const
{
    return rustStrToByteArr(m_groupSession->value->session_id());
}

QOlmExpected<QByteArray> QOlmOutboundGroupSession::sessionKey() const
{
    return rustStrToByteArr(m_groupSession->value->session_key()->to_base64());
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
