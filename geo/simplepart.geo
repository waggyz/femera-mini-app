/* Divides into n partitions.
*/
If(n<=0)
  n=1;
EndIf

If( (n>1) & (n<500) )
  PartitionMesh n;
EndIf
If( (n>=500) & (n<4000) )
  Plugin(SimplePartition).NumSlicesX = 10;
  Plugin(SimplePartition).NumSlicesY = 10;
  Plugin(SimplePartition).NumSlicesZ = Round( n/10/10 );
  Plugin(SimplePartition).Run;
EndIf
If( n>=4000 )
  Plugin(SimplePartition).NumSlicesX = 20;
  Plugin(SimplePartition).NumSlicesY = 25;
  Plugin(SimplePartition).NumSlicesZ = Round( n/20/25 );
  Plugin(SimplePartition).Run;
EndIf
