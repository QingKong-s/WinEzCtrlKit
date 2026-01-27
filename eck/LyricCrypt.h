#pragma once
#include "Compress.h"

ECK_NAMESPACE_BEGIN
ECK_LYRIC_NAMESPACE_BEGIN
namespace Priv
{
    typedef enum {
        QRC_DES_ENCRYPT,
        QRC_DES_DECRYPT
    } QRC_DES_MODE;

    typedef unsigned int DES_WORD;
#define ECK_LRCP_BITNUM(a,b,c)      (((a[(b) / 32 * 4 + 3 - (b) % 32 / 8] >> (7 - (b % 8))) & 0x01) << (c))
#define ECK_LRCP_BITNUMINTR(a,b,c)  ((((a) >> (31 - (b))) & 0x00000001) << (c))
#define ECK_LRCP_BITNUMINTL(a,b,c)  ((((a) << (b)) & 0x80000000) >> (c))
#define ECK_LRCP_SBOXBIT(a)         (((a) & 0x20) | (((a) & 0x1f) >> 1) | (((a) & 0x01) << 4))
    static const BYTE sbox1[64] = {
        14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7,
        0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8,
        4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0,
        15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13
    };
    static const BYTE sbox2[64] = {
        15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10,
        3,13,4,7,15,2,8,15,12,0,1,10,6,9,11,5,
        0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15,
        13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9
    };
    static const BYTE sbox3[64] = {
        10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8,
        13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1,
        13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7,
        1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12
    };
    static const BYTE sbox4[64] = {
        7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15,
        13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9,
        10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4,
        3,15,0,6,10,10,13,8,9,4,5,11,12,7,2,14
    };
    static const BYTE sbox5[64] = {
        2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9,
        14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6,
        4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14,
        11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3
    };
    static const BYTE sbox6[64] = {
        12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11,
        10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8,
        9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6,
        4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13
    };
    static const BYTE sbox7[64] = {
        4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1,
        13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6,
        1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2,
        6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12
    };
    static const BYTE sbox8[64] = {
        13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7,
        1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2,
        7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8,
        2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11
    };
    static void IP(DES_WORD state[], const BYTE in[])
    {
        state[0] = ECK_LRCP_BITNUM(in, 57, 31) | ECK_LRCP_BITNUM(in, 49, 30) | ECK_LRCP_BITNUM(in, 41, 29) | ECK_LRCP_BITNUM(in, 33, 28) |
            ECK_LRCP_BITNUM(in, 25, 27) | ECK_LRCP_BITNUM(in, 17, 26) | ECK_LRCP_BITNUM(in, 9, 25) | ECK_LRCP_BITNUM(in, 1, 24) |
            ECK_LRCP_BITNUM(in, 59, 23) | ECK_LRCP_BITNUM(in, 51, 22) | ECK_LRCP_BITNUM(in, 43, 21) | ECK_LRCP_BITNUM(in, 35, 20) |
            ECK_LRCP_BITNUM(in, 27, 19) | ECK_LRCP_BITNUM(in, 19, 18) | ECK_LRCP_BITNUM(in, 11, 17) | ECK_LRCP_BITNUM(in, 3, 16) |
            ECK_LRCP_BITNUM(in, 61, 15) | ECK_LRCP_BITNUM(in, 53, 14) | ECK_LRCP_BITNUM(in, 45, 13) | ECK_LRCP_BITNUM(in, 37, 12) |
            ECK_LRCP_BITNUM(in, 29, 11) | ECK_LRCP_BITNUM(in, 21, 10) | ECK_LRCP_BITNUM(in, 13, 9) | ECK_LRCP_BITNUM(in, 5, 8) |
            ECK_LRCP_BITNUM(in, 63, 7) | ECK_LRCP_BITNUM(in, 55, 6) | ECK_LRCP_BITNUM(in, 47, 5) | ECK_LRCP_BITNUM(in, 39, 4) |
            ECK_LRCP_BITNUM(in, 31, 3) | ECK_LRCP_BITNUM(in, 23, 2) | ECK_LRCP_BITNUM(in, 15, 1) | ECK_LRCP_BITNUM(in, 7, 0);
        state[1] = ECK_LRCP_BITNUM(in, 56, 31) | ECK_LRCP_BITNUM(in, 48, 30) | ECK_LRCP_BITNUM(in, 40, 29) | ECK_LRCP_BITNUM(in, 32, 28) |
            ECK_LRCP_BITNUM(in, 24, 27) | ECK_LRCP_BITNUM(in, 16, 26) | ECK_LRCP_BITNUM(in, 8, 25) | ECK_LRCP_BITNUM(in, 0, 24) |
            ECK_LRCP_BITNUM(in, 58, 23) | ECK_LRCP_BITNUM(in, 50, 22) | ECK_LRCP_BITNUM(in, 42, 21) | ECK_LRCP_BITNUM(in, 34, 20) |
            ECK_LRCP_BITNUM(in, 26, 19) | ECK_LRCP_BITNUM(in, 18, 18) | ECK_LRCP_BITNUM(in, 10, 17) | ECK_LRCP_BITNUM(in, 2, 16) |
            ECK_LRCP_BITNUM(in, 60, 15) | ECK_LRCP_BITNUM(in, 52, 14) | ECK_LRCP_BITNUM(in, 44, 13) | ECK_LRCP_BITNUM(in, 36, 12) |
            ECK_LRCP_BITNUM(in, 28, 11) | ECK_LRCP_BITNUM(in, 20, 10) | ECK_LRCP_BITNUM(in, 12, 9) | ECK_LRCP_BITNUM(in, 4, 8) |
            ECK_LRCP_BITNUM(in, 62, 7) | ECK_LRCP_BITNUM(in, 54, 6) | ECK_LRCP_BITNUM(in, 46, 5) | ECK_LRCP_BITNUM(in, 38, 4) |
            ECK_LRCP_BITNUM(in, 30, 3) | ECK_LRCP_BITNUM(in, 22, 2) | ECK_LRCP_BITNUM(in, 14, 1) | ECK_LRCP_BITNUM(in, 6, 0);
    }
    static void InvIP(DES_WORD state[], BYTE in[])
    {
        in[3] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 7, 7) | ECK_LRCP_BITNUMINTR(state[0], 7, 6) | ECK_LRCP_BITNUMINTR(state[1], 15, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 15, 4) | ECK_LRCP_BITNUMINTR(state[1], 23, 3) | ECK_LRCP_BITNUMINTR(state[0], 23, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 31, 1) | ECK_LRCP_BITNUMINTR(state[0], 31, 0));
        in[2] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 6, 7) | ECK_LRCP_BITNUMINTR(state[0], 6, 6) | ECK_LRCP_BITNUMINTR(state[1], 14, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 14, 4) | ECK_LRCP_BITNUMINTR(state[1], 22, 3) | ECK_LRCP_BITNUMINTR(state[0], 22, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 30, 1) | ECK_LRCP_BITNUMINTR(state[0], 30, 0));
        in[1] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 5, 7) | ECK_LRCP_BITNUMINTR(state[0], 5, 6) | ECK_LRCP_BITNUMINTR(state[1], 13, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 13, 4) | ECK_LRCP_BITNUMINTR(state[1], 21, 3) | ECK_LRCP_BITNUMINTR(state[0], 21, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 29, 1) | ECK_LRCP_BITNUMINTR(state[0], 29, 0));
        in[0] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 4, 7) | ECK_LRCP_BITNUMINTR(state[0], 4, 6) | ECK_LRCP_BITNUMINTR(state[1], 12, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 12, 4) | ECK_LRCP_BITNUMINTR(state[1], 20, 3) | ECK_LRCP_BITNUMINTR(state[0], 20, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 28, 1) | ECK_LRCP_BITNUMINTR(state[0], 28, 0));
        in[7] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 3, 7) | ECK_LRCP_BITNUMINTR(state[0], 3, 6) | ECK_LRCP_BITNUMINTR(state[1], 11, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 11, 4) | ECK_LRCP_BITNUMINTR(state[1], 19, 3) | ECK_LRCP_BITNUMINTR(state[0], 19, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 27, 1) | ECK_LRCP_BITNUMINTR(state[0], 27, 0));
        in[6] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 2, 7) | ECK_LRCP_BITNUMINTR(state[0], 2, 6) | ECK_LRCP_BITNUMINTR(state[1], 10, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 10, 4) | ECK_LRCP_BITNUMINTR(state[1], 18, 3) | ECK_LRCP_BITNUMINTR(state[0], 18, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 26, 1) | ECK_LRCP_BITNUMINTR(state[0], 26, 0));
        in[5] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 1, 7) | ECK_LRCP_BITNUMINTR(state[0], 1, 6) | ECK_LRCP_BITNUMINTR(state[1], 9, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 9, 4) | ECK_LRCP_BITNUMINTR(state[1], 17, 3) | ECK_LRCP_BITNUMINTR(state[0], 17, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 25, 1) | ECK_LRCP_BITNUMINTR(state[0], 25, 0));
        in[4] = (byte)(ECK_LRCP_BITNUMINTR(state[1], 0, 7) | ECK_LRCP_BITNUMINTR(state[0], 0, 6) | ECK_LRCP_BITNUMINTR(state[1], 8, 5) |
            ECK_LRCP_BITNUMINTR(state[0], 8, 4) | ECK_LRCP_BITNUMINTR(state[1], 16, 3) | ECK_LRCP_BITNUMINTR(state[0], 16, 2) |
            ECK_LRCP_BITNUMINTR(state[1], 24, 1) | ECK_LRCP_BITNUMINTR(state[0], 24, 0));
    }
    static DES_WORD f(DES_WORD state, const BYTE key[])
    {
        BYTE lrgstate[6]; //,i;
        DES_WORD t1, t2;
        t1 = ECK_LRCP_BITNUMINTL(state, 31, 0) | ((state & 0xf0000000) >> 1) | ECK_LRCP_BITNUMINTL(state, 4, 5) |
            ECK_LRCP_BITNUMINTL(state, 3, 6) | ((state & 0x0f000000) >> 3) | ECK_LRCP_BITNUMINTL(state, 8, 11) |
            ECK_LRCP_BITNUMINTL(state, 7, 12) | ((state & 0x00f00000) >> 5) | ECK_LRCP_BITNUMINTL(state, 12, 17) |
            ECK_LRCP_BITNUMINTL(state, 11, 18) | ((state & 0x000f0000) >> 7) | ECK_LRCP_BITNUMINTL(state, 16, 23);
        t2 = ECK_LRCP_BITNUMINTL(state, 15, 0) | ((state & 0x0000f000) << 15) | ECK_LRCP_BITNUMINTL(state, 20, 5) |
            ECK_LRCP_BITNUMINTL(state, 19, 6) | ((state & 0x00000f00) << 13) | ECK_LRCP_BITNUMINTL(state, 24, 11) |
            ECK_LRCP_BITNUMINTL(state, 23, 12) | ((state & 0x000000f0) << 11) | ECK_LRCP_BITNUMINTL(state, 28, 17) |
            ECK_LRCP_BITNUMINTL(state, 27, 18) | ((state & 0x0000000f) << 9) | ECK_LRCP_BITNUMINTL(state, 0, 23);
        lrgstate[0] = (t1 >> 24) & 0x000000ff;
        lrgstate[1] = (t1 >> 16) & 0x000000ff;
        lrgstate[2] = (t1 >> 8) & 0x000000ff;
        lrgstate[3] = (t2 >> 24) & 0x000000ff;
        lrgstate[4] = (t2 >> 16) & 0x000000ff;
        lrgstate[5] = (t2 >> 8) & 0x000000ff;
        lrgstate[0] ^= key[0];
        lrgstate[1] ^= key[1];
        lrgstate[2] ^= key[2];
        lrgstate[3] ^= key[3];
        lrgstate[4] ^= key[4];
        lrgstate[5] ^= key[5];
        state = (sbox1[ECK_LRCP_SBOXBIT(lrgstate[0] >> 2)] << 28) |
            (sbox2[ECK_LRCP_SBOXBIT(((lrgstate[0] & 0x03) << 4) | (lrgstate[1] >> 4))] << 24) |
            (sbox3[ECK_LRCP_SBOXBIT(((lrgstate[1] & 0x0f) << 2) | (lrgstate[2] >> 6))] << 20) |
            (sbox4[ECK_LRCP_SBOXBIT(lrgstate[2] & 0x3f)] << 16) |
            (sbox5[ECK_LRCP_SBOXBIT(lrgstate[3] >> 2)] << 12) |
            (sbox6[ECK_LRCP_SBOXBIT(((lrgstate[3] & 0x03) << 4) | (lrgstate[4] >> 4))] << 8) |
            (sbox7[ECK_LRCP_SBOXBIT(((lrgstate[4] & 0x0f) << 2) | (lrgstate[5] >> 6))] << 4) |
            sbox8[ECK_LRCP_SBOXBIT(lrgstate[5] & 0x3f)];
        state = ECK_LRCP_BITNUMINTL(state, 15, 0) | ECK_LRCP_BITNUMINTL(state, 6, 1) | ECK_LRCP_BITNUMINTL(state, 19, 2) |
            ECK_LRCP_BITNUMINTL(state, 20, 3) | ECK_LRCP_BITNUMINTL(state, 28, 4) | ECK_LRCP_BITNUMINTL(state, 11, 5) |
            ECK_LRCP_BITNUMINTL(state, 27, 6) | ECK_LRCP_BITNUMINTL(state, 16, 7) | ECK_LRCP_BITNUMINTL(state, 0, 8) |
            ECK_LRCP_BITNUMINTL(state, 14, 9) | ECK_LRCP_BITNUMINTL(state, 22, 10) | ECK_LRCP_BITNUMINTL(state, 25, 11) |
            ECK_LRCP_BITNUMINTL(state, 4, 12) | ECK_LRCP_BITNUMINTL(state, 17, 13) | ECK_LRCP_BITNUMINTL(state, 30, 14) |
            ECK_LRCP_BITNUMINTL(state, 9, 15) | ECK_LRCP_BITNUMINTL(state, 1, 16) | ECK_LRCP_BITNUMINTL(state, 7, 17) |
            ECK_LRCP_BITNUMINTL(state, 23, 18) | ECK_LRCP_BITNUMINTL(state, 13, 19) | ECK_LRCP_BITNUMINTL(state, 31, 20) |
            ECK_LRCP_BITNUMINTL(state, 26, 21) | ECK_LRCP_BITNUMINTL(state, 2, 22) | ECK_LRCP_BITNUMINTL(state, 8, 23) |
            ECK_LRCP_BITNUMINTL(state, 18, 24) | ECK_LRCP_BITNUMINTL(state, 12, 25) | ECK_LRCP_BITNUMINTL(state, 29, 26) |
            ECK_LRCP_BITNUMINTL(state, 5, 27) | ECK_LRCP_BITNUMINTL(state, 21, 28) | ECK_LRCP_BITNUMINTL(state, 10, 29) |
            ECK_LRCP_BITNUMINTL(state, 3, 30) | ECK_LRCP_BITNUMINTL(state, 24, 31);
        return(state);
    }
    static void des_key_setup(const BYTE key[], BYTE schedule[][6], QRC_DES_MODE mode)
    {
        DES_WORD i, j, to_gen, C, D;
        const DES_WORD key_rnd_shift[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };
        const DES_WORD key_perm_c[28] = { 56,48,40,32,24,16,8,0,57,49,41,33,25,17,9,1,58,50,42,34,26,18,10,2,59,51,43,35 };
        const DES_WORD key_perm_d[28] = { 62,54,46,38,30,22,14,6,61,53,45,37,29,21,13,5,60,52,44,36,28,20,12,4,27,19,11,3 };
        const DES_WORD key_compression[48] = { 13,16,10,23,0,4,2,27,14,5,20,9,22,18,11,3,25,7,15,6,26,19,12,1,
                40,51,30,36,46,54,29,39,50,44,32,47,43,48,38,55,33,52,45,41,49,35,28,31 };
        for (i = 0, j = 31, C = 0; i < 28; ++i, --j)
            C |= ECK_LRCP_BITNUM(key, key_perm_c[i], j);
        for (i = 0, j = 31, D = 0; i < 28; ++i, --j)
            D |= ECK_LRCP_BITNUM(key, key_perm_d[i], j);
        for (i = 0; i < 16; ++i) {
            C = ((C << key_rnd_shift[i]) | (C >> (28 - key_rnd_shift[i]))) & 0xfffffff0;
            D = ((D << key_rnd_shift[i]) | (D >> (28 - key_rnd_shift[i]))) & 0xfffffff0;
            if (mode == QRC_DES_DECRYPT)
                to_gen = 15 - i;
            else
                to_gen = i;
            for (j = 0; j < 6; ++j)
                schedule[to_gen][j] = 0;
            for (j = 0; j < 24; ++j)
                schedule[to_gen][j / 8] |= ECK_LRCP_BITNUMINTR(C, key_compression[j], 7 - (j % 8));
            for (; j < 48; ++j)
                schedule[to_gen][j / 8] |= ECK_LRCP_BITNUMINTR(D, key_compression[j] - 27, 7 - (j % 8));
        }
    }
    static void des_crypt(const BYTE in[], BYTE out[], const BYTE key[][6])
    {
        DES_WORD state[2], idx, t;
        IP(state, in);
        for (idx = 0; idx < 15; ++idx) {
            t = state[1];
            state[1] = f(state[1], key[idx]) ^ state[0];
            state[0] = t;
        }
        state[0] = f(state[1], key[15]) ^ state[0];
        InvIP(state, out);
    }
    static void Qrc3DesSetupKey(const BYTE key[], BYTE schedule[][16][6], QRC_DES_MODE mode) noexcept
    {
        if (mode == QRC_DES_ENCRYPT) {
            des_key_setup(&key[0], schedule[0], mode);
            des_key_setup(&key[8], schedule[1], QRC_DES_DECRYPT);
            des_key_setup(&key[16], schedule[2], mode);
        }
        else {
            des_key_setup(&key[16], schedule[0], mode);
            des_key_setup(&key[8], schedule[1], QRC_DES_ENCRYPT);
            des_key_setup(&key[0], schedule[2], mode);
        }
    }
    static void Qrc3DesCrypt(const BYTE in[], BYTE out[], const BYTE key[][16][6]) noexcept
    {
        des_crypt(in, out, key[0]);
        des_crypt(out, out, key[1]);
        des_crypt(out, out, key[2]);
    }

    inline void Qmc1Decrypt(BYTE* p, size_t cb) noexcept
    {
        constexpr BYTE Key[128]
        {
            0xc3, 0x4a, 0xd6, 0xca, 0x90, 0x67, 0xf7, 0x52,
            0xd8, 0xa1, 0x66, 0x62, 0x9f, 0x5b, 0x09, 0x00,
            0xc3, 0x5e, 0x95, 0x23, 0x9f, 0x13, 0x11, 0x7e,
            0xd8, 0x92, 0x3f, 0xbc, 0x90, 0xbb, 0x74, 0x0e,
            0xc3, 0x47, 0x74, 0x3d, 0x90, 0xaa, 0x3f, 0x51,
            0xd8, 0xf4, 0x11, 0x84, 0x9f, 0xde, 0x95, 0x1d,
            0xc3, 0xc6, 0x09, 0xd5, 0x9f, 0xfa, 0x66, 0xf9,
            0xd8, 0xf0, 0xf7, 0xa0, 0x90, 0xa1, 0xd6, 0xf3,
            0xc3, 0xf3, 0xd6, 0xa1, 0x90, 0xa0, 0xf7, 0xf0,
            0xd8, 0xf9, 0x66, 0xfa, 0x9f, 0xd5, 0x09, 0xc6,
            0xc3, 0x1d, 0x95, 0xde, 0x9f, 0x84, 0x11, 0xf4,
            0xd8, 0x51, 0x3f, 0xaa, 0x90, 0x3d, 0x74, 0x47,
            0xc3, 0x0e, 0x74, 0xbb, 0x90, 0xbc, 0x3f, 0x92,
            0xd8, 0x7e, 0x11, 0x13, 0x9f, 0x23, 0x95, 0x5e,
            0xc3, 0x00, 0x09, 0x5b, 0x9f, 0x62, 0x66, 0xa1,
            0xd8, 0x52, 0xf7, 0x67, 0x90, 0xca, 0xd6, 0x4a,
        };

        for (size_t i = 0; i < cb; ++i)
            p[i] ^= (i > 0x7fff) ? Key[(i % 0x7fff) & 0x7f] : Key[i & 0x7f];
    }
}

