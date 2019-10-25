/* Divides into n partitions. */
/*
If( n<=0 )
  n=1;
EndIf
If( (n>1) & (n<1000) )
  PartitionMesh n;
EndIf
Mesh.PartitionSplitMeshFiles  = 0 ;//1 ;
Mesh.PartitionCreatePhysicals = 0 ;//FIXME Is this useful?
Mesh.PartitionCreateTopology  = 1 ;
Mesh.PartitionCreateGhostCells= 0 ;// Is this necessary?
*/
//If( n>=1000 )
  /*
  * C,N,X,Y,Z are integers; N>>C, both given
  * for C=40: (5*X) * (8*Y) * Z approx N
  *
  * try: X=Y=Z=S: (8*S) * (10*S) * (4..20)*S approx N
  * try: X=Y=Z=S: (5*S) * (8*S) * (3..16)*S approx N
  */
  Plugin(SimplePartition).NumSlicesX = sx;
  Plugin(SimplePartition).NumSlicesY = sy;
  Plugin(SimplePartition).NumSlicesZ = sz;
  Plugin(SimplePartition).Run;
//EndIf