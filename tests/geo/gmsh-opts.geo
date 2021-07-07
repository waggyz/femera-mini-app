// Set Gmsh options.

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
If(p>2)
Mesh.SecondOrderIncomplete    = 0;
EndIf
Mesh.SecondOrderLinear        = 1;
SetOrder p;

Mesh.CharacteristicLengthFromPoints=1;
Mesh.PartitionSplitMeshFiles  = 0 ;
Mesh.PartitionCreatePhysicals = 0 ;// Is this useful?
Mesh.PartitionCreateTopology  = 1 ;
Mesh.PartitionCreateGhostCells= 0 ;// Is this necessary?
Mesh.SaveAll                  = 0 ;// Includes 2d surfaces

Mesh.Algorithm  = 6;// 6 seems fatest XS (WAS 1,2; 5: for less regular mesh)
// 1: MeshAdapt, 2: Automatic, 5: Delaunay, 6: Frontal-Delaunay,
// 7: BAMG, 8: DelQuad, 9: Packing of Parallelograms

Mesh.Algorithm3D = 10;// WAS 10,1
// 1: Delaunay, 4: Frontal [slow], 10: HXT [fast LG, elems need fixing]
// 3: Initial mesh only, Frontal Hex, 7: MMG3D, 9: R-tree

Mesh.Optimize=0;
RecombineAll=0;// Only make tets
