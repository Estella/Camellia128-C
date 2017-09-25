#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define u8 uint8_t

//iterators
int i = 0, j = 0, jj = 0;

//data structures (W is a 32-bit word, L 64 and Q 128)
struct W {
   u8 b[4];
};

struct L {
   struct W w[2];
};

struct Q {
   struct L l[2];
};

struct Q2 {
    struct Q q[2];
};

//utilities
struct Q zeroQ = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

struct Q tab_to_Q(u8 *t, int n) {

   struct Q res;

   for(i = 0; i < n; i++) {
      res.l[i/8].w[(i%8)/4].b[i%4] = t[i];
   }

   return res;
}

void printL(struct L l) {

   int i, j;
   for(i = 0; i < 2; i++)
      for(j = 0; j < 4; j++)
         printf("%02X", l.w[i].b[j]);

   printf("\n");
}

void printQ(struct Q q) {

   int i, j, jj;
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++)
            printf("%02X", q.l[i].w[j].b[jj]);

   printf("\n");
}

struct W and_W(struct W w, struct W k) {

   struct W res;
   int i;

   for(i = 0; i < 4; i++) {
      res.b[i] = w.b[i] & k.b[i];
   }

   return res;
}

struct W or_W(struct W w, struct W k) {

   struct W res;
   int i;

   for(i = 0; i < 4; i++) {
      res.b[i] = w.b[i] | k.b[i];
   }

   return res;
}

struct W xor_W(struct W w, struct W k) {

   struct W res;
   int i;

   for(i = 0; i < 4; i++) {
      res.b[i] = w.b[i] ^ k.b[i];
   }

   return res;
}

struct L xor_L(struct L l, struct L k) {

   struct L res;
   //printf("xorL:");
   //printL(l);
   //printL(k);

   res.w[0] = xor_W(l.w[0], k.w[0]);
   res.w[1] = xor_W(l.w[1], k.w[1]);

   //printL(res);

   return res;
}

struct Q xor_Q(struct Q q, struct Q k) {

   struct Q res;

   res.l[0] = xor_L(q.l[0], k.l[0]);
   res.l[1] = xor_L(q.l[1], k.l[1]);

   return res;
}

struct W shiftl_W(struct W w) {

   struct W res;
   int i;
   u8 tmp[4];

   for(i = 0; i < 4; i++) {
      tmp[i] = (w.b[i] & 0x80) >> 7;
   }

   for(i = 0; i < 4; i++) {
      res.b[i] = (w.b[i] << 1) ^ tmp[(i+1)%4];
   }

   return res;
}

//declarations of sboxes
u8 sbox1[256] = {
   112, 130,  44, 236, 179,  39, 192, 229, 228, 133,  87,  53, 234,  12, 174,  65,
    35, 239, 107, 147,  69,  25, 165,  33, 237,  14,  79,  78,  29, 101, 146, 189,
   134, 184, 175, 143, 124, 235,  31, 206,  62,  48, 220,  95,  94, 197,  11,  26,
   166, 225,  57, 202, 213,  71,  93,  61, 217,   1,  90, 214,  81,  86, 108,  77,
   139,  13, 154, 102, 251, 204, 176,  45, 116,  18,  43,  32, 240, 177, 132, 153,
   223,  76, 203, 194,  52, 126, 118,   5, 109, 183, 169,  49, 209,  23,   4, 215,
    20,  88,  58,  97, 222,  27,  17,  28,  50,  15, 156,  22,  83,  24, 242,  34,
   254,  68, 207, 178, 195, 181, 122, 145,  36,   8, 232, 168,  96, 252, 105,  80,
   170, 208, 160, 125, 161, 137,  98, 151,  84,  91,  30, 149, 224, 255, 100, 210,
    16, 196,   0,  72, 163, 247, 117, 219, 138,   3, 230, 218,   9,  63, 221, 148,
   135,  92, 131,   2, 205,  74, 144,  51, 115, 103, 246, 243, 157, 127, 191, 226,
    82, 155, 216,  38, 200,  55, 198,  59, 129, 150, 111,  75,  19, 190,  99,  46,
   233, 121, 167, 140, 159, 110, 188, 142,  41, 245, 249, 182,  47, 253, 180,  89,
   120, 152,   6, 106, 231,  70, 113, 186, 212,  37, 171,  66, 136, 162, 141, 250,
   114,   7, 185,  85, 248, 238, 172,  10,  54,  73,  42, 104,  60,  56, 241, 164,
    64,  40, 211, 123, 187, 201,  67, 193,  21, 227, 173, 244, 119, 199, 128, 158
};
u8 sbox2[256];
u8 sbox3[256];
u8 sbox4[256];

