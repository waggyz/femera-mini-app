/*
 * Usage:
 * gmsh -nt 40 -clmin 1 -clmax 5 Grain_*.stl stl2gmsh.geo -format msh2 -save -o rve.msh
 * 
 */
CreateTopology;
//Coherence;// doesn't help

grain_n = newv-1; grain_i = 0;
For( 1 : grain_n )
  grain_i = grain_i+1;
  Surface Loop   ( grain_i ) = { grain_i };
  Volume         ( grain_i ) = { grain_i };
  Physical Volume( grain_i ) =   grain_i  ;
EndFor

p=2;

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
If(p>2)
  Mesh.SecondOrderIncomplete  = 0;
EndIf
Mesh.SecondOrderLinear        = 1;
SetOrder p;

Mesh.CharacteristicLengthFromPoints         = 0;
Mesh.CharacteristicLengthFromCurvature      = 0;
Mesh.CharacteristicLengthExtendFromBoundary = 0;

//Mesh.Algorithm  = 2;
// 1: MeshAdapt, 2: Automatic, 5: Delaunay, 6: Frontal, 7: BAMG, 8: DelQuad

Mesh.Algorithm3D = 1;
// 1: Delaunay, 4: Frontal, 5: Frontal Delaunay, 6: Frontal Hex, 7: MMG3D,
// 9: R-tree, 10: HXT

RecombineAll   = 0;// Only make tets
Mesh.Smoothing = 0;
Mesh.Optimize  = 0;

Mesh 3;
Coherence Mesh;
