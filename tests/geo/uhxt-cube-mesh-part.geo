/* Makes a structured mesh cube,
 * having N and (N-1) nodes along each each,
 * divided into n partitions.

gmsh -setnumber p 1 -setnumber h 2 -setnumber n 2 geo/unst-cube.geo -

*/
Geometry.CopyMeshingMethod = 1;

//porder=1;

If(p<=0)
  p=1;
EndIf
If(h<=0)
  h=1;
EndIf
//If(n<=0)
n=-1;
//EndIf
N=h+1;

Point(1)={0,0,0, 1/(N-1)};
Point(2)={1,0,0, 1/(N-1)};
Line(1)={1,2};

S=Extrude{0,1,0}{ Curve {1}; };
V=Extrude{0,0,1}{ Surface {S[1]}; };

Physical Surface( 111 )={S[1]};// bottom xy plane, out normal:-z
Physical Surface( 222 )={V[0]};// top    xy plane, out normal: z
Physical Surface( 333 )={V[2]};// back   xz plane, out normal: y
Physical Surface( 555 )={V[4]};// front  xz plane, out normal:-y
Physical Surface( "fmr:Load:Dirichlet_x::0.001",444 )={V[3]};
// right  yz plane, out normal: x
Physical Surface( "fmr:Load:Fix_x",666 )={V[5]};
// left   yz plane, out normal:-x

Physical Volume ( "fmr:Material:Elastic",123 )={V[1]};
//NOTE msh4 bug reading: surfaces of volume inherit this tag

//RenumberMeshNodes;
//RenumberMeshElements;

Mesh.ElementOrder             = p;
Mesh.SecondOrderIncomplete    = 1;
If(p>2)
Mesh.SecondOrderIncomplete    = 0;
EndIf
Mesh.SecondOrderLinear        = 1;

Mesh.CharacteristicLengthFromPoints=1;
Mesh.NbPartitions             = n ;
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

Mesh 3;
//RenumberMeshNodes;
//RenumberMeshElements;
If(part_size>0)
  n = Ceil (Mesh.NbNodes/part_size);//TODO round to ncpu
EndIf
If(n>1)
  PartitionMesh n;
EndIf
/*
If ( n>=1000 )
  If( n<4000 )
    sx = 10;
    sy = 10;
    sz = Floor( n/10/10 /2)*2;
  EndIf
  If( n>=4000 )
    sx = 20;
    sy = 25;
    sz = Floor( n/20/25 /2)*2;
  EndIf
  Plugin(SimplePartition).NumSlicesX = sx;
  Plugin(SimplePartition).NumSlicesY = sy;
  Plugin(SimplePartition).NumSlicesZ = sz;
  Plugin(SimplePartition).Run;
EndIf
*/