//key schedule constants
struct L sigma_tab[] = {

      0xA0, 0x9E, 0x66, 0x7F, 0x3B, 0xCC, 0x90, 0x8B
   ,
      0xB6, 0x7A, 0xE8, 0x58, 0x4C, 0xAA, 0x73, 0xB2
   ,
      0xC6, 0xEF, 0x37, 0x2F, 0xE9, 0x4F, 0x82, 0xBE
   ,
      0x54, 0xFF, 0x53, 0xA5, 0xF1, 0xD3, 0x6F, 0x1C
   ,
      0x10, 0xE5, 0x27, 0xFA, 0xDE, 0x68, 0x2D, 0x1D
   ,
      0xB0, 0x56, 0x88, 0xC2, 0xB3, 0xE6, 0xC1, 0xFD

};

//camellia functions
struct L func_S(struct L l) {

   struct L res;

   //printf("l:");
   //printL(l);
   res.w[0].b[0] = sbox1[l.w[0].b[0]];
   res.w[0].b[1] = sbox2[l.w[0].b[1]];
   res.w[0].b[2] = sbox3[l.w[0].b[2]];
   res.w[0].b[3] = sbox4[l.w[0].b[3]];
   res.w[1].b[0] = sbox2[l.w[1].b[0]];
   res.w[1].b[1] = sbox3[l.w[1].b[1]];
   res.w[1].b[2] = sbox4[l.w[1].b[2]];
   res.w[1].b[3] = sbox1[l.w[1].b[3]];

   return res;
}

struct L func_P(struct L l) {

   struct L res;

   res.w[0].b[0] = l.w[0].b[0] ^ l.w[0].b[2] ^ l.w[0].b[3] ^ l.w[1].b[1] ^ l.w[1].b[2] ^ l.w[1].b[3];
   res.w[0].b[1] = l.w[0].b[0] ^ l.w[0].b[1] ^ l.w[0].b[3] ^ l.w[1].b[0] ^ l.w[1].b[2] ^ l.w[1].b[3];
   res.w[0].b[2] = l.w[0].b[0] ^ l.w[0].b[1] ^ l.w[0].b[2] ^ l.w[1].b[0] ^ l.w[1].b[1] ^ l.w[1].b[3];
   res.w[0].b[3] = l.w[0].b[1] ^ l.w[0].b[2] ^ l.w[0].b[3] ^ l.w[1].b[0] ^ l.w[1].b[1] ^ l.w[1].b[2];
   res.w[1].b[0] = l.w[0].b[0] ^ l.w[0].b[1] ^ l.w[1].b[1] ^ l.w[1].b[2] ^ l.w[1].b[3];
   res.w[1].b[1] = l.w[0].b[1] ^ l.w[0].b[2] ^ l.w[1].b[0] ^ l.w[1].b[2] ^ l.w[1].b[3];
   res.w[1].b[2] = l.w[0].b[2] ^ l.w[0].b[3] ^ l.w[1].b[0] ^ l.w[1].b[1] ^ l.w[1].b[3];
   res.w[1].b[3] = l.w[0].b[0] ^ l.w[0].b[3] ^ l.w[1].b[0] ^ l.w[1].b[1] ^ l.w[1].b[2];

   return res;
}

struct L func_F(struct L l, struct L k) {

