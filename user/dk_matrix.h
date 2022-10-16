
#ifndef _DK_MATRIX_H
#define _DK_MATRIX_H

//#pragma once

class CVector;

typedef float matrix_row[4];

class CMatrix {
protected:
    //the rows in the CMatrix.
    matrix_row rows[3];

public:
    __inline matrix_row &operator[](int index);
    __inline const matrix_row &operator[](int index) const;

    //multiplies two matrices together.
    void mul(const CMatrix &left, const CMatrix &right);

    void MakeTransformation(const CVector &forward, const CVector &right, const CVector &up, const CVector &origin);
    void MakeTransformation(const CVector &angle_radians, const CVector &origin);

    void MakeTranslation(const CVector &disp);

    void MakeRotation(const CVector &forward, const CVector &right, const CVector &up);
    void MakeRotation(const CVector &angle_radians);

    void MakeScale(const CVector &scale);
};

matrix_row &CMatrix::operator[](int index) {
    return rows[index];
}

const matrix_row &CMatrix::operator[](int index) const {
    return rows[index];
}

#endif // _DK_MATRIX_H