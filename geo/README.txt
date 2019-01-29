unit-cube.geo unst-cube.geo

Command-line use:

gmsh -setnumber p 1 -setnumber h 2 -setnumber m 2 geo/unit-cube.geo -

p >= 1: Element order
h >= 2: Number of elements along each axis [node_n = (M+1)^3]
m >= 1: Total number of partitions

Output: ../cube/cube<h>p<p>m<m>_<i>.msh 

A total of m files, one for each partition i, will be written to the ../cube/ directory.

