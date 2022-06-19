/******************************************************************************
 * THIS FILE IS GENERATED - ANY EDITS WILL BE OVERWRITTEN
 */

#include "receipts.h"

using namespace Quotient;

PostReceiptJob::PostReceiptJob(const QString& roomId, const QString& receiptType,
                               const QString& eventId,
                               const QJsonObject& receipt)
    : BaseJob(HttpVerb::Post, QStringLiteral("PostReceiptJob"),
              makePath("/_matrix/client/v3", "/rooms/", roomId, "/receipt/",
                       receiptType, "/", eventId))
{
    setRequestData({ toJson(receipt) });
}
