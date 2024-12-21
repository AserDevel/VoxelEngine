#ifndef MAT4X4_H
#define MAT4X4_H

#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"
#include "Quat.h"
#include <array>

struct Mat4x4 {
    float data[4][4] = { 0 };

    inline Mat4x4() {};

    inline Mat4x4(float i) {
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                if (r == c) {
                    this->data[r][c] = i;
                } else {
                    this->data[r][c] = 0;
                }      
            }
        }
    }

    inline float* operator[](std::size_t row) { return data[row]; }

    inline const float* operator[](std::size_t row) const { return data[row]; }

    inline Vec4 operator*(const Vec4& i) {
        Vec4 v;
        v.x = i.x * this->data[0][0] + i.y * this->data[1][0] + i.z * this->data[2][0] + i.w * this->data[3][0];
        v.y = i.x * this->data[0][1] + i.y * this->data[1][1] + i.z * this->data[2][1] + i.w * this->data[3][1];
        v.z = i.x * this->data[0][2] + i.y * this->data[1][2] + i.z * this->data[2][2] + i.w * this->data[3][2];
        v.w = i.x * this->data[0][3] + i.y * this->data[1][3] + i.z * this->data[2][3] + i.w * this->data[3][3];
        return v;
    }

    inline Vec3 operator*(const Vec3& i) {
        Vec3 v;
        v.x = i.x * this->data[0][0] + i.y * this->data[1][0] + i.z * this->data[2][0] + 1 * this->data[3][0];
        v.y = i.x * this->data[0][1] + i.y * this->data[1][1] + i.z * this->data[2][1] + 1 * this->data[3][1];
        v.z = i.x * this->data[0][2] + i.y * this->data[1][2] + i.z * this->data[2][2] + 1 * this->data[3][2];
        return v;
    }

    // Column major matrix multiplication (applies right to left)
    inline Mat4x4 operator*(const Mat4x4& m) {
        Mat4x4 matrix(1);
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                matrix[c][r] = 
                this->data[0][r] * m[c][0] + 
                this->data[1][r] * m[c][1] + 
                this->data[2][r] * m[c][2] +
                this->data[3][r] * m[c][3];
            }
        }
        return matrix;
    }

    inline void print() {
        printf("[%f, %f, %f, %f]\n", this->data[0][0], this->data[0][1], this->data[0][2], this->data[0][3]);
        printf("[%f, %f, %f, %f]\n", this->data[1][0], this->data[1][1], this->data[1][2], this->data[1][3]);
        printf("[%f, %f, %f, %f]\n", this->data[2][0], this->data[2][1], this->data[2][2], this->data[2][3]);
        printf("[%f, %f, %f, %f]\n", this->data[3][0], this->data[3][1], this->data[3][2], this->data[3][3]);
    }
};

inline Mat4x4 MatrixRotationX(float pitch) {
    Mat4x4 matrix(1);
    matrix[0][0] = 1.0f;
    matrix[1][1] = cosf(pitch);
    matrix[1][2] = sinf(pitch);
    matrix[2][1] = -sinf(pitch);
    matrix[2][2] = cosf(pitch);
    matrix[3][3] = 1.0f;
    return matrix;
}

inline Mat4x4 MatrixRotationY(float yaw) {
    Mat4x4 matrix(1);
    matrix[0][0] = cosf(yaw);
    matrix[0][2] = sinf(yaw);
    matrix[2][0] = -sinf(yaw);
    matrix[1][1] = 1.0f;
    matrix[2][2] = cosf(yaw);
    matrix[3][3] = 1.0f;
    return matrix;
}

inline Mat4x4 MatrixRotationZ(float roll) {
    Mat4x4 matrix(1);
    matrix[0][0] = cosf(roll);
    matrix[0][1] = sinf(roll);
    matrix[1][0] = -sinf(roll);
    matrix[1][1] = cosf(roll);
    matrix[2][2] = 1.0f;
    matrix[3][3] = 1.0f;
    return matrix;
}

