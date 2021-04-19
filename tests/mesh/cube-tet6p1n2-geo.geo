/* Makes a structured mesh cube,
 * having N and (N-1) nodes along each each,
 * optionally divided into n partitions.

gmsh  geo/unit-cube.geo -part n -3

*/
Geometry.CopyMeshingMethod = 1;

p=1;
h=1;
n=2;

If(p<=0)
  p=1;
EndIf
If(h<=0)
  h=1;
EndIf
//If(n==0)
//  n=4;
//EndIf
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

Transfinite Surface { S[1],V[0],V[2],V[3],V[4],V[5] };

Transfinite Volume { V[1] };
If(0==0)
Physical Surface( 111 )={S[1]};// bottom xy plane, out normal:-z
Physical Surface( 222 )={V[0]};// top    xy plane, out normal: z
Physical Surface( 333 )={V[2]};// back   xz plane, out normal: y
Physical Surface( 555 )={V[4]};// front  xz plane, out normal:-y
Physical Surface( "fmr:Load:Dirichlet_x:0.001",444 )={V[3]};
// right  yz plane, out normal: x
Physical Surface( "fmr:Load:Fix_x",666 )={V[5]};
// left   yz plane, out normal:-x

Physical Volume ( "fmr:Material:Elastic",123 )={V[1]};
EndIf

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
If(p>2)
Mesh.SecondOrderIncomplete    = 0;
EndIf
Mesh.SecondOrderLinear        = 1;
RecombineAll                  = 0;

Mesh.NbPartitions             = n ;
Mesh.PartitionSplitMeshFiles  = 0 ;
Mesh.PartitionCreatePhysicals = 0 ;//TODO use true to get partitions?
Mesh.PartitionCreateTopology  = 1 ;// needed to include surfaces w/physics
Mesh.PartitionCreateGhostCells= 0 ;// 0:requires coordinate sync in Femera.

Mesh.SaveAll                  = 0 ;//

If(0==1)
Mesh 3;
If( (n>1) )
  PartitionMesh n;
EndIf
EndIf

//Save Sprintf("../cube/unit%gp%gn%g.msh2",h,p,n);

//These are probably not needed
//Coherence Mesh;
//RenumberMeshNodes;
//RenumberMeshElements;

