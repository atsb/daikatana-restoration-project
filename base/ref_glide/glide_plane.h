#pragma once

class plane : public point {
public:
    //the distance of the plane from the origin.
    //a positive value means the origin in on the side of the plane that the 
    //normal is pointing.
    float d;

    //computes d given a point that lies on the plane.
    __inline void compute_d(const point &p);

    //returns the distance from the given point to the plane.
    __inline float dist(const point &p);
};

void plane::compute_d(const point &p) {
    d = -dot(p);
}

float plane::dist(const point &p) {
    return dot(p) + d;
}



