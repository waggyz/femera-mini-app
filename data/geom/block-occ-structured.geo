//
L = 0.100;// x
W = 0.050;// y
H = 0.030;// z
el = 0.01;
//
L_n = 1+ L / el;
W_n = 1+ W / el;
H_n = 1+ H / el;
//
SetFactory("OpenCASCADE");
//
Box (1) = {0,0,0, L,W,H};
//
Transfinite Curve { 9,10,11,12} = L_n;
Transfinite Curve { 2, 4, 6, 8} = W_n;
Transfinite Curve { 1, 3, 5, 7} = H_n;
//
Transfinite Surface { 1, 2, 3, 4, 5, 6};
//
