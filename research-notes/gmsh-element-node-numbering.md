# 10.2 Node ordering ¶

>https://gmsh.info/dev/doc/texinfo/gmsh.html#Node-ordering


Historically, Gmsh first supported linear elements (lines, triangles, quadrangles, tetrahedra, prisms and hexahedra). Then, support for second and some third order elements has been added. Below we distinguish such “low order elements”, which are hardcoded (i.e. they are explicitly defined in the code), and general “high-order elements”, that have been coded in a more general fashion, theoretically valid for any order.

## 10.2.1 Low order elements

For all mesh and post-processing file formats, the reference elements are defined as follows.

```text
Line:                  Line3:           Line4:

      v
      ^
      |
      |
0-----+-----1 --> u    0----2----1      0---2---3---1


```text
Triangle:               Triangle6:          Triangle9/10:
v
^
|
2                       2                    2
|`\                     |`\                  | \
|  `\                   |  `\                7   6
|    `\                 5    `4              |     \
|      `\               |      `\            8  (9)  5
|        `\             |        `\          |         \
0----------1--> u       0-----3----1         0---3---4---1
```

```text

Triangle12/15:
 v
 ^
 |
 2
 | \
 9   8
 |     \
10 (14)  7
 |         \
11 (12) (13) 6
 |             \
 0---3---4---5---1--> u
```

```text
Quadrangle:            Quadrangle8:            Quadrangle9:

      v
      ^
      |
3-----------2          3-----6-----2           3-----6-----2
|     |     |          |           |           |           |
|     |     |          |           |           |           |
|     +---- | --> u    7           5           7     8     5
|           |          |           |           |           |
|           |          |           |           |           |
0-----------1          0-----4-----1           0-----4-----1
```

```text
Tetrahedron:                          Tetrahedron10:

                   v
                 .
               ,/
              /
           2                                     2
         ,/|`\                                 ,/|`\
       ,/  |  `\                             ,/  |  `\
     ,/    '.   `\                         ,6    '.   `5
   ,/       |     `\                     ,/       8     `\
 ,/         |       `\                 ,/         |       `\
0-----------'.--------1 --> u         0--------4--'.--------1
 `\.         |      ,/                 `\.         |      ,/
    `\.      |    ,/                      `\.      |    ,9
       `\.   '. ,/                           `7.   '. ,/
          `\. |/                                `\. |/
             `3                                    `3
                `\.
                   ` w
```

```text
Hexahedron:             Hexahedron20:          Hexahedron27:

       v
3----------2            3----13----2           3----13----2
|\     ^   |\           |\         |\          |\         |\
| \    |   | \          | 15       | 14        |15    24  | 14
|  \   |   |  \         9  \       11 \        9  \ 20    11 \
|   7------+---6        |   7----19+---6       |   7----19+---6
|   |  +-- |-- | -> u   |   |      |   |       |22 |  26  | 23|
0---+---\--1   |        0---+-8----1   |       0---+-8----1   |
 \  |    \  \  |         \  17      \  18       \ 17    25 \  18
  \ |     \  \ |         10 |        12|        10 |  21    12|
   \|      w  \|           \|         \|          \|         \|
    4----------5            4----16----5           4----16----5
```

```text
Prism:                      Prism15:               Prism18:

           w
           ^
           |
           3                       3                      3
         ,/|`\                   ,/|`\                  ,/|`\
       ,/  |  `\               12  |  13              12  |  13
     ,/    |    `\           ,/    |    `\          ,/    |    `\
    4------+------5         4------14-----5        4------14-----5
    |      |      |         |      8      |        |      8      |
    |    ,/|`\    |         |      |      |        |    ,/|`\    |
    |  ,/  |  `\  |         |      |      |        |  15  |  16  |
    |,/    |    `\|         |      |      |        |,/    |    `\|
   ,|      |      |\        10     |      11       10-----17-----11
 ,/ |      0      | `\      |      0      |        |      0      |
u   |    ,/ `\    |    v    |    ,/ `\    |        |    ,/ `\    |
    |  ,/     `\  |         |  ,6     `7  |        |  ,6     `7  |
    |,/         `\|         |,/         `\|        |,/         `\|
    1-------------2         1------9------2        1------9------2