   //return func_P(func_S(xor_L(l, k)));
   struct L res;
   //printf("l: ");
   //printL(l);
   //printf("k: ");
   //printL(k);
   res = xor_L(l, k);
   //printf("xor: ");
   //printL(res);
   res = func_S(res);
   //printf("S: ");
   //printL(res);
   res = func_P(res);
   //printf("P: ");
   //printL(res);
   return res;
}

struct L func_FL(struct L l, struct L k) {
   //printf("FL: ");

   struct W tmp1 = l.w[0], tmp2 = l.w[1];
   struct L res;

   res.w[1] = xor_W(shiftl_W(and_W(tmp1, k.w[0])), tmp2);
   res.w[0] = xor_W(or_W(res.w[1], k.w[1]), tmp1);

   //printL(res);
   return res;
}

struct L func_FL_1(struct L l, struct L k) {
   //printf("FL_1: ");

   struct W tmp1 = l.w[0], tmp2 = l.w[1];
   struct L res;

   res.w[0] = xor_W(or_W(tmp2, k.w[1]), tmp1);
   res.w[1] = xor_W(shiftl_W(and_W(res.w[0], k.w[0])), tmp2);

   //printL(res);
   return res;
}

struct Q func_round(struct Q q, struct L *k, int nr) {
   //printf("round %d:\n", i+1);

   struct Q res;

   res.l[0] = xor_L(q.l[1], func_F(q.l[0], k[nr]));
   res.l[1] = q.l[0];

   return res;
}

//key schedule routine
void get_Ka(struct Q *Ka, struct Q Kl, struct Q Kr) {

   *Ka = func_round(xor_Q(Kl, Kr), sigma_tab, 0);
   //printf("Ka: 1r\n");
   //printQ(*Ka);
   *Ka = func_round(*Ka, sigma_tab, 1);
   //printf("Ka: 2r\n");
   //printQ(*Ka);
   *Ka = xor_Q(*Ka, Kl);
   //printf("Ka: xor Kl\n");
   //printQ(*Ka);
   *Ka = func_round(*Ka, sigma_tab, 2);
   //printf("Ka: 3r\n");
   //printQ(*Ka);
   *Ka = func_round(*Ka, sigma_tab, 3);
   //printf("Ka: 4r\n");
   //printQ(*Ka);
}

void get_Ka_Kb(struct Q *Ka, struct Q *Kb, struct Q Kl, struct Q Kr) {

   *Ka = func_round(xor_Q(Kl, Kr), sigma_tab, 0);
   *Ka = func_round(*Ka, sigma_tab, 1);
   *Ka = xor_Q(*Ka, Kl);
   *Ka = func_round(*Ka, sigma_tab, 2);
   *Ka = func_round(*Ka, sigma_tab, 3);

   *Kb = xor_Q(*Ka, Kr);
   *Kb = func_round(*Kb, sigma_tab, 4);
   *Kb = func_round(*Kb, sigma_tab, 5);
}

struct Q shiftl_l15(struct Q K) {

   u8 tmp[16];
   int i, j, jj;
   //copying to tmps
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = K.l[i].w[j].b[jj];
         }
   //shifting by 2 bytes left
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = tmp[(8*i + 4*j + jj + 2) % 16];
         }
   //shifting by 1 bit right
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = (K.l[i].w[j].b[jj] & 0x01) << 7;//saving the right-most bit of each byte and transporting it to left-most position
         }
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = (K.l[i].w[j].b[jj] >> 1) ^ tmp[(8*i + 4*j + jj + 15) % 16];//xoring rotated byte with previously saved bit
         }

   return K;
}

struct Q shiftl_l30(struct Q K) {

   u8 tmp[16];
   int i, j, jj;
   //copying to tmps
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = K.l[i].w[j].b[jj];
         }
   //shifting by 4 bytes left
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = tmp[(8*i + 4*j + jj + 4) % 16];
         }
   //shifting by 2 bit right
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = (K.l[i].w[j].b[jj] & 0x03) << 6;//saving 2 right-most bits of each byte and transporting them to left-most position
         }
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = (K.l[i].w[j].b[jj] >> 2) ^ tmp[(8*i + 4*j + jj + 15) % 16];//xoring rotated byte with previously saved bits
         }

   return K;
}

