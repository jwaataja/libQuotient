/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "pusher.h"

using namespace Quotient;

QUrl GetPushersJob::makeRequestUrl(QUrl baseUrl)
{
    return BaseJob::makeRequestUrl(std::move(baseUrl),
                                   makePath("/_matrix/client/v3", "/pushers"));
}

GetPushersJob::GetPushersJob()
    : BaseJob(HttpVerb::Get, QStringLiteral("GetPushersJob"),
              makePath("/_matrix/client/v3", "/pushers"))
{}

PostPusherJob::PostPusherJob(const QString& pushkey, const QString& kind,
                             const QString& appId, const QString& appDisplayName,
                             const QString& deviceDisplayName,
                             const QString& lang, const PusherData& data,
                             const QString& profileTag, Omittable<bool> append)
    : BaseJob(HttpVerb::Post, QStringLiteral("PostPusherJob"),
              makePath("/_matrix/client/v3", "/pushers/set"))
{
    QJsonObject _data;
    addParam<>(_data, QStringLiteral("pushkey"), pushkey);
    addParam<>(_data, QStringLiteral("kind"), kind);
    addParam<>(_data, QStringLiteral("app_id"), appId);
    addParam<>(_data, QStringLiteral("app_display_name"), appDisplayName);
    addParam<>(_data, QStringLiteral("device_display_name"), deviceDisplayName);
    addParam<IfNotEmpty>(_data, QStringLiteral("profile_tag"), profileTag);
    addParam<>(_data, QStringLiteral("lang"), lang);
    addParam<>(_data, QStringLiteral("data"), data);
    addParam<IfNotEmpty>(_data, QStringLiteral("append"), append);
    setRequestData(std::move(_data));
}
