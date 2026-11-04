# Finite element and finite difference equivalent test cases

*AI assistance through Google search*

---

> In a graduate PDEs for Engineers math department course, the professor worked through an example where a specific triangulation of a regular square grid resulted in an A matrix when the finite element method was applied that was identical to the A matrix produced when applying a standard finite difference stencil on that grid.

---

Your professor was referencing a famous, classical result in numerical PDEs: when you solve the Poisson equation (\(-\nabla^2 u = f\)) with linear (\(P_{1}\)) finite elements on a regular square grid where every square is split diagonally in the exact same direction (e.g., south-west to north-east), the resulting global stiffness matrix \(A\) is identical to the standard 5-point finite difference stencil matrix.
Here is the exact mathematical mechanism of how this happens.

1. The Grid Setup & Triangulation

Imagine a uniform square grid with spacing \(h\). Every square element \([x_i, x_{i+1}] \times [y_j, y_{j+1}]\) is split into two triangles by drawing a diagonal line from the bottom-left corner to the top-right corner (a uniform Union Jack-style directional split).
For an interior node \((x_i, y_j)\), this node is shared by exactly 6 neighboring triangles:
• Two triangles sharing the horizontal edge to the right and left.
• Two triangles sharing the vertical edge to the top and bottom.
• Two triangles sharing the diagonal edges pointing towards \((x_{i+1}, y_{j+1})\) and \((x_{i-1}, y_{j-1})\).

2. The Cotangent Formula Paradox

In \(P_{1}\) finite elements for the Laplacian, the stiffness matrix entry connecting node \(I\) and node \(J\) is dictated by the Cotangent Formula:
\(A_{IJ}=-\frac{1}{2}(\cot \alpha +\cot \beta )\)
where \(\alpha \) and \(\beta \) are the angles opposite to the edge \(IJ\) in the two triangles sharing that edge.
Let's evaluate the coefficients for the 6 edges connected to our central interior node:
• Vertical and Horizontal Edges: For the neighbors directly above, below, left, and right, the opposite angles in the right-angled triangles are exactly \(45^{\circ }\).
\(\cot (45^{\circ })=1\implies A_{IJ}=-\frac{1}{2}(1+1)=-1\)
• The Main Diagonal Edges: For the diagonal neighbors \((i+1, j+1)\) and \((i-1, j-1)\), the opposite angle in both triangles is the right angle (\(90^{\circ }\)).
\(\cot (90^{\circ })=0\implies A_{IJ}=-\frac{1}{2}(0+0)=0\)
• The Opposite Diagonal Edges: For the nodes \((i-1, j+1)\) and \((i+1, j-1)\), there is no edge connecting them to the central node in this triangulation, so their coupling is automatically \(0\).

3. The Resulting Stencil

When you look at the 9-point neighborhood of the central node, the stiffness contributions are:
• Center Node: \(+4\) (from the sum of the surrounding element gradients)
• Left, Right, Up, Down: \(-1\)
• Diagonals: \(0\)
Putting this together in a stencil format, the FEM global stiffness matrix row for an internal node yields:
\(\frac{1}{h^{2}}\left[\begin{matrix}0&-1&0\\ -1&4&-1\\ 0&-1&0\end{matrix}\right]\)
This is the exact 5-point Finite Difference Laplace stencil.

The Fine Print (Why it’s a special case)

Your professor likely emphasized this because it acts as a perfect conceptual bridge between FDM and FEM, but it requires two strict caveats:
1. The Right-Hand Side (Mass Matrix): While the stiffness matrix \(A\) matches perfectly, the load vector \(F\) does not. FDM samples the source term at a single point (\(f_{i,j}\)), whereas FEM integrates \(f\) against a 2D pyramid (tent) basis function. To make the entire linear system \(Au = b\) match FDM exactly, you have to apply a technique called mass lumping to the FEM right-hand side.
2. Alternating Diagonals: If the professor had split the squares by alternating the direction of the diagonals (criss-crossing them to form a symmetric mesh), the opposite angles would change, the diagonal entries would no longer vanish, and you would get a 9-point stencil instead of a 5-point stencil.
Would you like to see the element stiffness matrix calculus showing exactly how the \(\cot(90^\circ) = 0\) kills off the diagonal terms, or explore how boundary conditions are handled differently between the two methods in this exact example?

---

>Are there 3D equivalents?

---