inline BOOL KrcDecrypt(_Inout_bytecount_(cb) void* p, size_t cb, CRefBin& rbResult) noexcept
{
    if (!p || cb <= 4)
        return FALSE;
    if (memcmp(p, "krc1", 4) != 0)
        return FALSE;
    p = (BYTE*)p + 4;
    cb -= 4;
    constexpr BYTE Key[]{ 64,71,97,119,94,50,116,71,81,54,49,45,206,210,110,105 };
    for (size_t i = 0; i < cb; ++i)
        ((BYTE*)p)[i] ^= Key[i % ARRAYSIZE(Key)];
    return ZLibSuccess(ZLibDecompress(p, cb, rbResult));
}

inline BOOL QrcDecrypt(_Inout_bytecount_(cb) void* p, size_t cb, CRefBin& rbResult) noexcept
{
    if (!p || cb <= 11)
        return FALSE;
    Priv::Qmc1Decrypt((BYTE*)p, cb);
    p = (BYTE*)p + 11;
    cb -= 11;
    if (cb % 8)
        return FALSE;
    constexpr BYTE Key[]{ "!@#)(*$%123ZXC!@!@#)(NHL" };
    BYTE Sch[3][16][6]{}, Out[8];
    Priv::Qrc3DesSetupKey(Key, Sch, Priv::QRC_DES_DECRYPT);
    for (auto q = (BYTE*)p; q < (BYTE*)p + cb; q += 8)
    {
        Priv::Qrc3DesCrypt(q, Out, Sch);
        memcpy(q, Out, sizeof(Out));
    }
    return ZLibSuccess(ZLibDecompress(p, cb, rbResult));
}
ECK_LYRIC_NAMESPACE_END
ECK_NAMESPACE_END