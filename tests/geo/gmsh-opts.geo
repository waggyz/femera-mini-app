// Set Gmsh options.

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
If(p>2)
Mesh.SecondOrderIncomplete    = 0;
EndIf
Mesh.SecondOrderLinear        = 1;

Mesh.CharacteristicLengthFromPoints=1;
Mesh.PartitionSplitMeshFiles  = 0 ;//1 ;
Mesh.PartitionCreatePhysicals = 0 ;// Is this useful?
Mesh.PartitionCreateTopology  = 1 ;
Mesh.PartitionCreateGhostCells= 0 ;// Is this necessary?
Mesh.SaveAll                  = 0 ;// Includes 2d surfaces

Mesh.Algorithm  =3;// Chosen for less regular mesh
// 1: MeshAdapt, 2: Automatic, 5: Delaunay, 6: Frontal, 7: BAMG, 8: DelQuad

Mesh.Algorithm3D=10;// WAS 1
// 1: Delaunay, 4: Frontal, 5: Frontal Delaunay, 6: Frontal Hex, 7: MMG3D,
// 9: R-tree, 10: HXT

Mesh.Optimize=0;
SetOrder p;
RecombineAll=0;// Only make tets