inline Mat4x4 MatrixRotation(float pitch, float yaw, float roll) {
    return MatrixRotationX(pitch) * MatrixRotationY(yaw) * MatrixRotationZ(roll);
}

inline Mat4x4 quatToMatrix(Quat q) {
    Mat4x4 matrix;
    matrix[0][0] = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
    matrix[0][1] = 2.0f * (q.x * q.y - q.z * q.w);
    matrix[0][2] = 2.0f * (q.x * q.z + q.y * q.w);
    matrix[1][0] = 2.0f * (q.x * q.y + q.z * q.w);
    matrix[1][1] = 1.0f - 2.0f * (q.x * q.x + q.z * q.z);
    matrix[1][2] = 2.0f * (q.y * q.z - q.x * q.w);
    matrix[2][0] = 2.0f * (q.x * q.z - q.y * q.w);
    matrix[2][1] = 2.0f * (q.y * q.z + q.x * q.w);
    matrix[2][2] = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
    matrix[3][3] = 1.0f;
    return matrix;
}

inline Mat4x4 MatrixProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar) {
    float fFovRad = 1.0f / tanf(fFovDegrees * 0.5f / 180.0f * 3.14159f);
    Mat4x4 matrix(1);
    matrix[0][0] = fAspectRatio * fFovRad;
    matrix[1][1] = fFovRad;
    matrix[2][2] = fFar / (fFar - fNear);
    matrix[3][2] = (-fFar * fNear) / (fFar - fNear);
    matrix[2][3] = 1.0f;
    matrix[3][3] = 0.0f;
    return matrix;
}

inline Mat4x4 MatrixTranslation(const Vec3& pos) {
    Mat4x4 matrix(1);
    matrix[0][0] = 1.0f;
    matrix[1][1] = 1.0f;
    matrix[2][2] = 1.0f;
    matrix[3][3] = 1.0f;
    matrix[3][0] = pos.x;
    matrix[3][1] = pos.y;
    matrix[3][2] = pos.z;
    return matrix;
}

inline Mat4x4 MatrixScaling(const Vec3& scale) {
    Mat4x4 matrix(1);
    matrix[0][0] = scale.x; 
    matrix[1][1] = scale.y; 
    matrix[2][2] = scale.z; 
    return matrix;
}

inline Mat4x4 MatrixWorld(const Vec3& pos, const Quat& rot, const Vec3& scale) {
    return MatrixTranslation(pos) * quatToMatrix(rot) * MatrixScaling(scale);
} 

inline Mat4x4 MatrixLookAt(Vec3& eye, Vec3& target, Vec3& up) {
    Vec3 zAxis = eye - target;
    zAxis = normalise(zAxis);
    Vec3 xAxis = cross(up, zAxis);
    xAxis = normalise(xAxis);
    Vec3 yAxis = cross(zAxis, xAxis);

    Mat4x4 view(1); // Identity matrix
    view[0][0] = xAxis.x; view[1][0] = xAxis.y; view[2][0] = xAxis.z;
    view[0][1] = yAxis.x; view[1][1] = yAxis.y; view[2][1] = yAxis.z;
    view[0][2] = zAxis.x; view[1][2] = zAxis.y; view[2][2] = zAxis.z;
    view[3][0] = -dot(xAxis, eye);
    view[3][1] = -dot(yAxis, eye);
    view[3][2] = -dot(zAxis, eye);
    return view;
}

inline Mat4x4 MatrixOrtho(float left, float right, float bottom, float top, float fNear, float fFar) {
    Mat4x4 matrix(1);
    matrix[0][0] = 2.0f / (right - left);
    matrix[1][1] = 2.0f / (top - bottom);
    matrix[2][2] = -2.0f / (fFar - fNear); // Negative for depth inversion
    matrix[0][3] = -(right + left) / (right - left);
    matrix[1][3] = -(top + bottom) / (top - bottom);
    matrix[2][3] = -(fFar + fNear) / (fFar - fNear);
    return matrix;
}


#endif