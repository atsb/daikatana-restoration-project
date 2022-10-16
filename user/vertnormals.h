#ifndef __VERTNORMALS_H__
#define __VERTNORMALS_H__


/*
====================
  Ash -- a general purpose class to support an array of vectors.
  They're kinda uniformly distributed too...
====================
*/

#define NUMVERTEXNORMALS    255

class CVertexNormals
{
protected:
    CVector mN[NUMVERTEXNORMALS];

public:
    CVertexNormals()
    {
        int vectNum = 0;
    
        float theta, dtheta, phi, dphi;
    
        int thetasamples = 23,
            phisamples = 11,    // this needs to be odd so we get an equator
            i, j;
    
        float cosphi, costheta, sintheta;
    
        // north pole
        mN[vectNum].x = 0.0f;
        mN[vectNum].y = 0.0f;
        mN[vectNum].z = 1.0f;
        vectNum++;
    
        // south pole
        mN[vectNum].x = 0.0f;
        mN[vectNum].y = 0.0f;
        mN[vectNum].z = -1.0f;
        vectNum++;
    
        // now we'll use 11 latitudinal and 23 longitudinal samples
        // total samples = 11x23+2 = 255, so we waste a slot -- oh well
        dtheta = 360.0f/thetasamples;   // there are thetasamples theta intervals
        dphi = 180.0f/(phisamples+1);   // there are phisamples+2-1 phi intervals (we don't need to do the poles, 
                                        // but we still want even spacing)
        for (i=0;i<thetasamples;i++)
        {
            theta = (float)i*dtheta; // degrees
            costheta = cos(DEG2RAD(theta));
            sintheta = sin(DEG2RAD(theta));
            for (j=-phisamples/2;j<=phisamples/2;j++)
            {
                phi = (float)j*dphi; // degrees
                cosphi = cos(DEG2RAD(phi));
    
                mN[vectNum].x = cosphi*costheta;
                mN[vectNum].y = cosphi*sintheta;
                mN[vectNum].z = sin(DEG2RAD(phi));
                vectNum++;
            }
        }
    }

    ~CVertexNormals() {};
    
    int GetClosestVertNormal(const CVector& v) const
    {
        int i, bestnormal;
        float dotprod, maxdotprod;
    
        // remember we wasted a slot...
        bestnormal = -1;
        maxdotprod = -1.0f;   // min dot prod value between unit vectors at the origin
        for(i=0;i<NUMVERTEXNORMALS;i++)
        {
            dotprod = v.DotProduct(mN[i]);
    
            if (dotprod>maxdotprod)
            {
                maxdotprod = dotprod;
                bestnormal = i;
            }
        }
    
        return bestnormal;
    }


    inline CVector& GetNormal(const int i) { return mN[i]; }
    float* GetFloatArray()  { return (float *)mN; } // ugh, for friggin common.cpp

};



#endif
