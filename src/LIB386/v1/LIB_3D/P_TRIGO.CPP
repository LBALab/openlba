/*
 * P_TRIGO.c - 3D Trigonometry and Matrix Library
 * Converted from P_TRIGO.ASM to C98
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LIB_3D.H"
#include "../LIB_SYS/ADELINE.H"

extern short P_SinTab[1024];

#define TYPE_3D  0
#define TYPE_ISO 1

#define LT00 0
#define LT01 1
#define LT02 2
#define LT10 3
#define LT11 4
#define LT12 5
#define LT20 6
#define LT21 7
#define LT22 8

/* Local variables */
static WORD compteur = 0;

/* Global variables */
WORD TypeProj = 0;

WORD XCentre = 320;
WORD YCentre = 200;

WORD Xp = 0;
WORD Yp = 0;

WORD IsoScale = 500;

WORD Z_Min = 0;
WORD Z_Max = 0;

LONG KFactor = 128;
LONG LFactorX = 1024;
LONG LFactorY = 840;

LONG CameraX = 0;
LONG CameraY = 0;
LONG CameraZ = 0;

LONG CameraXr = 0;
LONG CameraYr = 0;
LONG CameraZr = 0;

LONG AlphaLight = 0;
LONG BetaLight = 0;
LONG GammaLight = 0;

LONG Alpha = 0;
LONG Beta = 0;
LONG Gamma = 0;

LONG lAlpha = 0;
LONG lBeta = 0;
LONG lGamma = 0;

LONG NormalXLight = 0x61;
LONG NormalYLight = 0;
LONG NormalZLight = 0;

LONG X0 = 0;
LONG Y0 = 0;
LONG Z0 = 0;

LONG EX0 = 0;  /* Rot2D sur LONG */
LONG EY0 = 0;

/* Matrix structures */
LONG LMatriceRot[9] = {0};
LONG LMatW00 = 0, LMatW01 = 0, LMatW02 = 0;
LONG LMatW10 = 0, LMatW11 = 0, LMatW12 = 0;
LONG LMatW20 = 0, LMatW21 = 0, LMatW22 = 0;
LONG LMatriceWorld[9] = {0};

LONG LMatriceTempo[9] = {0};
LONG LMatriceDummy[9] = {0};

LONG TabMat[9 * 30]; /* 30 matrices max */

static LONG Save1 = 0;

void FlipMatrice(LONG *matsour, LONG *matdest)
{
    matdest[LT00] = matsour[LT00];
    matdest[LT01] = matsour[LT10];
    matdest[LT02] = matsour[LT20];
    matdest[LT10] = matsour[LT01];
    matdest[LT11] = matsour[LT11];
    matdest[LT12] = matsour[LT21];
    matdest[LT20] = matsour[LT02];
    matdest[LT21] = matsour[LT12];
    matdest[LT22] = matsour[LT22];
}


void RotList(WORD *source, WORD *dest, LONG *matrice, WORD nbpoints)
{
    WORD i;
    WORD *src = source;
    WORD *dst = dest;
    
    for (i = 0; i < nbpoints; i++) {
        LONG x, y, z;
        LONG result;
        
        /* Load coordinates - assuming packed as X,Y,Z WORDs */
        x = (LONG)((WORD)((src[0] & 0xFFFF) | ((src[1] & 0xFFFF) << 16))) & 0xFFFF;
        if (x & 0x8000) x |= 0xFFFF0000; /* sign extend */
        
        y = (LONG)((WORD)((src[0] >> 16) & 0xFFFF));
        if (y & 0x8000) y |= 0xFFFF0000; /* sign extend */
        
        z = (LONG)src[2];
        if (z & 0x8000) z |= 0xFFFF0000; /* sign extend */
        
        /* X transformation */
        result = (matrice[LT00] * x + matrice[LT01] * y + matrice[LT02] * z) >> 14;
        dst[0] = (WORD)(result + (X0 & 0xFFFF));
        
        /* Y transformation */
        result = (matrice[LT10] * x + matrice[LT11] * y + matrice[LT12] * z) >> 14;
        dst[1] = (WORD)(result + (Y0 & 0xFFFF));
        
        /* Z transformation */
        result = (matrice[LT20] * x + matrice[LT21] * y + matrice[LT22] * z) >> 14;
        dst[2] = (WORD)(result + (Z0 & 0xFFFF));
        
        src += 3;
        dst += 3;
    }
}