struct Q shiftl_l45(struct Q K) {

   u8 tmp[16];
   int i, j, jj;
   //copying to tmps
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = K.l[i].w[j].b[jj];
         }
   //shifting by 6 bytes left
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = tmp[(8*i + 4*j + jj + 6) % 16];
         }
   //shifting by 3 bit right
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = (K.l[i].w[j].b[jj] & 0x07) << 5;//saving 3 right-most bits of each byte and transporting them to left-most position
         }
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = (K.l[i].w[j].b[jj] >> 3) ^ tmp[(8*i + 4*j + jj + 15) % 16];//xoring rotated byte with previously saved bits
         }

   return K;
}

struct Q shiftl_l60(struct Q K) {

   u8 tmp[16];
   int i, j, jj;
   //copying to tmps
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = K.l[i].w[j].b[jj];
         }
   //shifting by 8 bytes left
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = tmp[(8*i + 4*j + jj + 8) % 16];
         }
   //shifting by 4 bit right
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = (K.l[i].w[j].b[jj] & 0x0F) << 4;//saving 4 right-most bits of each byte and transporting them to left-most position
         }
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = (K.l[i].w[j].b[jj] >> 4) ^ tmp[(8*i + 4*j + jj + 15) % 16];//xoring rotated byte with previously saved bits
         }

   return K;
}

struct Q shiftl_l77(struct Q K) {

   u8 tmp[16];
   int i, j, jj;
   //copying to tmps
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = K.l[i].w[j].b[jj];
         }
   //shifting by 10 bytes left
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = tmp[(8*i + 4*j + jj + 10) % 16];
         }
   //shifting by 3 bit right
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = (K.l[i].w[j].b[jj] & 0x07) << 5;//saving 3 right-most bits of each byte and transporting them to left-most position
         }
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = (K.l[i].w[j].b[jj] >> 3) ^ tmp[(8*i + 4*j + jj + 15) % 16];//xoring rotated byte with previously saved bits
         }

   return K;
}

struct Q shiftl_l94(struct Q K) {

   u8 tmp[16];
   int i, j, jj;
   //copying to tmps
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = K.l[i].w[j].b[jj];
         }
   //shifting by 12 bytes left
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = tmp[(8*i + 4*j + jj + 12) % 16];
         }
   //shifting by 2 bit right
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = (K.l[i].w[j].b[jj] & 0x03) << 6;//saving 2 right-most bits of each byte and transporting them to left-most position
         }
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = (K.l[i].w[j].b[jj] >> 2) ^ tmp[(8*i + 4*j + jj + 15) % 16];//xoring rotated byte with previously saved bits
         }

   return K;
}

struct Q shiftl_l111(struct Q K) {

   u8 tmp[16];
   int i, j, jj;
   //copying to tmps
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = K.l[i].w[j].b[jj];
         }
   //shifting by 14 bytes left
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = tmp[(8*i + 4*j + jj + 14) % 16];
         }
   //shifting by 1 bit right
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            tmp[8*i + 4*j + jj] = (K.l[i].w[j].b[jj] & 0x01) << 7;//saving the right-most bit of each byte and transporting it to left-most position
         }
   for(i = 0; i < 2; i++)
      for(j = 0; j < 2; j++)
         for(jj = 0; jj < 4; jj++) {
            K.l[i].w[j].b[jj] = (K.l[i].w[j].b[jj] >> 1) ^ tmp[(8*i + 4*j + jj + 15) % 16];//xoring rotated byte with previously saved bit
         }

   return K;
}

