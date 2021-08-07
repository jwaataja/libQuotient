// SPDX-FileCopyrightText: 2015 Felix Rohrbach <kde@fxrh.de>
// SPDX-FileCopyrightText: 2016 Kitsune Ral <Kitsune-Ral@users.sf.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "avatar.h"

#include <QtCore/QObject>

namespace Quotient {
class Connection;
class Room;
class RoomMemberEvent;

class User : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(bool isGuest READ isGuest CONSTANT)
    Q_PROPERTY(int hue READ hue CONSTANT)
    Q_PROPERTY(qreal hueF READ hueF CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY defaultNameChanged)
    Q_PROPERTY(QString displayName READ displayname NOTIFY defaultNameChanged STORED false)
    Q_PROPERTY(QString fullName READ fullName NOTIFY defaultNameChanged STORED false)
    Q_PROPERTY(QString bridgeName READ bridged NOTIFY defaultNameChanged STORED false)
    Q_PROPERTY(QString avatarMediaId READ avatarMediaId NOTIFY defaultAvatarChanged STORED false)
    Q_PROPERTY(QUrl avatarUrl READ avatarUrl NOTIFY defaultAvatarChanged)
public:
    User(QString userId, Connection* connection);
    ~User() override;

    Connection* connection() const;

    /** Get unique stable user id
     * User id is generated by the server and is not changed ever.
     */
    QString id() const;

    /** Get the name chosen by the user
     * This may be empty if the user didn't choose the name or cleared
     * it. If the user is bridged, the bridge postfix (such as '(IRC)')
     * is stripped out. No disambiguation for the room is done.
     * \sa displayName, rawName
     */
    QString name(const Room* room = nullptr) const;

    /** Get the user name along with the bridge postfix
     * This function is similar to name() but appends the bridge postfix
     * (such as '(IRC)') to the user name. No disambiguation is done.
     * \sa name, displayName
     */
    [[deprecated("Bridge postfixes exist no more, use name() instead")]]
    QString rawName(const Room* room = nullptr) const;

    /** Get the displayed user name
     * When \p room is null, this method returns result of name() if
     * the name is non-empty; otherwise it returns user id.
     * When \p room is non-null, this call is equivalent to
     * Room::roomMembername invocation for the user (i.e. the user's
     * disambiguated room-specific name is returned).
     * \sa name, id, fullName, Room::roomMembername
     */
    QString displayname(const Room* room = nullptr) const;

    /** Get user name and id in one string
     * The constructed string follows the format 'name (id)'
     * which the spec recommends for users disambiguation in
     * a room context and in other places.
     * \sa displayName, Room::roomMembername
     */
    QString fullName(const Room* room = nullptr) const;

    /**
     * Returns the name of bridge the user is connected from or empty.
     */
    [[deprecated("Bridged status is no more supported; this always returns"
                 " an empty string")]]
    QString bridged() const;

    /** Whether the user is a guest
     * As of now, the function relies on the convention used in Synapse
     * that guests and only guests have all-numeric IDs. This may or
     * may not work with non-Synapse servers.
     */
    bool isGuest() const;

    /** Hue color component of this user based on id.
     * The implementation is based on XEP-0392:
     * https://xmpp.org/extensions/xep-0392.html
     * Naming and ranges are the same as QColor's hue methods:
     * https://doc.qt.io/qt-5/qcolor.html#integer-vs-floating-point-precision
     */
    int hue() const;
    qreal hueF() const;

    /// Get a reference to a user avatar object for a given room
    /*! This reference should be considered short-lived: processing the next
     * room member event for this user may (or may not) invalidate it.
     */
    const Avatar& avatarObject(const Room* room = nullptr) const;
    Q_INVOKABLE QImage avatar(int dimension,
                              const Quotient::Room* room = nullptr) const;
    Q_INVOKABLE QImage avatar(int requestedWidth, int requestedHeight,
                              const Quotient::Room* room = nullptr) const;
    QImage avatar(int width, int height, const Room* room,
                  const Avatar::get_callback_t& callback) const;

    QString avatarMediaId(const Room* room = nullptr) const;
    QUrl avatarUrl(const Room* room = nullptr) const;

public Q_SLOTS:
    /// Set a new name in the global user profile
    void rename(const QString& newName);
    /// Set a new name for the user in one room
    void rename(const QString& newName, const Room* r);
    /// Upload the file and use it as an avatar
    bool setAvatar(const QString& fileName);
    /// Upload contents of the QIODevice and set that as an avatar
    bool setAvatar(QIODevice* source);
    /// Removes the avatar from the profile
    void removeAvatar();
    /// Create or find a direct chat with this user
    /*! The resulting chat is returned asynchronously via
     * Connection::directChatAvailable()
     */
    void requestDirectChat();
    /// Add the user to the ignore list
    void ignore();
    /// Remove the user from the ignore list
    void unmarkIgnore();
    /// Check whether the user is in ignore list
    bool isIgnored() const;
    /// Force loading displayName and avartar url. This is required in
    /// some cases where the you need to use an user independent of the
    /// room.
    void load();

Q_SIGNALS:
    void defaultNameChanged();
    void defaultAvatarChanged();

private:
    class Private;
    QScopedPointer<Private> d;

    template <typename SourceT>
    bool doSetAvatar(SourceT&& source);
};
} // namespace Quotient