void TransRotList(WORD *source, WORD *dest, LONG *matrice, WORD nbpoints)
{
    WORD i;
    WORD *src = source;
    WORD *dst = dest;
    
    for (i = 0; i < nbpoints; i++) {
        LONG x, y, z;
        LONG result;
        
        /* Load and translate coordinates */
        x = (LONG)src[0] + lAlpha;
        y = (LONG)src[1] + lBeta;
        z = (LONG)src[2] + lGamma;
        
        /* X transformation */
        result = (matrice[LT00] * x + matrice[LT01] * y + matrice[LT02] * z) >> 14;
        dst[0] = (WORD)(result + (X0 & 0xFFFF));
        
        /* Y transformation */
        result = (matrice[LT10] * x + matrice[LT11] * y + matrice[LT12] * z) >> 14;
        dst[1] = (WORD)(result + (Y0 & 0xFFFF));
        
        /* Z transformation */
        result = (matrice[LT20] * x + matrice[LT21] * y + matrice[LT22] * z) >> 14;
        dst[2] = (WORD)(result + (Z0 & 0xFFFF));
        
        src += 3;
        dst += 3;
    }
}

void Rot(void)
{
    LONG x, y, z;

    x = X0;
    y = Y0;
    z = Z0;
    
    X0 = (LMatriceRot[LT00] * x + LMatriceRot[LT01] * y + LMatriceRot[LT02] * z) >> 14;
    Y0 = (LMatriceRot[LT10] * x + LMatriceRot[LT11] * y + LMatriceRot[LT12] * z) >> 14;
    Z0 = (LMatriceRot[LT20] * x + LMatriceRot[LT21] * y + LMatriceRot[LT22] * z) >> 14;
}

void RotMatIndex2(LONG *source, LONG *dest)
{
    LONG *ebp = source;
    LONG *eax = dest;
    LONG s, c, s2, c2;
    LONG temp[9];
    int use_temp = 0;
    
    if (lAlpha != 0) {
        LONG alpha = lAlpha & 1023;
        s = (LONG)P_SinTab[alpha];
        c = (LONG)P_SinTab[(alpha + 256) & 1023];
        
        eax[LT00] = ebp[LT00];
        eax[LT10] = ebp[LT10];
        eax[LT20] = ebp[LT20];
        
        eax[LT01] = (c * ebp[LT01] + s * ebp[LT02]) >> 14;
        eax[LT02] = (c * ebp[LT02] - s * ebp[LT01]) >> 14;
        eax[LT11] = (c * ebp[LT11] + s * ebp[LT12]) >> 14;
        eax[LT12] = (c * ebp[LT12] - s * ebp[LT11]) >> 14;
        eax[LT21] = (c * ebp[LT21] + s * ebp[LT22]) >> 14;
        eax[LT22] = (c * ebp[LT22] - s * ebp[LT21]) >> 14;
        
        ebp = eax;
    }
    
    if (lGamma != 0) {
        LONG gamma = lGamma & 1023;
        s2 = (LONG)P_SinTab[gamma];
        c2 = (LONG)P_SinTab[(gamma + 256) & 1023];
        
        LMatriceDummy[LT02] = ebp[LT02];
        LMatriceDummy[LT12] = ebp[LT12];
        LMatriceDummy[LT22] = ebp[LT22];
        
        LMatriceDummy[LT00] = (c2 * ebp[LT00] + s2 * ebp[LT01]) >> 14;
        LMatriceDummy[LT01] = (c2 * ebp[LT01] - s2 * ebp[LT00]) >> 14;
        LMatriceDummy[LT10] = (c2 * ebp[LT10] + s2 * ebp[LT11]) >> 14;
        LMatriceDummy[LT11] = (c2 * ebp[LT11] - s2 * ebp[LT10]) >> 14;
        LMatriceDummy[LT20] = (c2 * ebp[LT20] + s2 * ebp[LT21]) >> 14;
        LMatriceDummy[LT21] = (c2 * ebp[LT21] - s2 * ebp[LT20]) >> 14;
        
        ebp = LMatriceDummy;
    }
    
    if (lBeta != 0) {
        LONG beta = lBeta & 1023;
        s = (LONG)P_SinTab[beta];
        c = (LONG)P_SinTab[(beta + 256) & 1023];
        
        if (ebp == eax) {
            memcpy(temp, eax, sizeof(temp));
            ebp = temp;
        }
        
        eax[LT01] = ebp[LT01];
        eax[LT11] = ebp[LT11];
        eax[LT21] = ebp[LT21];
        eax[LT01] = (c * ebp[LT01] - s * ebp[LT02]) >> 14;
        eax[LT02] = (c * ebp[LT02] + s * ebp[LT01]) >> 14;
        eax[LT10] = (c * ebp[LT10] - s * ebp[LT12]) >> 14;
        eax[LT12] = (c * ebp[LT12] + s * ebp[LT10]) >> 14;
        eax[LT20] = (c * ebp[LT20] - s * ebp[LT22]) >> 14;
        eax[LT22] = (c * ebp[LT22] + s * ebp[LT20]) >> 14;
    } else if (ebp != eax) {
        memcpy(eax, ebp, 9 * sizeof(LONG));
    }
}

