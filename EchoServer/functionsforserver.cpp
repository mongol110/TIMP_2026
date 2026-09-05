#include "functionsforserver.h"
#include "dataBase.h"

#include <QStringList>
#include <QByteArray>
#include <cmath>
#include <cstdint>

#include <openssl/evp.h>

// f(x) = x^3 - x - 2, корень ~1.5213797; f'(x) = 3x^2 - 1
static double fNewton(double x) { return x * x * x - x - 2.0; }
static double dfNewton(double x) { return 3.0 * x * x - 1.0; }

// ================= SHA1 вручную (FIPS 180-4, без библиотек) =================

static inline uint32_t sha1Rol(uint32_t v, int n) { return (v << n) | (v >> (32 - n)); }

static QByteArray sha1RawManual(const QByteArray& data)
{
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE,
             h3 = 0x10325476, h4 = 0xC3D2E1F0;
    QByteArray msg = data;
    uint64_t bitLen = (uint64_t)msg.size() * 8;
    msg.append(char(0x80));
    while (msg.size() % 64 != 56) msg.append(char(0x00));
    for (int i = 7; i >= 0; --i)
        msg.append(char((bitLen >> (i * 8)) & 0xFF));
    for (int off = 0; off < msg.size(); off += 64) {
        uint32_t w[80];
        const unsigned char* p = reinterpret_cast<const unsigned char*>(msg.constData() + off);
        for (int i = 0; i < 16; ++i)
            w[i] = ((uint32_t)p[i*4] << 24) | ((uint32_t)p[i*4+1] << 16) |
                   ((uint32_t)p[i*4+2] << 8) | (uint32_t)p[i*4+3];
        for (int i = 16; i < 80; ++i)
            w[i] = sha1Rol(w[i-3] ^ w[i-8] ^ w[i-14] ^ w[i-16], 1);
        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20)      { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;            k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;            k = 0xCA62C1D6; }
            uint32_t tmp = sha1Rol(a, 5) + f + e + k + w[i];
            e = d; d = c; c = sha1Rol(b, 30); b = a; a = tmp;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }
    QByteArray out;
    out.reserve(20);
    for (uint32_t h : {h0, h1, h2, h3, h4})
        for (int i = 3; i >= 0; --i) out.append(char((h >> (i * 8)) & 0xFF));
    return out;
}

// ================= AES-128-CBC через OpenSSL EVP =================

static QByteArray aesMakeKey(const QString& key)
{
    QByteArray k = key.toUtf8();
    if (k.size() > 16) k.resize(16);
    else if (k.size() < 16) k.append(QByteArray(16 - k.size(), '\0'));
    return k;
}

static const QByteArray kIV(16, '\0');

QString parsing(const QString& command, const QString& currentUser, QString& newUser, bool& authOk)
{
    newUser = currentUser;
    authOk = false;
    if (command.isEmpty()) return "error";

    const QStringList parts = command.split('|');
    const QString cmd = parts[0].toUpper().trimmed();

    if (cmd == "REGISTER" && parts.size() >= 3) {
        QString login = parts[1].trimmed();
        QString pass = parts.mid(2).join('|');
        return DataBase::instance().registerUser(login, pass);
    }
    if (cmd == "AUTH" && parts.size() >= 3) {
        QString login = parts[1].trimmed();
        QString pass = parts.mid(2).join('|');
        QString res = DataBase::instance().authUser(login, pass);
        if (res.startsWith("ok:")) {
            newUser = login;
            authOk = true;
        }
        return res;
    }

    if (currentUser.isEmpty())
        return "error: not authenticated";

    if (cmd == "SHA1" && parts.size() >= 2)
        return sha1Hash(parts.mid(1).join('|'));

    if (cmd == "AES_ENCRYPT" && parts.size() >= 3)
        return aesEncrypt(parts[1], parts.mid(2).join('|'));

    if (cmd == "AES_DECRYPT" && parts.size() >= 3)
        return aesDecrypt(parts[1], parts[2]);

    if (cmd == "NEWTON" && parts.size() >= 3) {
        bool ok1, ok2;
        double x0 = parts[1].toDouble(&ok1);
        double eps = parts[2].toDouble(&ok2);
        if (!ok1 || !ok2) return "error";
        return newtonMethod(x0, eps);
    }

    if (cmd == "AUDIO_EMBED" && parts.size() >= 3)
        return audioEmbed(parts[1], parts.mid(2).join('|'));

    if (cmd == "AUDIO_EXTRACT" && parts.size() >= 2)
        return audioExtract(parts[1]);

    return "error";
}

