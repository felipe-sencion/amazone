#ifndef USER_H
#define USER_H

#include <QString>

using namespace std;

class User
{
public:
    User();

    QString getEmail() const;
    void setEmail(const QString &value);

    QString getName() const;
    void setName(const QString &value);

    QString getPassword() const;
    void setPassword(const QString &value);

private:
    QString email;
    QString name;
    QString password;
};

#endif // USER_H
