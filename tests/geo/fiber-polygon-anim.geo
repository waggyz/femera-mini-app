/* Makes a series of RVEs
gmsh -setnumber frame 1 tests/geo/fiber-shape-movie.geo
*/
// These will NOT be applied if already set by gmsh::onelab::setNumber (...).
frame = DefineNumber[0.0, Name "frame"];
wall  = DefineNumber[0.0, Name "wall"];
//=============================================================================
box_x  =   1.0;// RVE width
box_y  =   1.0;// RVE length
box_z  =   1.0;// RVE height (longitudinal direction)

View.AutoPosition    = 0;
General.Orthographic = 0;
General.DisplayBorderFactor = 0.0;
//
General.RotationCenterGravity = 0;// 1: approximate center of mass, 0: use below
General.RotationCenterX = box_x / 2;
General.RotationCenterY = box_y / 2;
General.RotationCenterZ = box_z / 2;
General.RotationX = -60; General.RotationY = 0; General.RotationZ = -150;

Mesh.SurfaceFaces  = 1;
Mesh.ColorCarousel = 2;

//Mesh.Nodes = 1;//FIXME No worky.
//General.GraphicsFontSize = 15;//FIXME No worky.
//General.GraphicsFontSizeTitle = 30;
//General.FontSize = 12;

Mesh.ElementOrder      = 2;
Mesh.SecondOrderLinear = 1;
//-----------------------------------------------------------------------------
If (frame >= 1)

poly_n = Floor ((frame-1) / 30)    + 3;
poly_r = 0.4 * ((frame-1) % 30)/30 + 0.05;

SetFactory("OpenCASCADE");

i=1; For (1:poly_n)
  Point (i) = {
    box_x/2 + poly_r*Cos (2*Pi *i/poly_n),
    box_y/2 + poly_r*Sin (2*Pi *i/poly_n), 0};
i++; EndFor

i=1; For (1:poly_n)
  Line (i) = {(((i-1)%poly_n)+1),(((i)%poly_n)+1)};
i++; EndFor

Curve Loop (1) = {1:poly_n};
Surface (1) = {1};

Box (1) = {0,0,0, box_x,box_y,box_z};

fiber[] = Extrude {0,0,box_z} {Surface{1};};

//twist = 2*Pi / poly_n;//TODO Twisting extrude not available with OpenCascade.
//fiber[] = Extrude {{0,0,box_z},{0,0,1},{box_x/2,box_y/2,0},twist} {Surface{1};};

BooleanFragments {Volume{:}; Delete;} {Volume{1}; Delete;}

Physical Surface (1) = {(poly_n+9):(poly_n+15)};// Matrix surface
Physical Surface (2) = {1,(poly_n+3):(poly_n+8)};// Fiber surface
//-----------------------------------------------------------------------------
//Mesh 3;
//SetOrder 2;
//-----------------------------------------------------------------------------
b=0.33;// Hack to make camera distance consistent by enlarging the bounding box.
Point (newp) = {-box_x*b    ,-box_y*b    ,-box_z*b    };
Point (newp) = { box_x*(1+b), box_y*(1+b), box_z*(1+b)};

Show "*";
Hide {Volume {:}; Surface {poly_n+11,poly_n+12,poly_n+14}; Point{:};}

If (1==0)
Hide "*";
Show {Surface {1,(poly_n+5):(poly_n+10),poly_n+13,poly_n+15};}
Endif

If (1==0)
  sims = (frame-1)/30;
  sims_mn = Floor (sims / 60);
  sims_sc = Floor (sims-sims_mn*60);
  wall_mn = Floor (wall / 60);
  wall_sc = Floor (wall-wall_mn*60);
  View "comments" {
    T2 (10,15,0) {Sprintf(" Sim time %2g:%02g", sims_mn, sims_sc)};
    T2 (10,30,0) {Sprintf("Wall time %2g:%02g", wall_mn, wall_sc)};
    T2 (10,45,0) {"TODO Fix times"};
  };
Endif
EndIf
//=============================================================================
