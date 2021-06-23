/* Makes a series of RVEs

gmsh geo/fiber-shape-movie.geo -

*/

// These will NOT be applied if already set by gmsh::onelab::setNumber (...).
frame = DefineNumber[45.0, Name "frame"];
wall = DefineNumber[3.35, Name "wall"];

//=============================================================================
box_x  =   1 ;// RVE width
box_y  =   1 ;// RVE length
box_z  =   1 ;// RVE depth (longitudinal direction)

poly_n = 3 + Floor((frame-1)/30);
poly_r = 0.05 + 0.40 * ((frame-1) % 30)/30;

SetFactory("OpenCASCADE");

i=1; For (1:poly_n)
Point (i) = {poly_r*Cos (2*Pi *i/poly_n), poly_r*Sin (2*Pi *i/poly_n),0};
i++; EndFor

i=1; For (1:poly_n)
Line (i) = {(((i-1)%poly_n)+1),(((i)%poly_n)+1)};
i++; EndFor

Curve Loop (1) = {1:poly_n};
Surface (1) = {1};

Box (1) = {-box_x/2,-box_y/2,0, box_x,box_y,box_z};

fiber[] = Extrude {0,0,box_z}{ Surface{1}; };

BooleanFragments { Volume{:}; Delete;}{ Volume{1}; Delete;}

Physical Surface (1) = {(poly_n+9):(poly_n+15)};// Matrix surface
Physical Surface (2) = {1,(poly_n+3):(poly_n+8)};// Fiber surface

//-----------------------------------------------------------------------------
General.Trackball = 0;
General.RotationX = -60; General.RotationY = 0; General.RotationZ = 30;

//Mesh.Nodes = 1;//FIXME No worky.
Mesh.SurfaceFaces  = 1;
Mesh.ColorCarousel = 2;
General.GraphicsFontSize = 15;//FIXME No worky.
//General.GraphicsFontSizeTitle = 30;
//General.FontSize = 12;

Mesh.SecondOrderLinear = 1;

Mesh 3;
SetOrder 2;

Show "*";
Hide { Volume {:}; Surface{(poly_n+9):(poly_n+11)}; Point{:}; }

sims_mn = Floor(frame/30 / 60);
sims_sc = (frame/30) % 60;

wall_mn = Floor(wall / 60);
wall_sc = wall % 60;

View "comments" {
  T2(10,15,0){Sprintf("Sims %2.0f:%02.0f",sims_mn,sims_sc)};
  T2(10,30,0){Sprintf("Wall %2.0f:%02.0f",wall_mn,wall_sc)};
};
