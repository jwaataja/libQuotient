// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "qolmaccount.h"

#include "connection.h"
#include "e2ee/qolmsession.h"
#include "e2ee/qolmutility.h"
#include "e2ee/qolmutils.h"
#include "lib.rs.h"

#include "csapi/keys.h"

#include <QtCore/QRandomGenerator>

#include <olm/olm.h>

using namespace Quotient;

QByteArray rustStrToByteArr(const rust::String& str)
{
    return QByteArray(str.data(), str.length());
}

QString rustStrToQStr(const rust::String& str)
{
    return QString::fromUtf8(str.data(), str.length());
}

std::logic_error notImplemented(std::string_view functionName)
{
    std::string message("called unimplemented function ");
    message += functionName;
    return std::logic_error(message);
}

// Converts an exception thrown by vodozemac into a QOlmError.
QOlmError toQOlmError(const std::exception& e)
{
    // TODO: Would the fromString functoin work here? It expects strings from
    // libOlm, which might differ from vodozemac.
    return Quotient::Unknown;
}

using PicklingKey = std::array<std::uint8_t, 32>;

PicklingKey picklingModeToKey(const PicklingMode& mode)
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

// Convert olm error to enum
QOlmError lastError(OlmAccount *account) {
    return fromString(olm_account_last_error(account));
}

QOlmAccount::QOlmAccount(const QString& userId, const QString& deviceId,
                         QObject* parent)
    : QObject(parent)
    , m_account(std::nullopt)
    , m_userId(userId)
    , m_deviceId(deviceId)
{}

void QOlmAccount::createNewAccount()
{
    m_account = olm::new_account();
    emit needsSave();
}

void QOlmAccount::unpickle(QByteArray &pickled, const PicklingMode &mode)
{
    try {
        m_account =
            olm::account_from_pickle(pickled.data(), picklingModeToKey(mode));
    } catch (const std::exception& e) {
        qCWarning(E2EE) << "Failed to unpickle olm account";
        // TODO: Do something that is not dying
        //  Probably log the user out since we have no way of getting to the keys
    }
}

QOlmExpected<QByteArray> QOlmAccount::pickle(const PicklingMode &mode)
{
    return rustStrToByteArr(m_account.value()->pickle(picklingModeToKey(mode)));
}

IdentityKeys QOlmAccount::identityKeys() const
{
    return { rustStrToByteArr(m_account.value()->curve25519_key()->to_base64()),
             rustStrToByteArr(
                 m_account.value()->curve25519_key()->to_base64()) };
}

QByteArray QOlmAccount::sign(const QByteArray &message) const
{
    return rustStrToByteArr(
        m_account.value()->sign(message.data())->to_base64());
}

QByteArray QOlmAccount::sign(const QJsonObject &message) const
{
    return sign(QJsonDocument(message).toJson(QJsonDocument::Compact));
}

QByteArray QOlmAccount::signIdentityKeys() const
{
    const auto keys = identityKeys();
    return sign(QJsonObject {
        { "algorithms", QJsonArray { "m.olm.v1.curve25519-aes-sha2",
                                     "m.megolm.v1.aes-sha2" } },
        { "user_id", m_userId },
        { "device_id", m_deviceId },
        { "keys", QJsonObject { { QStringLiteral("curve25519:") + m_deviceId,
                                  QString::fromUtf8(keys.curve25519) },
                                { QStringLiteral("ed25519:") + m_deviceId,
                                  QString::fromUtf8(keys.ed25519) } } } });
}

size_t QOlmAccount::maxNumberOfOneTimeKeys() const
{
    return m_account.value()->max_number_of_one_time_keys();
}

size_t QOlmAccount::generateOneTimeKeys(size_t numberOfKeys)
{
    m_account.value()->generate_one_time_keys(numberOfKeys);
    emit needsSave();
    // TODO: Is this the correct return value?
    return 0;
}

UnsignedOneTimeKeys QOlmAccount::oneTimeKeys() const
{
    UnsignedOneTimeKeys keys;
    for (const auto& key : m_account.value()->one_time_keys()) {
        keys.keys[Curve25519Key][key.key_id.data()] =
            rustStrToQStr(key.key->to_base64());
    }
    return keys;
}

