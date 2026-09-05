#include "functionsforserver.h"
#include "dataBase.h"

#include <QStringList>
#include <QByteArray>
#include <cmath>
#include <cstdint>
#include <vector>

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

// ================= AES-128 вручную (FIPS 197, без библиотек) =================

static const uint8_t kSbox[256] = {
0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16};

static const uint8_t kInvSbox[256] = {
0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d};

static const uint8_t kRcon[10] = {
0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36};

static uint8_t aesXtime(uint8_t x) { return (x << 1) ^ ((x & 0x80) ? 0x1b : 0x00); }
static uint8_t aesMul(uint8_t a, uint8_t b) {
    uint8_t r = 0;
    while (b) { if (b & 1) r ^= a; a = aesXtime(a); b >>= 1; }
    return r;
}

// state[16] column-major: state[row + 4*col]
static void aesAddRoundKey(uint8_t* s, const uint8_t* rk) { for (int i = 0; i < 16; ++i) s[i] ^= rk[i]; }
static void aesSubBytes(uint8_t* s) { for (int i = 0; i < 16; ++i) s[i] = kSbox[s[i]]; }
static void aesInvSubBytes(uint8_t* s) { for (int i = 0; i < 16; ++i) s[i] = kInvSbox[s[i]]; }
static void aesShiftRows(uint8_t* s) {
    uint8_t t[16];
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c) t[r + 4*c] = s[r + 4*((c + r) % 4)];
    for (int i = 0; i < 16; ++i) s[i] = t[i];
}
static void aesInvShiftRows(uint8_t* s) {
    uint8_t t[16];
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c) t[r + 4*c] = s[r + 4*((c - r + 4) % 4)];
    for (int i = 0; i < 16; ++i) s[i] = t[i];
}
static void aesMixColumns(uint8_t* s) {
    for (int c = 0; c < 4; ++c) {
        uint8_t a0 = s[0+4*c], a1 = s[1+4*c], a2 = s[2+4*c], a3 = s[3+4*c];
        s[0+4*c] = aesMul(a0,2) ^ aesMul(a1,3) ^ a2 ^ a3;
        s[1+4*c] = a0 ^ aesMul(a1,2) ^ aesMul(a2,3) ^ a3;
        s[2+4*c] = a0 ^ a1 ^ aesMul(a2,2) ^ aesMul(a3,3);
        s[3+4*c] = aesMul(a0,3) ^ a1 ^ a2 ^ aesMul(a3,2);
    }
}
static void aesInvMixColumns(uint8_t* s) {
    for (int c = 0; c < 4; ++c) {
        uint8_t a0 = s[0+4*c], a1 = s[1+4*c], a2 = s[2+4*c], a3 = s[3+4*c];
        s[0+4*c] = aesMul(a0,0x0e) ^ aesMul(a1,0x0b) ^ aesMul(a2,0x0d) ^ aesMul(a3,0x09);
        s[1+4*c] = aesMul(a0,0x09) ^ aesMul(a1,0x0e) ^ aesMul(a2,0x0b) ^ aesMul(a3,0x0d);
        s[2+4*c] = aesMul(a0,0x0d) ^ aesMul(a1,0x09) ^ aesMul(a2,0x0e) ^ aesMul(a3,0x0b);
        s[3+4*c] = aesMul(a0,0x0b) ^ aesMul(a1,0x0d) ^ aesMul(a2,0x09) ^ aesMul(a3,0x0e);
    }
}

static void aesKeyExpand(const uint8_t key[16], uint8_t rk[176])
{
    for (int i = 0; i < 16; ++i) rk[i] = key[i];
    for (int i = 16; i < 176; i += 4) {
        uint8_t t[4] = {rk[i-4], rk[i-3], rk[i-2], rk[i-1]};
        if ((i / 4) % 4 == 0) {
            uint8_t u = t[0]; t[0] = t[1]; t[1] = t[2]; t[2] = t[3]; t[3] = u; // RotWord
            for (int k = 0; k < 4; ++k) t[k] = kSbox[t[k]];                    // SubWord
            t[0] ^= kRcon[(i / 16) - 1];
        }
        rk[i] = rk[i-16] ^ t[0]; rk[i+1] = rk[i-15] ^ t[1];
        rk[i+2] = rk[i-14] ^ t[2]; rk[i+3] = rk[i-13] ^ t[3];
    }
}