void gen_keys128(struct L *kw, struct L *k, struct L *kl, struct Q key) {

   struct Q Ka;
   get_Ka(&Ka, key, zeroQ);
   //extracting values (4) to kw
   *kw = key.l[0];
   *(kw+1) = key.l[1];
   *(kw+2) = shiftl_l111(Ka).l[0];
   *(kw+3) = shiftl_l111(Ka).l[1];
   //extracting values (18) to k
   *k = Ka.l[0];
   *(k+1) = Ka.l[1];
   *(k+2) = shiftl_l15(key).l[0];
   *(k+3) = shiftl_l15(key).l[1];
   *(k+4) = shiftl_l15(Ka).l[0];
   *(k+5) = shiftl_l15(Ka).l[1];
   *(k+6) = shiftl_l45(key).l[0];
   *(k+7) = shiftl_l45(key).l[1];
   *(k+8) = shiftl_l45(Ka).l[0];
   *(k+9) = shiftl_l60(key).l[1];
   *(k+10) = shiftl_l60(Ka).l[0];
   *(k+11) = shiftl_l60(Ka).l[1];
   *(k+12) = shiftl_l94(key).l[0];
   *(k+13) = shiftl_l94(key).l[1];
   *(k+14) = shiftl_l94(Ka).l[0];
   *(k+15) = shiftl_l94(Ka).l[1];
   *(k+16) = shiftl_l111(key).l[0];
   *(k+17) = shiftl_l111(key).l[1];
   //extracting values (4) to kl
   *kl = shiftl_l30(Ka).l[0];
   *(kl+1) = shiftl_l30(Ka).l[1];
   *(kl+2) = shiftl_l77(key).l[0];
   *(kl+3) = shiftl_l77(key).l[1];
}

void gen_keys192(struct L *kw, struct L *k, struct L *kl, struct Q2 key) {

    struct Q Ka, Kb;
    //get_Ka_Kb(&Ka, &Kb, key.q[0], );
}

void gen_keys256(struct L *kw, struct L *k, struct L *kl, struct Q2 key) {

    struct Q Ka, Kb;
}

