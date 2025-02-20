/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "read_markers.h"

using namespace Quotient;

SetReadMarkerJob::SetReadMarkerJob(const QString& roomId,
                                   const QString& mFullyRead,
                                   const QString& mRead)
    : BaseJob(HttpVerb::Post, QStringLiteral("SetReadMarkerJob"),
              makePath("/_matrix/client/v3", "/rooms/", roomId, "/read_markers"))
{
    QJsonObject _data;
    addParam<>(_data, QStringLiteral("m.fully_read"), mFullyRead);
    addParam<IfNotEmpty>(_data, QStringLiteral("m.read"), mRead);
    setRequestData(std::move(_data));
}