static void aesEncryptBlock(const uint8_t in[16], uint8_t out[16], const uint8_t rk[176])
{
    uint8_t s[16];
    for (int i = 0; i < 16; ++i) s[i] = in[i];
    aesAddRoundKey(s, rk);
    for (int round = 1; round <= 9; ++round) {
        aesSubBytes(s); aesShiftRows(s); aesMixColumns(s);
        aesAddRoundKey(s, rk + round * 16);
    }
    aesSubBytes(s); aesShiftRows(s);
    aesAddRoundKey(s, rk + 160);
    for (int i = 0; i < 16; ++i) out[i] = s[i];
}

static void aesDecryptBlock(const uint8_t in[16], uint8_t out[16], const uint8_t rk[176])
{
    uint8_t s[16];
    for (int i = 0; i < 16; ++i) s[i] = in[i];
    aesAddRoundKey(s, rk + 160);
    aesInvShiftRows(s); aesInvSubBytes(s);
    for (int round = 9; round >= 1; --round) {
        aesAddRoundKey(s, rk + round * 16);
        aesInvMixColumns(s);
        aesInvShiftRows(s); aesInvSubBytes(s);
    }
    aesAddRoundKey(s, rk);
    for (int i = 0; i < 16; ++i) out[i] = s[i];
}

static QByteArray aesMakeKey(const QString& key)
{
    QByteArray k = key.toUtf8();
    // resize() не зануляет новые байты — добиваем нулями явно,
    // иначе encrypt и decrypt получат разный мусор в хвосте ключа.
    if (k.size() > 16) k.resize(16);
    else if (k.size() < 16) k.append(QByteArray(16 - k.size(), '\0'));
    return k;
}

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
    QByteArray kb = aesMakeKey(key);
    uint8_t rk[176];
    aesKeyExpand(reinterpret_cast<const uint8_t*>(kb.constData()), rk);
    QByteArray pt = plaintext.toUtf8();
    int pad = 16 - (pt.size() % 16);
    for (int i = 0; i < pad; ++i) pt.append(char(pad)); // PKCS7 вручную
    QByteArray out;
    out.reserve(pt.size());
    uint8_t prev[16] = {0}; // IV = 0
    for (int off = 0; off < pt.size(); off += 16) {
        uint8_t blk[16], enc[16];
        for (int i = 0; i < 16; ++i)
            blk[i] = (uint8_t)pt[off + i] ^ prev[i];
        aesEncryptBlock(blk, enc, rk);
        out.append(reinterpret_cast<const char*>(enc), 16);
        for (int i = 0; i < 16; ++i) prev[i] = enc[i];
    }
    return QString::fromLatin1(out.toHex());
}

QString aesDecrypt(const QString& key, const QString& ciphertextHex)
{
    QByteArray ct = QByteArray::fromHex(ciphertextHex.toLatin1());
    if (ct.isEmpty() || ct.size() % 16 != 0) return "error: bad hex";
    QByteArray kb = aesMakeKey(key);
    uint8_t rk[176];
    aesKeyExpand(reinterpret_cast<const uint8_t*>(kb.constData()), rk);
    QByteArray out;
    out.reserve(ct.size());
    uint8_t prev[16] = {0};
    for (int off = 0; off < ct.size(); off += 16) {
        uint8_t dec[16];
        aesDecryptBlock(reinterpret_cast<const uint8_t*>(ct.constData() + off), dec, rk);
        for (int i = 0; i < 16; ++i) dec[i] ^= prev[i];
        out.append(reinterpret_cast<const char*>(dec), 16);
        for (int i = 0; i < 16; ++i) prev[i] = (uint8_t)ct[off + i];
    }
    if (out.isEmpty()) return "error";
    int pad = (unsigned char)out[out.size() - 1];
    if (pad < 1 || pad > 16 || pad > out.size()) return "error: decryption failed (bad key or corrupt data)";
    for (int i = 0; i < pad; ++i)
        if ((unsigned char)out[out.size() - 1 - i] != (unsigned char)pad)
            return "error: decryption failed (bad key or corrupt data)";
    out.chop(pad);
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
