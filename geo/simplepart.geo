/* Divides into n partitions.
*/
If(n<=0)
  n=1;
EndIf

If( (n>1) & (n<2000) )
  PartitionMesh n;
EndIf
If( n>=2000 )
  Plugin(SimplePartition).NumSlicesX = 20;
  Plugin(SimplePartition).NumSlicesY = 25;
  Plugin(SimplePartition).NumSlicesZ = Round( n/20/25 );
  Plugin(SimplePartition).Run;
EndIf
