/* Divides into n partitions.
*/
If( n<=0 )
  n=1;
EndIf

If( (n>1) & (n<1000) )
  PartitionMesh n;
EndIf
If( n>=1000 )
  If( n<4000 )
    sx = 10;
    sy = 10;
    sz = Round( n/10/10 /2)*2;
  EndIf
  If( n>=4000 )
    sx = 20;
    sy = 25;
    sz = Round( n/20/25 /2)*2;
  EndIf
  Plugin(SimplePartition).NumSlicesX = sx;
  Plugin(SimplePartition).NumSlicesY = sy;
  Plugin(SimplePartition).NumSlicesZ = sz;
  Plugin(SimplePartition).Run;
EndIf