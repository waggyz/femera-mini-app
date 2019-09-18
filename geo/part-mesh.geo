/*
*/



If(p==0)
  p=1;
EndIf
If(h==0)
  h=10;
EndIf
If(n==0)
  n=1;
EndIf

Mesh.CharacteristicLengthFromPoints=1;
Mesh.PartitionSplitMeshFiles  = 1 ;
Mesh.PartitionCreatePhysicals = 0 ;//FIXME Is this useful?
Mesh.PartitionCreateTopology  = 1 ;
Mesh.PartitionCreateGhostCells= 1 ;
Mesh.SaveAll                  = 0 ;// Includes 2d surfaces

If(n>1)
  PartitionMesh n;
EndIf
Save Sprintf("../cube/unst%gp%gn%g.msh2",h,p,n);