void RotMatW(void)
{
    RotMatIndex2(LMatriceWorld, LMatriceRot);
}

void SetLightVector(WORD alpha, WORD beta, WORD gamma)
{
    AlphaLight = (LONG)alpha;
    lAlpha = (LONG)alpha;
    BetaLight = (LONG)beta;
    lBeta = (LONG)beta;
    GammaLight = (LONG)gamma;
    lGamma = (LONG)gamma;
    
    RotMatW();
    Rot();
    
    NormalXLight = X0;
    NormalYLight = Y0;
    NormalZLight = Z0;
}

void RotatePoint(LONG X, LONG Y, LONG Z)
{
    X0 = X;
    Y0 = Y;
    Z0 = Z;
    Rot();
}

void LongWorldRot(void)
{
    LONG x = X0;
    LONG y = Y0;
    LONG z = Z0;
    long long temp;

    temp = ((long long)LMatriceWorld[LT00] * x + 
            (long long)LMatriceWorld[LT01] * y + 
            (long long)LMatriceWorld[LT02] * z) >> 14;
    X0 = (LONG)temp;

    temp = ((long long)LMatriceWorld[LT10] * x + 
            (long long)LMatriceWorld[LT11] * y + 
            (long long)LMatriceWorld[LT12] * z) >> 14;
    Y0 = (LONG)temp;

    temp = ((long long)LMatriceWorld[LT20] * x + 
            (long long)LMatriceWorld[LT21] * y + 
            (long long)LMatriceWorld[LT22] * z) >> 14;
    Z0 = (LONG)temp;
}

void LongWorldRotatePoint(LONG x, LONG y, LONG angle)
{
    Rot2D(x, y, angle);
}

void WorldRot(void)
{
    LONG x = X0;
    LONG y = Y0;
    LONG z = Z0;
    
    X0 = (LMatriceWorld[LT00] * x + LMatriceWorld[LT01] * y + LMatriceWorld[LT02] * z) >> 14;
    Y0 = (LMatriceWorld[LT10] * x + LMatriceWorld[LT11] * y + LMatriceWorld[LT12] * z) >> 14;
    Z0 = (LMatriceWorld[LT20] * x + LMatriceWorld[LT21] * y + LMatriceWorld[LT22] * z) >> 14;
}

void WorldRotatePoint(LONG x, LONG y, LONG angle)
{
    Rot2D(x, y, angle);
}

void LongInverseRot(void)
{
    LONG x = X0;
    LONG y = Y0;
    LONG z = Z0;
    long long tmp;
    
    tmp = ((x * LMatriceWorld[LT00]) + (y * LMatriceWorld[LT10]) + (z * LMatriceWorld[LT20])) >> 14;
	X0 = (LONG)tmp;

	tmp = ((x * LMatriceWorld[LT01]) + (y * LMatriceWorld[LT11]) + (z * LMatriceWorld[LT21])) >> 14;
	Y0 = (LONG)tmp;

	tmp = ((x * LMatriceWorld[LT02]) + (y * LMatriceWorld[LT12]) + (z * LMatriceWorld[LT22])) >> 14;
	Z0 = (LONG)tmp;
}


void LongInverseRotatePoint(LONG x, LONG y, LONG angle)
{
    Rot2D(x, y, -angle);
}

void RotateMatriceWorld(LONG palpha, LONG pbeta, LONG pgamma)
{
    lAlpha = palpha;
    lBeta = pbeta;
    lGamma = pgamma;
    
    RotMatIndex2(LMatriceWorld, LMatriceRot);
}

