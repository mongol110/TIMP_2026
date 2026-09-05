#ifndef FUNCTIONSFORSERVER_H
#define FUNCTIONSFORSERVER_H

#include <QString>

QString parsing(const QString& command, const QString& currentUser, QString& newUser, bool& authOk);

QString aesEncrypt(const QString& key, const QString& plaintext);
QString aesDecrypt(const QString& key, const QString& ciphertextHex);
QString sha1Hash(const QString& data);
QString newtonMethod(double x0, double eps);
QString audioEmbed(const QString& base64Wav, const QString& message);
QString audioExtract(const QString& base64Wav);

#endif