```

```text
Pyramid:                     Pyramid13:

               4                            4
             ,/|\                         ,/|\
           ,/ .'|\                      ,/ .'|\
         ,/   | | \                   ,/   | | \
       ,/    .' | `.                ,/    .' | `.
     ,/      |  '.  \             ,7      |  12  \
   ,/       .' w |   \          ,/       .'   |   \
 ,/         |  ^ |    \       ,/         9    |    11
0----------.'--|-3    `.     0--------6-.'----3    `.
 `\        |   |  `\    \      `\        |      `\    \
   `\     .'   +----`\ - \ -> v  `5     .'        10   \
     `\   |    `\     `\  \        `\   |           `\  \
       `\.'      `\     `\`          `\.'             `\`
          1----------------2            1--------8-------2
                    `\
                       u
```


```text
Pyramid14:
               4
             ,/|\
           ,/ .'|\
         ,/   | | \
       ,/    .' | `.
     ,7      |  12  \
   ,/       .'   |   \
 ,/         9    |    11
0--------6-.'----3    `.
  `\        |      `\    \
    `5     .' 13     10   \
      `\   |           `\  \
        `\.'             `\`
           1--------8-------2
                    `\
                       u
```

## 10.2.2 High-order elements

The nodes of an element of order p are ordered with respect to its corners, numbered as for the low order elements above:

the corners;
the nodes inside the edges, edge after edge in the order of the table below: the p-1 nodes of an edge are equally spaced, from the first corner of the edge to the second one;
for a volume element, the nodes inside the faces, face after face in the order of the table below: the nodes of a face are ordered as the nodes inside a triangle or a quadrangle of order p whose corners are the corners of the face, in the order of the table;
the nodes inside the element, ordered as all the nodes of an element of the same type and of lower order placed inside it: of order p-3 for a triangle, p-2 for a quadrangle, p-4 for a tetrahedron, p-2 for a hexahedron and p-3 for a pyramid. Prisms are the exception, see below.

The rule thus applies recursively to the nodes inside the faces and inside the element. Incomplete (serendipity) elements only have the nodes of the corners and of the edges. The low order elements above follow the same rule.

Element	Edges (first corner, second corner)

- Triangle	(0,1) (1,2) (2,0)
- Quadrangle	(0,1) (1,2) (2,3) (3,0)
- Tetrahedron	(0,1) (1,2) (2,0) (3,0) (3,2) (3,1)
- Hexahedron	(0,1) (0,3) (0,4) (1,2) (1,5) (2,3) (2,6) (3,7) (4,5) (4,7) (5,6) (6,7)
- Prism	(0,1) (0,2) (0,3) (1,2) (1,4) (2,5) (3,4) (3,5) (4,5)
- Pyramid	(0,1) (0,3) (0,4) (1,2) (1,4) (2,3) (2,4) (3,4)

Element	Faces (corners)

- Tetrahedron	(0,2,1) (0,1,3) (0,3,2) (3,1,2)
- Hexahedron	(0,3,2,1) (0,1,5,4) (0,4,7,3) (1,2,6,5) (2,3,7,6) (4,5,6,7)
- Prism	(0,2,1) (3,4,5) (0,1,4,3) (0,3,5,2) (1,2,5,4)
- Pyramid	(0,1,4) (3,0,4) (1,2,4) (2,3,4) (0,3,2,1)

For example, the face (3,1,2) of a tetrahedron of order 4 has 3 nodes inside, ordered as the 3 nodes inside a triangle of order 4 whose corners 0, 1 and 2 are the corners 3, 1 and 2 of the tetrahedron. Note that the edges do not always go from the corner of lower number to the corner of higher number, nor do the faces always start at the corner of lowest number.
