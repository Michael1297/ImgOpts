#include "md5.h"

#include <iomanip>

// Константы для MD5
constexpr uint32_t INIT_A = 0x67452301;
constexpr uint32_t INIT_B = 0xEFCDAB89;
constexpr uint32_t INIT_C = 0x98BADCFE;
constexpr uint32_t INIT_D = 0x10325476;

constexpr uint32_t T[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dde,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

constexpr uint32_t S[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};

// Вспомогательные функции
inline uint32_t F(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
inline uint32_t G(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
inline uint32_t H(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
inline uint32_t I(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }

inline uint32_t rotateLeft(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

// Функция для вычисления MD5
std::string crypto::md5(const std::string& input) {
    // Добавление padding к входным данным
    size_t originalLength = input.size();
    size_t paddedLength = ((originalLength + 8) / 64 + 1) * 64;
    std::string paddedInput(paddedLength, 0);
    std::copy(input.begin(), input.end(), paddedInput.begin());
    paddedInput[originalLength] = 0x80;

    uint64_t bitLength = originalLength * 8;
    for (size_t i = 0; i < 8; ++i) {
        paddedInput[paddedLength - 8 + i] = static_cast<uint8_t>((bitLength >> (8 * i)) & 0xFF);
    }

    // Инициализация переменных
    uint32_t a = INIT_A, b = INIT_B, c = INIT_C, d = INIT_D;

    // Обработка блоков по 512 бит
    for (size_t i = 0; i < paddedLength; i += 64) {
        uint32_t X[16];
        for (size_t j = 0; j < 16; ++j) {
            X[j] = (static_cast<uint32_t>(paddedInput[i + j * 4]) << 0) |
                   (static_cast<uint32_t>(paddedInput[i + j * 4 + 1]) << 8) |
                   (static_cast<uint32_t>(paddedInput[i + j * 4 + 2]) << 16) |
                   (static_cast<uint32_t>(paddedInput[i + j * 4 + 3]) << 24);
        }

        uint32_t A = a, B = b, C = c, D = d;

        // Раунды MD5
        for (int j = 0; j < 64; ++j) {
            uint32_t f, g;
            if (j < 16) {
                f = F(B, C, D);
                g = j;
            } else if (j < 32) {
                f = G(B, C, D);
                g = (5 * j + 1) % 16;
            } else if (j < 48) {
                f = H(B, C, D);
                g = (3 * j + 5) % 16;
            } else {
                f = I(B, C, D);
                g = (7 * j) % 16;
            }

            const uint32_t temp = D;
            D = C;
            C = B;
            B = B + rotateLeft(A + f + X[g] + T[j], S[j]);
            A = temp;
        }

        a += A;
        b += B;
        c += C;
        d += D;
    }

    // Формирование результата
    std::ostringstream hexStream;
    hexStream << std::hex << std::setfill('0');
    for (const uint32_t val : {a, b, c, d}) {
        for (int i = 0; i < 4; ++i) {
            hexStream << std::setw(2) << ((val >> (8 * i)) & 0xFF);
        }
    }

    return hexStream.str();
}