void SetPosCamera(LONG poswx, LONG poswy, LONG poswz)
{
    CameraX = poswx;
    CameraY = poswy;
    CameraZ = poswz;
}

void SetAngleCamera(LONG palpha, LONG pbeta, LONG pgamma)
{
    LONG s, c, s2, c2;
    LONG alpha_idx, beta_idx, gamma_idx;
    LONG h;
    
    Alpha = palpha & 1023;
    Beta = pbeta & 1023;
    Gamma = pgamma & 1023;
    
    alpha_idx = Alpha;
    s = (LONG)P_SinTab[alpha_idx];
    c = (LONG)P_SinTab[(alpha_idx + 256) & 1023];
    
    gamma_idx = Gamma;
    s2 = (LONG)P_SinTab[gamma_idx];
    c2 = (LONG)P_SinTab[(gamma_idx + 256) & 1023];
    
    LMatW00 = c2;
    LMatW01 = -s2;
    LMatW10 = (s2 * c) >> 14;
    LMatW11 = (c2 * c) >> 14;
    LMatW20 = (s2 * s) >> 14;
    LMatW21 = (c2 * s) >> 14;
    
    beta_idx = Beta;
    s2 = (LONG)P_SinTab[beta_idx];
    c2 = (LONG)P_SinTab[(beta_idx + 256) & 1023];
    
    h = LMatW00;
    LMatW00 = (c2 * h) >> 14;
    LMatW02 = (s2 * h) >> 14;
    
    h = LMatW10;
    LMatW10 = (c2 * h + s2 * s) >> 14;
    LMatW12 = (-c2 * s + s2 * h) >> 14;
    
    h = LMatW20;
    LMatW20 = (c2 * h - s2 * c) >> 14;
    LMatW22 = (c2 * c + s2 * h) >> 14;
    
    LMatriceWorld[LT00] = LMatW00;
    LMatriceWorld[LT01] = LMatW01;
    LMatriceWorld[LT02] = LMatW02;
    LMatriceWorld[LT10] = LMatW10;
    LMatriceWorld[LT11] = LMatW11;
    LMatriceWorld[LT12] = LMatW12;
    LMatriceWorld[LT20] = LMatW20;
    LMatriceWorld[LT21] = LMatW21;
    LMatriceWorld[LT22] = LMatW22;

    X0 = CameraX;
    Y0 = CameraY;
    Z0 = CameraZ;
    LongWorldRot();
    
    CameraXr = X0;
    CameraYr = Y0;
    CameraZr = Z0;
}

void SetInverseAngleCamera(LONG palpha, LONG pbeta, LONG pgamma)
{
    SetAngleCamera(palpha, pbeta, pgamma);
    FlipMatrice(LMatriceWorld, LMatriceDummy);
    CopyMatrice(LMatriceDummy, LMatriceWorld);
    
    X0 = CameraX;
    Y0 = CameraY;
    Z0 = CameraZ;
    LongWorldRot();
    
    CameraXr = X0;
    CameraYr = Y0;
    CameraZr = Z0;
}

void RotXY(WORD *x, WORD *z, WORD angle)
{
    if (angle == 0) return;
    
    LONG lx = (LONG)*x;
    LONG lz = (LONG)*z;
    LONG s, c;
    LONG result_x, result_z;
    
    WORD angle_idx = angle & 0x3FF;
    s = (LONG)P_SinTab[angle_idx];
    c = (LONG)P_SinTab[(angle_idx + 256) & 0x3FF];
    
    /* X' = X*cos(T) + Z*sin(T) */
    result_x = (lx * c + lz * s) >> 14;
    
    /* Z' = Z*cos(T) - X*sin(T) */
    result_z = (lz * c - lx * s) >> 14;
    
    *x = (WORD)result_x;
    *z = (WORD)result_z;
}

void Rot2D(LONG x, LONG y, LONG angle)
{
    WORD sx = (WORD)x;
    WORD sy = (WORD)y;
    WORD ang = (WORD)angle;
    
    RotXY(&sx, &sy, ang);
    EX0 = (LONG)sx;
    EY0 = (LONG)sy;
}

