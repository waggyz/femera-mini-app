// Dimensions
L = 0.100; // x
W = 0.050; // y
H = 0.030; // z

// Element size
el = 0.01;
//
L_n = 1+ L / el;
W_n = 1+ W / el;
H_n = 1+ H / el;

// Points
Point(1) = {0, 0, H};
Point(2) = {0, 0, 0};
Point(3) = {0, W, H};
Point(4) = {0, W, 0};
Point(5) = {L, 0, H};
Point(6) = {L, 0, 0};
Point(7) = {L, W, H};
Point(8) = {L, W, 0};

// Lines
Line(1) = {1, 2};
Line(2) = {1, 3};
Line(3) = {3, 4};
Line(4) = {2, 4};
Line(5) = {5, 6};
Line(6) = {5, 7};
Line(7) = {7, 8};
Line(8) = {6, 8};
Line(9) = {2, 6};
Line(10) = {1, 5};
Line(11) = {4, 8};
Line(12) = {3, 7};

// Surfaces
Line Loop(1) = {1, 4, -3, -2};    Plane Surface(1) = {1};  // Left
Line Loop(2) = {5, 8, -7, -6};    Plane Surface(2) = {2};  // Right
Line Loop(3) = {1, 9, -5, -10};   Plane Surface(3) = {3};  // Front
Line Loop(4) = {12, 7, -11, -3};  Plane Surface(4) = {4};  // Back
Line Loop(5) = {4, 11, -8, -9};   Plane Surface(5) = {5};  // Under
Line Loop(6) = {2, 12, -6, -10};  Plane Surface(6) = {6};  // Top

// Volume
Surface Loop(1) = {1, 2, 3, 4, 5, 6}; Volume(1) = {1};
//
Transfinite Curve { 9,10,11,12} = L_n;
Transfinite Curve { 2, 4, 6, 8} = W_n;
Transfinite Curve { 1, 3, 5, 7} = H_n;
//
Transfinite Surface { 1, 2, 3, 4, 5, 6};
//
