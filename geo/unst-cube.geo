/* Makes a structured mesh cube,
 * having N and (N-1) nodes along each each,
 * divided into P partitions.

gmsh -setnumber p 1 -setnumber h 2 -setnumber n 2 geo/unst-cube.geo -

*/
Geometry.CopyMeshingMethod = 1;

//porder=1;

If(p==0)
  p=1;
EndIf
If(h==0)
  h=10;
EndIf
If(n==0)
  n=4;
EndIf
N=h+1;

Point(1)={0,0,0, 1/(N-1)};
Point(2)={1,0,0, 1/(N-1)};
Line(1)={1,2};

S=Extrude{0,1,0}{ Curve {1}; };

V=Extrude{0,0,1}{ Surface {S[1]}; };


Physical Surface( 111 )={S[1]};// bottom xy plane, normal:z
Physical Surface( 222 )={V[0]};// top xy plane, normal:z
Physical Surface( 333 )={V[2]};// front xz plane, normal:y

Physical Surface( 444 )={V[3]};// right yz plane, normal:x
Physical Surface( 555 )={V[4]};// back xz plane, normal:y
Physical Surface( 666 )={V[5]};// left yz plane, normal:x

Physical Volume ( 123 )={V[1]};//FIXME msh4 bug reading: surfaces of volume inherit this tag

RenumberMeshNodes;
RenumberMeshElements;

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
Mesh.SecondOrderLinear        = 1;

Mesh.CharacteristicLengthFromPoints=1;
Mesh.PartitionSplitMeshFiles  = 1 ;
Mesh.PartitionCreatePhysicals = 0 ;//FIXME Is this useful?
Mesh.PartitionCreateTopology  = 1 ;
Mesh.PartitionCreateGhostCells= 1 ;
Mesh.SaveAll                  = 0 ;// Includes 2d surfaces

Mesh.Algorithm  =3;// Chosen for less regular mesh
// 1: MeshAdapt, 2: Automatic, 5: Delaunay, 6: Frontal, 7: BAMG, 8: DelQuad

Mesh.Algorithm3D=1;
//1: Delaunay, 4: Frontal, 5: Frontal Delaunay, 6: Frontal Hex, 7: MMG3D, 9: R-tree, 10: HXT

Mesh.Optimize=0;

//SetOrder p;
RecombineAll=0;// Only make tets
Mesh 3;
RenumberMeshNodes;
RenumberMeshElements;

PartitionMesh n;

Save Sprintf("../cube/unst%gp%gn%g.msh2",h,p,n);

//FIXME These are probably not needed
//Coherence Mesh;
//RenumberMeshNodes;
//RenumberMeshElements;

