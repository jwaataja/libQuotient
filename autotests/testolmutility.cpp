// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "testolmutility.h"

#include "e2ee/qolmaccount.h"
#include "e2ee/qolmutility.h"
#include "vodozemac/src/lib.rs.h"

using namespace Quotient;

void TestOlmUtility::canonicalJSON()
{
    // Examples taken from
    // https://matrix.org/docs/spec/appendices.html#canonical-json
    auto data = QJsonDocument::fromJson(QByteArrayLiteral(R"({
    "auth": {
      "success": true,
      "mxid": "@john.doe:example.com",
      "profile": {
        "display_name": "John Doe",
        "three_pids": [{
          "medium": "email",
          "address": "john.doe@example.org"
        }, {
          "medium": "msisdn",
          "address": "123456789"
        }]
      }}})"));

    QCOMPARE(data.toJson(QJsonDocument::Compact),
      "{\"auth\":{\"mxid\":\"@john.doe:example.com\",\"profile\":{\"display_name\":\"John "
      "Doe\",\"three_pids\":[{\"address\":\"john.doe@example.org\",\"medium\":\"email\"},{"
      "\"address\":\"123456789\",\"medium\":\"msisdn\"}]},\"success\":true}}");

    auto data0 = QJsonDocument::fromJson(QByteArrayLiteral(R"({"b":"2","a":"1"})"));
    QCOMPARE(data0.toJson(QJsonDocument::Compact), "{\"a\":\"1\",\"b\":\"2\"}");

    auto data1 = QJsonDocument::fromJson(QByteArrayLiteral(R"({ "本": 2, "日": 1 })"));
    QCOMPARE(data1.toJson(QJsonDocument::Compact), "{\"日\":1,\"本\":2}");

    auto data2 = QJsonDocument::fromJson(QByteArrayLiteral(R"({"a": "\u65E5"})"));
    QCOMPARE(data2.toJson(QJsonDocument::Compact), "{\"a\":\"日\"}");

    auto data3 = QJsonDocument::fromJson(QByteArrayLiteral(R"({ "a": null })"));
    QCOMPARE(data3.toJson(QJsonDocument::Compact), "{\"a\":null}");
}

void TestOlmUtility::verifySignedOneTimeKey()
{
    QOlmAccount aliceOlm { "@alice:matrix.org", "aliceDevice" };
    aliceOlm.createNewAccount();
    aliceOlm.generateOneTimeKeys(1);
    auto keys = aliceOlm.oneTimeKeys();

    auto firstKey = *keys.curve25519().begin();
    auto msgObj = QJsonObject({{"key", firstKey}});
    auto sig = aliceOlm.sign(msgObj);

    auto msg = QJsonDocument(msgObj).toJson(QJsonDocument::Compact);

    QByteArray signatureBuf1(sig.length(), '0');
    std::copy(sig.begin(), sig.end(), signatureBuf1.begin());

    auto edKey =
        types::ed25519_key_from_base64(aliceOlm.identityKeys().ed25519.data());
    auto signature = types::ed25519_signature_from_base64(sig.data());
    edKey->verify({ reinterpret_cast<const uint8_t*>(msg.data()),
                    static_cast<size_t>(msg.size()) },
                  *signature);

    QOlmUtility utility2;
    auto res2 =
        utility2
            .ed25519Verify(aliceOlm.identityKeys().ed25519, msg, signatureBuf1)
            .value_or(false);

    //QCOMPARE(std::string(olm_utility_last_error(utility)), "SUCCESS");
    QVERIFY(res2);
}

void TestOlmUtility::validUploadKeysRequest()
{
    const auto userId = QStringLiteral("@alice:matrix.org");
    const auto deviceId = QStringLiteral("FKALSOCCC");

    QOlmAccount alice { userId, deviceId };
    alice.createNewAccount();
    alice.generateOneTimeKeys(1);

    auto idSig = alice.signIdentityKeys();

    QJsonObject body
    {
        {"algorithms", QJsonArray{"m.olm.v1.curve25519-aes-sha2", "m.megolm.v1.aes-sha2"}},
        {"user_id", userId},
        {"device_id", deviceId},
        {"keys",
            QJsonObject{
                {QStringLiteral("curve25519:") + deviceId, QString::fromUtf8(alice.identityKeys().curve25519)},
                {QStringLiteral("ed25519:") + deviceId, QString::fromUtf8(alice.identityKeys().ed25519)}
            }
        },
        {"signatures",
            QJsonObject{
                {userId,
                    QJsonObject{
                        {"ed25519:" + deviceId, QString::fromUtf8(idSig)}
                    }
                }
            }
        }
    };

    DeviceKeys deviceKeys = alice.deviceKeys();
    QCOMPARE(QJsonDocument(toJson(deviceKeys)).toJson(QJsonDocument::Compact),
            QJsonDocument(body).toJson(QJsonDocument::Compact));

    QVERIFY(verifyIdentitySignature(fromJson<DeviceKeys>(body), deviceId, userId));
    QVERIFY(verifyIdentitySignature(deviceKeys, deviceId, userId));
}
QTEST_GUILESS_MAIN(TestOlmUtility)
