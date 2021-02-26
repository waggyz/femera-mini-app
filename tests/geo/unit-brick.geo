/* Makes a structured mesh cube,
 * having N and (N-1) nodes along each each,
 * divided into n partitions.

gmsh -setnumber p 1 -setnumber h 2 -setnumber n 2 geo/unit-cube.geo -

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

Nx=N; Ny=N; Nz=N;

Point(1)={0,0,0};
Point(2)={1,0,0};
Line(1)={1,2};
Transfinite Line{ 1 } = Nx;

S=Extrude{0,1,0}{ Curve {1}; };
Transfinite Line{ S[2],S[3] } = Ny;

V=Extrude{0,0,1}{ Surface {S[1]}; };

Transfinite Line{ 12,13,17,21 } = Nz;//TODO Check this.
//Transfinite Line{ 12:21     } = Nz;

Transfinite Surface { S[1],V[0],V[2],V[3],V[4],V[5] };

Transfinite Volume { V[1] };

Physical Surface( 111 )={S[1]};// bottom xy plane, normal:z
Physical Surface( 222 )={V[0]};// top xy plane, normal:z
Physical Surface( 333 )={V[2]};// back front xz plane, normal:y

Physical Surface( 444 )={V[3]};// right yz plane, normal:x
Physical Surface( 555 )={V[4]};// front xz plane, normal:y
Physical Surface( 666 )={V[5]};// left yz plane, normal:x

Recombine Surface{ : };

Physical Volume ( 123 )={V[1]};
//NOTE Bug reading: surfaces of volume inherit this tag

//RenumberMeshNodes;
//RenumberMeshElements;

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
If(p>2)
Mesh.SecondOrderIncomplete    = 0;
EndIf
Mesh.SecondOrderLinear        = 1;

Mesh.NbPartitions             = n ;
Mesh.PartitionSplitMeshFiles  = 0 ;
Mesh.PartitionCreatePhysicals = 0 ;//TODO use true to get # partitions?
Mesh.PartitionCreateTopology  = 1 ;// needed to include surfaces w/physics
Mesh.PartitionCreateGhostCells= 0 ;// 0:requires coordinate sync.
Mesh.SaveAll                  = 0 ;//
Mesh.Algorithm = 8;

//SetOrder p;
RecombineAll=1;
Mesh 3;
RecombineMesh;

//RenumberMeshNodes;
//RenumberMeshElements;

If( (n>1) )
  PartitionMesh n;
EndIf

//Save Sprintf("../cube/unit%gp%gn%g.msh2",h,p,n);

//These are probably not needed
//Coherence Mesh;
//RenumberMeshNodes;
//RenumberMeshElements;