void SetFollowCamera(LONG targetx, LONG targety, LONG targetz,
                     LONG camalpha, LONG cambeta, LONG camgamma,
                     LONG camzoom)
{
    CameraX = targetx;
    CameraY = targety;
    CameraZ = targetz;
    
    SetAngleCamera(camalpha, cambeta, camgamma);
    
    CameraZr += camzoom;
    
    X0 = CameraXr;
    Y0 = CameraYr;
    Z0 = CameraZr;
    LongInverseRot();
    
    CameraX = X0;
    CameraY = Y0;
    CameraZ = Z0;
}

void Proj_3D(void)
{
    WORD z = (WORD)(Z0 + (KFactor & 0xFFFF));
    LONG x, y;
    
    if (z < 0) {
        z = 32767; /* max value on overflow */
    }
    
    x = ((LONG)(X0 & 0xFFFF) * (LFactorX & 0xFFFF)) / z;
    x += XCentre;
    
    y = (-(LONG)(Y0 & 0xFFFF) * (LFactorY & 0xFFFF)) / z;
    y += YCentre;
    
    Xp = (WORD)x;
    Yp = (WORD)y;
}

void Proj_ISO(void)
{
    LONG x = X0;
    LONG y = Y0;
    LONG z = Z0;
    LONG temp_x, temp_z;
    
    temp_x = x;
    temp_z = z;
    
    x = (temp_x - temp_z) * 24 / 512;  /* Equivalent to shift operations */
    x += XCentre;
    
    y = ((temp_x + temp_z) * 12 - y * 30) / 512;
    y = -y + YCentre;
    
    Xp = (WORD)x;
    Yp = (WORD)y;
}

LONG ProjettePoint(LONG CoorX, LONG CoorY, LONG CoorZ)
{
    LONG x, y, z;
    
    if (TypeProj == TYPE_ISO) {
        X0 = CoorX;
        Y0 = CoorY;
        Z0 = CoorZ;
        Proj_ISO();
        return -1;
    }
    
    x = CoorX - CameraXr;
    y = CoorY - CameraYr;
    z = CameraZr - CoorZ;
    
    if (z < 0) {
        Xp = 0;
        Yp = 0;
        return 0;
    }
    
    X0 = x;
    Y0 = y;
    Z0 = z;
    Proj_3D();
    
    return -1;
}


LONG LongProjettePoint(LONG CoorX, LONG CoorY, LONG CoorZ)
{
    LONG x, y, z;
    LONG result_x, result_y;
    
    x = CoorX - CameraXr;
    y = CoorY - CameraYr;
    z = CameraZr - CoorZ;
    
    if (z < 0) {
        Xp = 0;
        Yp = 0;
        return 0;
    }
    
    z += KFactor;
    if (z < 0) {
        Xp = 0;
        Yp = 0;
        return 0;
    }
    
    result_x = (x * LFactorX) / z + XCentre;
    result_y = (-y * LFactorY) / z + YCentre;
    
    /* Clamp to 16-bit range */
    if (result_x > 32767) result_x = 32767;
    if (result_x < -32768) result_x = -32768;
    if (result_y > 32767) result_y = 32767;
    if (result_y < -32768) result_y = -32768;
    
    Xp = (WORD)result_x;
    Yp = (WORD)result_y;
    
    return -1;
}

void SetProjection(LONG xc, LONG yc, LONG kfact, LONG lfactx, LONG lfacty)
{
    XCentre = (WORD)xc;
    YCentre = (WORD)yc;
    KFactor = kfact;
    LFactorX = lfactx;
    LFactorY = lfacty;
    TypeProj = TYPE_3D;
}

void SetIsoProjection(LONG xc, LONG yc, LONG scale)
{
    XCentre = (WORD)xc;
    YCentre = (WORD)yc;
    IsoScale = (short)scale;
    TypeProj = TYPE_ISO;
}


int TestVuePoly(void *ptrpoly)
{
    short *poly = (short *)ptrpoly;
    LONG v1, v2, cross;
    
    v1 = (LONG)(poly[4] - poly[1]) * (LONG)(poly[8] - poly[7]);
    v2 = (LONG)(poly[5] - poly[2]) * (LONG)(poly[1] - poly[7]);
    
    cross = v1 - v2;
    
    return (cross >= 0) ? 1 : 0;
}

void CopyMatrice(LONG *matsour, LONG *matdest)
{
    int i;
    for (i = 0; i < 9; i++) {
        matdest[i] = matsour[i];
    }
}

ULONG Sqr(ULONG x)
{
    return x * x;
}
