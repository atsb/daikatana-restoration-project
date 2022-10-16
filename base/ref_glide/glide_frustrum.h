#pragma once

class matrix;


void FrustrumSetup(float width_radians, float height_radians, point &angles_degrees, point &origin);

//returns the current world to view transformation matrix.
const matrix &WorldToView();