OneTimeKeys QOlmAccount::signOneTimeKeys(const UnsignedOneTimeKeys &keys) const
{
    OneTimeKeys signedOneTimeKeys;
    for (const auto& curveKeys = keys.curve25519();
         const auto& [keyId, key] : asKeyValueRange(curveKeys))
        signedOneTimeKeys["signed_curve25519:" % keyId] =
            signedOneTimeKey(key.toUtf8(), sign(QJsonObject{{"key", key}}));
    return signedOneTimeKeys;
}

SignedOneTimeKey QOlmAccount::signedOneTimeKey(const QByteArray& key,
                                               const QString& signature) const
{
    return { key, { { m_userId, { { "ed25519:" + m_deviceId, signature } } } } };
}

std::optional<QOlmError> QOlmAccount::removeOneTimeKeys(
    const QOlmSession& session)
{
    // const auto error = olm_remove_one_time_keys(m_account, session.raw());
    //
    // if (error == olm_error()) {
    // return lastError(m_account);
    // }
    // emit needsSave();
    // return std::nullopt;

    throw notImplemented("QOlmAccount::removeOneTimeKeys");
}

OlmAccount* QOlmAccount::data()
{
    // return m_account;
    throw notImplemented("QOlmAccount::data");
}

DeviceKeys QOlmAccount::deviceKeys() const
{
    DeviceKeys deviceKeys;
    deviceKeys.userId = m_userId;
    deviceKeys.deviceId = m_deviceId;
    deviceKeys.algorithms = QStringList {"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"};

    const auto idKeys = identityKeys();
    deviceKeys.keys["curve25519:" + m_deviceId] = idKeys.curve25519;
    deviceKeys.keys["ed25519:" + m_deviceId] = idKeys.ed25519;

    const auto sign = signIdentityKeys();
    deviceKeys.signatures[m_userId]["ed25519:" + m_deviceId] = sign;

    return deviceKeys;
}

UploadKeysJob* QOlmAccount::createUploadKeyRequest(
    const UnsignedOneTimeKeys& oneTimeKeys) const
{
    return new UploadKeysJob(deviceKeys(), signOneTimeKeys(oneTimeKeys));
}

QOlmExpected<QOlmSessionPtr> QOlmAccount::createInboundSession(
    const QOlmMessage& preKeyMessage)
{
    Q_ASSERT(preKeyMessage.type() == QOlmMessage::PreKey);
    return QOlmSession::createInboundSession(this, preKeyMessage);
}

QOlmExpected<QOlmSessionPtr> QOlmAccount::createInboundSessionFrom(
    const QByteArray& theirIdentityKey, const QOlmMessage& preKeyMessage)
{
    Q_ASSERT(preKeyMessage.type() == QOlmMessage::PreKey);
    return QOlmSession::createInboundSessionFrom(this, theirIdentityKey,
                                                 preKeyMessage);
}

QOlmExpected<QOlmSessionPtr> QOlmAccount::createOutboundSession(
    const QByteArray& theirIdentityKey, const QByteArray& theirOneTimeKey)
{
    return QOlmSession::createOutboundSession(this, theirIdentityKey,
                                              theirOneTimeKey);
}

void QOlmAccount::markKeysAsPublished()
{
    m_account.value()->mark_keys_as_published();
    emit needsSave();
}

bool Quotient::verifyIdentitySignature(const DeviceKeys& deviceKeys,
                                       const QString& deviceId,
                                       const QString& userId)
{
    const auto signKeyId = "ed25519:" + deviceId;
    const auto signingKey = deviceKeys.keys[signKeyId];
    const auto signature = deviceKeys.signatures[userId][signKeyId];

    return ed25519VerifySignature(signingKey, toJson(deviceKeys), signature);
}

bool Quotient::ed25519VerifySignature(const QString& signingKey,
                                      const QJsonObject& obj,
                                      const QString& signature)
{
    if (signature.isEmpty())
        return false;

    QJsonObject obj1 = obj;

    obj1.remove("unsigned");
    obj1.remove("signatures");

    auto canonicalJson = QJsonDocument(obj1).toJson(QJsonDocument::Compact);

    QByteArray signingKeyBuf = signingKey.toUtf8();
    QOlmUtility utility;
    auto signatureBuf = signature.toUtf8();
    return utility.ed25519Verify(signingKeyBuf, canonicalJson, signatureBuf)
        .value_or(false);
}