QString aesEncrypt(const QString& key, const QString& plaintext)
{
    const QByteArray keyBytes = aesMakeKey(key);
    const QByteArray plainBytes = plaintext.toUtf8();

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "error";
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(keyBytes.constData()),
                           reinterpret_cast<const unsigned char*>(kIV.constData())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "error";
    }
    QByteArray out(plainBytes.size() + 16, '\0');
    int len = 0, finalLen = 0;
    EVP_EncryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(out.data()), &len,
                      reinterpret_cast<const unsigned char*>(plainBytes.constData()),
                      plainBytes.size());
    EVP_EncryptFinal_ex(ctx,
                        reinterpret_cast<unsigned char*>(out.data()) + len, &finalLen);
    EVP_CIPHER_CTX_free(ctx);
    out.resize(len + finalLen);
    return QString::fromLatin1(out.toHex());
}

QString aesDecrypt(const QString& key, const QString& ciphertextHex)
{
    const QByteArray keyBytes = aesMakeKey(key);
    const QByteArray cipherBytes = QByteArray::fromHex(ciphertextHex.toLatin1());
    if (cipherBytes.isEmpty()) return "error: bad hex";

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "error";
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr,
                           reinterpret_cast<const unsigned char*>(keyBytes.constData()),
                           reinterpret_cast<const unsigned char*>(kIV.constData())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "error";
    }
    QByteArray out(cipherBytes.size(), '\0');
    int len = 0, finalLen = 0;
    EVP_DecryptUpdate(ctx,
                      reinterpret_cast<unsigned char*>(out.data()), &len,
                      reinterpret_cast<const unsigned char*>(cipherBytes.constData()),
                      cipherBytes.size());
    if (EVP_DecryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(out.data()) + len,
                            &finalLen) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        return "error: decryption failed (bad key or corrupt data)";
    }
    EVP_CIPHER_CTX_free(ctx);
    out.resize(len + finalLen);
    return QString::fromUtf8(out);
}

QString sha1Hash(const QString& data)
{
    return QString::fromLatin1(sha1RawManual(data.toUtf8()).toHex());
}

QString newtonMethod(double x0, double eps)
{
    if (!(eps >= 1e-12 && eps <= 1.0)) return "error: bad eps (need 1e-12..1)";
    if (!std::isfinite(x0) || std::fabs(x0) > 1e6) return "error: bad x0";
    double x = x0;
    for (int i = 0; i < 10000; ++i) {
        double fx = fNewton(x);
        double dfx = dfNewton(x);
        if (std::fabs(dfx) < 1e-12) return "error: derivative too small";
        double x1 = x - fx / dfx;
        if (!std::isfinite(x1)) return "error: diverged";
        if (std::fabs(x1 - x) < eps) return QString::number(x1, 'f', 10);
        x = x1;
    }
    return QString::number(x, 'f', 10);
}

static bool splitWav(const QByteArray& wav, QByteArray& header, QByteArray& audio)
{
    if (wav.size() < 44) return false;
    if (!wav.startsWith("RIFF") || wav.mid(8, 4) != "WAVE") return false;
    header = wav.left(44);
    audio = wav.mid(44);
    return true;
}

QString audioEmbed(const QString& base64Wav, const QString& message)
{
    QByteArray wav = QByteArray::fromBase64(base64Wav.toLatin1());
    QByteArray header, audio;
    if (!splitWav(wav, header, audio)) return "error: bad wav (need 44-byte RIFF/WAVE)";
    QByteArray msg = message.toUtf8();
    qint64 need = (qint64)(4 + msg.size()) * 8;
    if (need > audio.size()) return "error: message too long for this wav";
    QByteArray payload;
    payload.reserve(4 + msg.size());
    qint32 len = (qint32)msg.size();
    payload.append(char((len >> 24) & 0xFF));
    payload.append(char((len >> 16) & 0xFF));
    payload.append(char((len >> 8) & 0xFF));
    payload.append(char(len & 0xFF));
    payload.append(msg);
    for (qint64 i = 0; i < need; ++i) {
        int bit = (payload[i / 8] >> (7 - (i % 8))) & 1;
        audio[(int)i] = char((audio[(int)i] & 0xFE) | bit);
    }
    return QString::fromLatin1((header + audio).toBase64());
}

QString audioExtract(const QString& base64Wav)
{
    QByteArray wav = QByteArray::fromBase64(base64Wav.toLatin1());
    QByteArray header, audio;
    if (!splitWav(wav, header, audio)) return "error: bad wav";
    if (audio.size() < 32) return "error: wav too small";
    qint32 len = 0;
    for (int i = 0; i < 32; ++i)
        len = (len << 1) | (audio[i] & 1);
    if (len < 0 || len > 10000000) return "error: invalid length header";
    if ((qint64)32 + (qint64)len * 8 > audio.size()) return "error: wav too small for claimed length";
    QByteArray out;
    out.reserve(len);
    for (int b = 0; b < len; ++b) {
        int v = 0;
        for (int k = 0; k < 8; ++k)
            v = (v << 1) | (audio[32 + b * 8 + k] & 1);
        out.append(char(v));
    }
    return QString::fromUtf8(out);
}
