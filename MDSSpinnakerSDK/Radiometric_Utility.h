#pragma once

#include "temperature.hpp"
#include "MDSSpinnakerSDK.h"

static double quicklog(double dbl_val)
{
#define sqr2div2 0.7071067811865476
#define ln2 0.6931471805599453
#define C1 2.88539129   
#define C3 0.961470632
#define C5 0.598978649

#ifdef _WIN32
    /* Little endian machine assumed */
#define DBL_2EXP_ADDR(dbl) ((unsigned short int*)&dbl+3)
#define TRUE_DBL_2EXP(dbl) (((*DBL_2EXP_ADDR(dbl)&0x7ff0)>>4)-0x3ff)
#define SET_DBL_2EXP(dbl) *DBL_2EXP_ADDR(dbl)&=~0x7ff0,\
                          *DBL_2EXP_ADDR(dbl)|= 0x3fe0
#else
    /* Big endian machine assumed */
#define DBL_2EXP_ADDR(dbl) ((unsigned long int*)&dbl)
#define TRUE_DBL_2EXP(dbl) (((*DBL_2EXP_ADDR(dbl)&0x7ff00000)>>20)-0x3ff)
#define SET_DBL_2EXP(dbl) *DBL_2EXP_ADDR(dbl)&=~0x7ff00000,\
                          *DBL_2EXP_ADDR(dbl)|= 0x3fe00000
#endif


    double Z, Z2;
    int times_2;

    times_2 = TRUE_DBL_2EXP(dbl_val) + 1;
    SET_DBL_2EXP(dbl_val); /* to -1 */

    Z = (dbl_val - sqr2div2) / (dbl_val + sqr2div2);
    Z2 = Z * Z;
    return (times_2 + (((C5 * Z2 + C3) * Z2 + C1) * Z - 0.5)) * ln2;

#undef sqr2div2
#undef ln2
#undef C1
#undef C3
#undef C5
#undef DBL_2EXP_ADDR
#undef TRUE_DBL_2EXP
#undef SET_DBL_2EXP
}

class Radiometric_Utility
{

public:
    Radiometric_Utility();
    virtual ~Radiometric_Utility();

    static CTemperature imgToTemp(long lPixval, TPConstants* constants);
    static double imgToPow(long lPixval, const TPConstants* constants);
    static USHORT tempToImg(double dKelvin, TPConstants* constants);
    static double objSigToPow(double dObjSig, const TPConstants* constants);
    static USHORT powToImg(double dPow, const TPConstants* constants);
    static double tempToObjSig(double dblKelvin, const TPConstants* constants);
    static double powToObjSig(double dPow, const TPConstants* constants);
    static CTemperature objSigToTemp(double dObjSig, const TPConstants* constants);
    static double doCalcAtmTao(const ObjParams* params, const stRParams* spectralResponse);
    static double doCalcK1(const ObjParams* params);
    static double doCalcK2(double dAmbObjSig, double dAtmObjSig, double dExtOptTempObjSig, const ObjParams* params);
    static void doUpdateCalcConst(ObjParams* params, stRParams* spectralResponse, TPConstants* constants);
};