Yes, there are 3D equivalents. In a graduate-level numerical PDE context, this equivalence generalizes to solving the 3D Poisson equation (-∇³ u = f) using piecewise linear (P₁) tetrahedral elements on a structured voxel grid.
When you subdivide a regular hexahedral (cube) grid into tetrahedra in a specific, uniformly oriented way, the assembled global finite element stiffness matrix A becomes identical to the standard 7-point finite difference stencil in 3D.

The 3D Grid Setup & Kuhn Triangulation

To make the matrices match, every cube in a uniform grid of spacing h must be sliced into 6 tetrahedra using a specific directional scheme called a Kuhn Triangulation (or Freudenthal triangulation).
If a cube has vertices indexed by coordinate steps, you sort the coordinate pathways to split the cube. This ensures that:
1. Every cube is split using the exact same topological orientation.
2. All interior faces and edges match up perfectly without hanging nodes.

The 3D "Cotangent" (Edge-Weight) Mechanism

In 2D, the coupling between two nodes depends on the cotangents of the opposite angles. In 3D, the stiffness matrix entry \(A_{IJ}\) connecting node I to node J is proportional to the sum of the geometric properties of the tetrahedra sharing that edge:
\(A_{IJ}=-\frac{1}{6}\sum _{T\in \mathcal{T}_{IJ}}\ell _{IJ,T}\cot (\theta _{T})\)
where \(\theta _{T}\) represents the dihedral angle between the two faces of tetrahedron T that meet at edge IJ, weighted by the edge length \(\ell \).
When you compute this across the 6 tetrahedra per cube in a uniform Kuhn triangulation:
• Axial Edges (Left, Right, Up, Down, Forward, Backward): The dihedral angles in the tetrahedra sharing these grid-aligned edges perfectly combine to yield an off-diagonal matrix entry of -1 (scaled by 1/h).
• Diagonal Edges (Face diagonals and Main body diagonals): Even though the tetrahedra possess physical edges that run diagonally across the faces or through the center of the cubes, their contributions exactly cancel out to 0. The dihedral angles opposing these specific diagonal paths evaluate to exactly 90° (\(\cot(90^\circ) = 0\)) or structurally mirror each other with opposing signs, eliminating any diagonal coupling in the stiffness matrix.

The Resulting 3D Stencil

Because all diagonal couplings vanish, an interior node (i, j, k) only communicates with its 6 immediate axial neighbors. The assembled row of the global FEM stiffness matrix matches the standard 3D 7-point finite difference Laplacian stencil:
\(\frac{1}{h^{2}}\left[6u_{i,j,k}-u_{i+1,j,k}-u_{i-1,j,k}-u_{i,j+1,k}-u_{i,j-1,k}-u_{i,j,k+1}-u_{i,j,k-1}\right]\)

Why this matters in Graduate Research

