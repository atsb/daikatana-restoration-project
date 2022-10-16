#pragma once

class rgba;
class draw_point;

class rgba {
public:
    byte r, g, b;

    //alpha
    byte a;

    __inline void operator=(const rgba &other);
    __inline void eq(byte r, byte g, byte b);
};

class texture_point {
public:
    //the texture coordinates.
    unit u, v;

    //the rgba
    rgba color;

    __inline void operator=(const texture_point &other);
};


class draw_point : public point {
public:
    //the texture point.
    texture_point texture;

    __inline void operator=(const draw_point &other);
};



void rgba::operator=(const rgba &other) {
    r = other.r;
    g = other.g;
    b = other.b;
    a = other.a;
}

void rgba::eq(byte r, byte g, byte b) {
    this->r = r;
    this->g = g;
    this->b = b;
    a = 255;
}

void texture_point::operator=(const texture_point &other) {
    u = other.u;
    v = other.v;

    color = other.color;
}

void draw_point::operator=(const draw_point &other) {
    *(point *)this = other;

    texture = other.texture;
}

