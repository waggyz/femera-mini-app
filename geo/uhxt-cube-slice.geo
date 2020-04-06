/* Makes a structured mesh cube,
 * having N and (N-1) nodes along each each,
 * divided into n partitions.

gmsh -setnumber p 1 -setnumber h 2 -setnumber n 2 geo/unst-cube.geo -

*/
Geometry.CopyMeshingMethod = 1;

//p=1;h=10;//n=1;
//Sx=5; Sy=6; Sz=8;
//Sx=2; Sy=2; Sz=2;

If(p<=0)
  p=1;
EndIf
If(h<=0)
  h=10;
EndIf
//If(n<=0)
//  n=1;
//EndIf

If(Sx<=0)
  Sx=1;
EndIf
If(Sy<=0)
  Sy=1;
EndIf
If(Sz<=0)
  Sz=1;
EndIf

N=h+1;

Point(1)={0,0,0, 1/(N-1)};
Point(2)={1/Sx,0,0, 1/(N-1)};
Line(1)={1,2};

S=Extrude{0,1/Sy,0}{ Curve {1}; };
V=Extrude{0,0,1/Sz}{ Surface {S[1]}; };

Vt=1;
Physical Volume ( Vt )={ V[1] };

Z=0;
For(1:Sz)
Y=0;
For(1:Sy)
X=0;
For(1:Sx)
If((X+Y+Z)>0)
Vt+=1;

V=Translate{ X,Y,Z }{ Duplicata{ Volume{V[1]}; } };
Physical Volume(Vt)={ V };

EndIf
X+=1/Sx;
EndFor
Y+=1/Sy;
EndFor
Z+=1/Sz;
EndFor

/*
RenumberMeshNodes;
RenumberMeshElements;
*/

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
If(p>2)
Mesh.SecondOrderIncomplete    = 0;
EndIf
Mesh.SecondOrderLinear        = 1;

Mesh.CharacteristicLengthFromPoints=1;
Mesh.PartitionSplitMeshFiles  = 0 ;//1 ;
Mesh.PartitionCreatePhysicals = 0 ;//FIXME Is this useful?
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

Mesh 3;

/*
RenumberMeshNodes;
RenumberMeshElements;
*/
/*
If( (n>1) )
  PartitionMesh n;
EndIf
*/