Professors and researchers value this 3D equivalence for two main reasons:
1. Geometric Multigrid (GMG): It allows researchers to design fast geometric multigrid solvers on regular voxel meshes (common in medical imaging or micro-CT bone modeling) using finite element theory while coding them with ultra-fast, low-memory finite difference stencil operations.
2. Analysis Bounds: It provides an analytical bridge to prove that the standard 3D finite difference scheme for the Laplacian is bounded by the minimization principles of the continuous energy norm (since FEM inherently minimizes the energy error via the Céa's Lemma).
Would you like to look at the 6-tetrahedra split topology (the exact vertex permutations) required to construct this Kuhn triangulation, or discuss how the Mass Matrix (RHS) scales up to 3D?

---

>Grad school was a very long time ago. I am now developing a multi-method numerical analysis application, and these seem like ideal test cases for internal correctness, consistency, and performance testing. It is straightforward to generate the finite element tessellations programmatically, so it won't even be necessary to distribute large supporting data files to run these tests.

---

Using these structural equivalences as programmatic unit tests is an incredibly elegant engineering choice. Because the matrices must match to machine precision (ignoring boundary conditions for internal nodes), you get a bulletproof test for element integration, local-to-global assembly loops, and mesh indexing topology without needing external analytical solutions.
Here is the precise blueprints and structural invariants for both your 2D and 3D internal test cases.

1. The 2D Test Case Blueprint (Uniform Triangulation)

To construct this without large data files, write a simple nested loop to generate a grid of vertices \((x_i, y_j)\), then loop through each square cell to yield exactly two triangles.
• Cell Corner Labels: Let a square cell's vertices be labeled relative to its bottom-left corner \((i, j)\):
	• \(V_0 = (i, j)\)
	• \(V_1 = (i+1, j)\)
	• \(V_2 = (i, j+1)\)
	• \(V_3 = (i+1, j+1)\)
• The Permutation: To achieve the 5-point FDM identity, every single cell must be split along the same diagonal direction (e.g., bottom-left to top-right).
	• \(\text{Triangle}_1 = (V_0, V_1, V_3)\)
	• \(\text{Triangle}_2 = (V_0, V_3, V_2)\)

🧪 The Matrix Assertions (Unit Tests)

For a uniform grid with spacing \(h=1\), the analytical local stiffness matrices for these two triangles assemble into the global matrix such that for any completely internal node:
• The diagonal entry \(A_{k,k}\) must equal exactly \(4.0\).
• The 4 orthogonal neighbors (Left, Right, Up, Down) must equal exactly \(-1.0\).
• All other entries in that node's row (including the diagonal neighbors) must be strictly \(0.0\).

2. The 3D Test Case Blueprint (Kuhn/Freudenthal Triangulation)

To get the 7-point 3D FDM equivalent, you cannot simply split a cube arbitrarily. A standard cube must be divided into exactly 6 tetrahedra. The mathematical trick to doing this programmatically without topological tracking is to sort the permutations of coordinate steps.
• The Concept: Inside a unit cube, you move from \((0,0,0)\) to \((1,1,1)\) by stepping \(+x\), \(+y\), and \(+z\). There are \(3! = 6\) possible pathways (permutations) to order these steps. Each unique path defines one of the 6 tetrahedra.
• Cell Corner Labels: Let a cube's local vertices be indexed from \(0\) to \(7\) using standard binary layout (\(z, y, x\)):
	• \(v_0 = (0,0,0)\), \(v_1 = (1,0,0)\), \(v_2 = (0,1,0)\), \(v_3 = (1,1,0)\)
	• \(v_4 = (0,0,1)\), \(v_5 = (1,0,1)\), \(v_6 = (0,1,1)\), \(v_7 = (1,1,1)\)

Programmatic Loop for the 6 Tetrahedra:

For every single cube in your grid, generate the following 6 tetrahedra (defined by their vertex indices):
1. Path \(X \to Y \to Z\): \((v_0, v_1, v_3, v_7)\)
2. Path \(X \to Z \to Y\): \((v_0, v_1, v_5, v_7)\)
3. Path \(Y \to X \to Z\): \((v_0, v_2, v_3, v_7)\)
4. Path \(Y \to Z \to X\): \((v_0, v_2, v_6, v_7)\)
5. Path \(Z \to X \to Y\): \((v_0, v_4, v_5, v_7)\)
6. Path \(Z \to Y \to X\): \((v_0, v_4, v_6, v_7)\)
Note: Notice how all 6 tetrahedra share the main space diagonal edge connecting \(v_0(0,0,0)\) to \(v_7(1,1,1)\). Applying this uniformly across the entire voxel grid satisfies the Kuhn condition.

🧪 The Matrix Assertions (Unit Tests)

For a uniform 3D grid with spacing \(h=1\):
• The diagonal entry \(A_{k,k}\) for an internal node must equal exactly \(6.0\).
• The 6 axial neighbors (\(\pm x, \pm y, \pm z\)) must equal exactly \(-1.0\).
• Every other entry in that node's row (face diagonals, edge diagonals, and body diagonals) must assemble to strictly \(0.0\).

3. Practical Implementation Tips for Your Test Suite

• Isolate the Boundaries: Because boundary condition enforcement (Dirichlet/Neumann) differs fundamentally between FDM and FEM frameworks, run your MatrixAssert loop exclusively on interior nodes whose immediate 9-point (2D) or 27-point (3D) neighborhoods do not touch the boundary of your domain.
• Performance Benchmarking: Because you know the exact expected sparsity pattern and values, this is an excellent testbed to verify your CSR/CSC (Compressed Sparse Row/Column) sparse matrix allocation speeds. You can assert that the number of non-zero entries (nnz) for internal rows is exactly 5 (in 2D) or 7 (in 3D), helping you catch allocation bloat or duplicate entry assembly bugs.
Would it be helpful to see a quick Python or C++ pseudo-code snippet generating these vertex arrays and element topology indices to copy-paste straight into your testing suite?