int main(int argc, char **argv) {
   //printing sigma_tab
   for(i = 0; i < 6; i++) {
      printf("sigma %d = ", i);
      printL(sigma_tab[i]);
   }

   //plaintext
   char *pp = argv[3];
   u8 plaintext[16];
   for(i = 0; i < 16; i++) {
      sscanf(pp, "%2hhx", &plaintext[i]);
      pp += 2;
      //printf("%02X", plaintext[i]);
   }
   struct Q pQ = tab_to_Q(plaintext, 16);
   printf("plaintext = ");
   printQ(pQ);

   //creating sboxes
   for(i = 0; i < 256; i++) sbox2[i] = (sbox1[i] << 1) | (sbox1[i] >> 7);
   for(i = 0; i < 256; i++) sbox3[i] = (sbox1[i] << 7) | (sbox1[i] >> 1);
   for(i = 0; i < 256; i++) sbox4[i] = sbox1[((i << 1) % 256) | (i >> 7)];

   //choise of the mode (encryption or decryption) and key length (128, 192 or 256)
   if(!(strcmp(argv[1], "-e"))){
      printf("encryption mode selected\n");

      if(!(strcmp(argv[2], "-128"))){ //encryption with 128-bit key
         //key
         char *kp = argv[4];
         u8 key[16];
         for(i = 0; i < 16; i++) {
            sscanf(kp, "%2hhx", &key[i]);
            kp += 2;
            //printf("%02X", key[i]);
         }
         //printf("\n");
         struct Q keyQ = tab_to_Q(key, 16);
         printf("key: ");
         printQ(keyQ);

         //key schedule
         struct L kw[4], k[18], kl[4];
         gen_keys128(kw, k, kl, keyQ);
         //printing keys
         printf("kw: \n");
         for(i = 0; i < 4; i++)
            printL(kw[i]);
         printf("k: \n");
         for(i = 0; i < 18; i++)
            printL(k[i]);
         printf("kl: \n");
         for(i = 0; i < 4; i++)
            printL(kl[i]);
         //encryption
         pQ.l[0] = xor_L(pQ.l[0], kw[0]);
         pQ.l[1] = xor_L(pQ.l[1], kw[1]);
         for(i = 0; i < 6; i++) {
            pQ = func_round(pQ, k, i);
         }
         pQ.l[0] = func_FL(pQ.l[0], kl[0]);
         pQ.l[1] = func_FL_1(pQ.l[1], kl[1]);
         for(i = 6; i < 12; i++) {
            pQ = func_round(pQ, k, i);
         }
         pQ.l[0] = func_FL(pQ.l[0], kl[2]);
         pQ.l[1] = func_FL_1(pQ.l[1], kl[3]);
         for(i = 12; i < 18; i++) {
            pQ = func_round(pQ, k, i);
         }
         struct L tmp = pQ.l[0];
         pQ.l[0] = xor_L(pQ.l[1], kw[2]);
         pQ.l[1] = xor_L(tmp, kw[3]);

         //printing result
         printQ(pQ);
      }
      else if(!(strcmp(argv[2], "-192"))){ //encryption with 192-bit key
         //key
         char *kp = argv[4];
         u8 key[24];
         for(i = 0; i < 24; i++) {
            sscanf(kp, "%2hhx", &key[i]);
            kp += 2;
            //printf("%02X", key[i]);
         }

      }
      else if(!(strcmp(argv[2], "-256"))){ //encryption with 256-bit key
         //key
         char *kp = argv[4];
         u8 key[32];
         for(i = 0; i < 32; i++) {
            sscanf(kp, "%2hhx", &key[i]);
            kp += 2;
            //printf("%02X", key[i]);
         }

      }
      else {
         printf("Wrong key length - please choose '-128', '-192' or '-256'\n");
      }
   }
   else if(!(strcmp(argv[1], "-d"))){
      printf("decryption mode selected\n");

      if(!(strcmp(argv[2], "-128"))){ //decryption with 128-bit key
         //key
         char *kp = argv[4];
         u8 key[16];
         for(i = 0; i < 16; i++) {
            sscanf(kp, "%2hhx", &key[i]);
            kp += 2;
            //printf("%02X", key[i]);
         }
         //printf("\n");
         struct Q keyQ = tab_to_Q(key, 16);
         printf("key: ");
         printQ(keyQ);

         //key schedule
         struct L kw[4], k[18], kl[4];
         gen_keys128(kw, k, kl, keyQ);
         //printing keys
         printf("kw: \n");
         for(i = 0; i < 4; i++)
            printL(kw[i]);
         printf("k: \n");
         for(i = 0; i < 18; i++)
            printL(k[i]);
         printf("kl: \n");
         for(i = 0; i < 4; i++)
            printL(kl[i]);
         //decryption
         pQ.l[0] = xor_L(pQ.l[0], kw[2]);
         pQ.l[1] = xor_L(pQ.l[1], kw[3]);
         for(i = 17; i > 11; i--) {
            pQ = func_round(pQ, k, i);
         }
         pQ.l[0] = func_FL(pQ.l[0], kl[3]);
         pQ.l[1] = func_FL_1(pQ.l[1], kl[2]);
         for(i = 11; i > 5; i--) {
            pQ = func_round(pQ, k, i);
         }
         pQ.l[0] = func_FL(pQ.l[0], kl[1]);
         pQ.l[1] = func_FL_1(pQ.l[1], kl[0]);
         for(i = 5; i >= 0; i--) {
            pQ = func_round(pQ, k, i);
         }
         struct L tmp = pQ.l[0];
         pQ.l[0] = xor_L(pQ.l[1], kw[0]);
         pQ.l[1] = xor_L(tmp, kw[1]);

         //printing result
         printQ(pQ);
      }
      else if(!(strcmp(argv[2], "-192"))){ //decryption with 192-bit key
         //key
         char *kp = argv[4];
         u8 key[24];
         for(i = 0; i < 24; i++) {
            sscanf(kp, "%2hhx", &key[i]);
            kp += 2;
            //printf("%02X", key[i]);
         }

      }
      else if(!(strcmp(argv[2], "-256"))){ //decryption with 256-bit key
         //key
         char *kp = argv[4];
         u8 key[32];
         for(i = 0; i < 32; i++) {
            sscanf(kp, "%2hhx", &key[i]);
            kp += 2;
            //printf("%02X", key[i]);
         }

      }
      else {
         printf("Wrong key length - please choose '-128', '-192' or '-256'\n");
      }
   }
   else {
      printf("Wrong mode selected - please choose '-e' or '-d'\n");
   }

}
