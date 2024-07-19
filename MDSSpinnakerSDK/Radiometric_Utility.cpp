#include "Radiometric_Utility.h"


Radiometric_Utility::Radiometric_Utility()
{
}

Radiometric_Utility::~Radiometric_Utility()
{
}

// =============================================================================
CTemperature Radiometric_Utility::imgToTemp(long lPixval, TPConstants* constants)
{
    if (constants == nullptr)
        return 0;

    double tmp;
    CTemperature K;

    tmp = imgToPow(lPixval, constants);
    //tmp = clipPow(tmp,pValState);
    tmp = powToObjSig(tmp, constants);
    K = objSigToTemp(tmp, constants);

    return (K);
}

// =============================================================================
double Radiometric_Utility::imgToPow(long lPixval, const TPConstants* constants)
{
    double pow = 0;

    pow = (lPixval - constants->J0) / constants->J1;

    return (pow);
}

// =============================================================================
USHORT Radiometric_Utility::tempToImg(double dKelvin, TPConstants* constants)
{
    USHORT pixVal;
    double tmp;

    tmp = tempToObjSig(dKelvin, constants);
    tmp = objSigToPow(tmp, constants);
    pixVal = powToImg(tmp, constants);

    return pixVal;
}

// =============================================================================
double Radiometric_Utility::objSigToPow(double dObjSig, const TPConstants* constants)
{
#define POW_OVERFLOW   100000.0  
    double p;

    if (constants->K1 > 0.0)
        p = (dObjSig + constants->K2) / constants->K1;
    else
        p = POW_OVERFLOW;

    return (p);
}

// =============================================================================
USHORT Radiometric_Utility::powToImg(double dPow, const TPConstants* constants)
{
    // Convert a power equivalent pixel value to mapped FPA pixel value  
    long rpix;

    rpix = (long)(constants->J1 * dPow + constants->J0);

    return (USHORT)rpix;
}

// =============================================================================
double Radiometric_Utility::tempToObjSig(double dblKelvin, const TPConstants* constants)
{
    double objSign = 0.0;
    double dbl_reg = dblKelvin;

    // objSign = R / (exp(B/T) - F)

    if (dbl_reg > 0.0) {

        dbl_reg = constants->B / dbl_reg;

        if (dbl_reg < EXP_SAFEGUARD) {
            dbl_reg = exp(dbl_reg);

            if (constants->F <= 1.0) {
                if (dbl_reg < ASY_SAFEGUARD)
                    dbl_reg = ASY_SAFEGUARD; // Don't get above a R/(1-F)
                                             // (horizontal) asymptote
            }
            else
            {
                // F > 1.0
                if (dbl_reg < constants->F * ASY_SAFEGUARD)
                    dbl_reg = constants->F * ASY_SAFEGUARD;
                // Don't get too close to a B/ln(F) (vertical) asymptote
            }

            objSign = constants->R / (dbl_reg - constants->F);
        }
    }

    return(objSign);
}

// =============================================================================
double Radiometric_Utility::powToObjSig(double dPow, const TPConstants* constants)
{
    return (constants->K1 * dPow - constants->K2);
}

// =============================================================================
CTemperature Radiometric_Utility::objSigToTemp(double dObjSig, const TPConstants* constants)
{
    double dbl_reg, tmp;
    CTemperature Tkelvin = 0.0;

    // Tkelvin = B /log(R / objSign + F)

    if (dObjSig > 0.0)
    {
        dbl_reg = constants->R / dObjSig + constants->F;

        if (constants->F <= 1.0) {
            if (dbl_reg < ASY_SAFEGUARD)
                dbl_reg = ASY_SAFEGUARD; // Don't get above a R/(1-F)
                                         // (horizontal) asymptote
        }
        else { // if (m_F > 1.0)

            tmp = constants->F * ASY_SAFEGUARD;
            if (dbl_reg < tmp)
                dbl_reg = tmp;
            // Don't get too close to a B/ln(F) (vertical) asymptote
        }
        Tkelvin = constants->B / quicklog(dbl_reg);
    }

    return (Tkelvin);
}

