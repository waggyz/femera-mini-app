//
L = 0.100;// x
W = 0.050;// y
H = 0.030;// z
//
el = 0.01;
L_n = 1+ L / el;
W_n = 1+ W / el;
H_n = 1+ H / el;
//
Point(1) = {0, 0, 0};
Point(2) = {L, 0, 0};
//
Line(1) = {1, 2};
//
Extrude {0, W, 0} { Curve{1}; }
Extrude {0, 0, H} { Surface{5}; }
//
Transfinite Curve { 1, 2, 7, 9} = L_n;
Transfinite Curve { 3, 4, 8,10} = W_n;
Transfinite Curve {12,13,17,21} = H_n;
//
Transfinite Surface {5,14,18,22,26,27};
//