double Radiometric_Utility::doCalcAtmTao(const ObjParams* params, const stRParams* spectralResponse)
{
    double tao, dtao;
    double H, T, sqrtD, X, a1, b1, a2, b2;
    double sqrtH2O;
    double TT;
    double a1b1sqH2O, a2b2sqH2O, exp1, exp2;
    CTemperature C(CTemperature::Celsius);

#define H2O_K1 +1.5587e+0
#define H2O_K2 +6.9390e-2
#define H2O_K3 -2.7816e-4
#define H2O_K4 +6.8455e-7
#define TAO_TATM_MIN -30.0
#define TAO_TATM_MAX  90.0
#define TAO_SQRTH2OMAX 6.2365
#define TAO_COMP_MIN 0.400
#define TAO_COMP_MAX 1.000

    H = params->RelHum;
    C = params->AtmTemp;
    T = C.Value();        // We need Celsius to use constants defined above
    sqrtD = sqrt(params->ObjectDistance);
    X = spectralResponse->X;
    a1 = spectralResponse->alpha1;
    b1 = spectralResponse->beta1;
    a2 = spectralResponse->alpha2;
    b2 = spectralResponse->beta2;

    if (T < TAO_TATM_MIN)
        T = TAO_TATM_MIN;
    else if (T > TAO_TATM_MAX)
        T = TAO_TATM_MAX;

    TT = T * T;

    sqrtH2O = sqrt(H * exp(H2O_K1 + H2O_K2 * T + H2O_K3 * TT + H2O_K4 * TT * T));

    if (sqrtH2O > TAO_SQRTH2OMAX)
        sqrtH2O = TAO_SQRTH2OMAX;

    a1b1sqH2O = (a1 + b1 * sqrtH2O);
    a2b2sqH2O = (a2 + b2 * sqrtH2O);
    exp1 = exp(-sqrtD * a1b1sqH2O);
    exp2 = exp(-sqrtD * a2b2sqH2O);

    tao = X * exp1 + (1 - X) * exp2;
    dtao = -(a1b1sqH2O * X * exp1 + a2b2sqH2O * (1 - X) * exp2);
    // The real D-derivative is also divided by 2 and sqrtD.
    // Here we only want the sign of the slope! */

    if (tao < TAO_COMP_MIN)
        tao = TAO_COMP_MIN;      // below min value, clip

    else if (tao > TAO_COMP_MAX)
    {
        // check tao at 1 000 000 m dist
        tao = X * exp(-(1.0E3) * a1b1sqH2O) + (1.0 - X) * exp(-(1.0E3) * a2b2sqH2O);

        if (tao > 1.0)    // above max, staying up, assume \/-shape
            tao = TAO_COMP_MIN;
        else
            tao = TAO_COMP_MAX; // above max, going down, assume /\-shape
    }
    else if (dtao > 0.0 && params->ObjectDistance > 0.0)
        tao = TAO_COMP_MIN;   // beween max & min, going up, assume \/

    // else between max & min, going down => OK as it is, ;-)

    return(tao);
}

// =============================================================================
double Radiometric_Utility::doCalcK1(const ObjParams* params)
{
    double dblVal = 1.0;

    dblVal = params->AtmTao * params->Emissivity * params->ExtOptTransm;

    if (dblVal > 0.0)
        dblVal = 1 / dblVal;

    return (dblVal);
}

// =============================================================================
double Radiometric_Utility::doCalcK2(double dAmbObjSig, double dAtmObjSig, double dExtOptTempObjSig, const ObjParams* params)
{
    double emi;
    double temp1 = 0.0;
    double temp2 = 0.0;
    double temp3 = 0.0;

    emi = params->Emissivity;

    if (emi > 0.0)
    {
        temp1 = (1.0 - emi) / emi * dAmbObjSig;

        if (params->AtmTao > 0.0) {
            temp2 = (1.0 - params->AtmTao) / (emi * params->AtmTao) * dAtmObjSig;

            if (params->ExtOptTransm > 0.0 && params->ExtOptTransm < 1.0) {
                temp3 = (1.0 - params->ExtOptTransm) /
                    (emi * params->AtmTao * params->ExtOptTransm) * dExtOptTempObjSig;
            }
        }
    }

    return (temp1 + temp2 + temp3);
}

// =============================================================================
void Radiometric_Utility::doUpdateCalcConst(ObjParams* params, stRParams* spectralResponse, TPConstants* constants)
{
    params->AtmTao = doCalcAtmTao(params, spectralResponse);

    constants->K1 = doCalcK1(params);

    constants->K2 = doCalcK2(tempToObjSig(params->AmbTemp, constants),
        tempToObjSig(params->AtmTemp, constants),
        tempToObjSig(params->ExtOptTemp, constants), params);
}