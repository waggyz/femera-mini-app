# Femera CPU L3 cache blocking

Femera performs best when the finite element model geometric domain is partitioned into blocks that each fit into the L3 cache available to each CPU core. Here is an example of a compute node approximating a typical Intel Skylake CPU compute node.

| Quantity | Units | Description |
| -------- | ----- | ----------- |
|      128 | GB    | Total node DRAM |
|      100 | GB    | Node DRAM available for model data |
|        2 | socket/node |  |
|       20 | core/socket | Physical CPUs |
|     38.5 | MB/socket   | L3 cache |
|       40 | core/node  | Physical CPUs |
|       77 | MB/node     | L3 cache |

For this example, this is nominally how Femera might partition double precision linear elastic quadratic tetrahedra models for L3 cache blocking.

| Quantity | Units | Description |
| -------- | ----- | ----------- |
|    100 | B/dof | Total model RAM size |
|     77 | B/dof | Active data RAM during solve |
| 19,250 | dof/core | Max total model size/core (fits L3 cache) |
| 25,000 | dof/core | L3 cache solve (partition) data size/core |
|      1 | Gdof/node | Max total model size/node |
|      1 | Mdof/node | L3 cache solve data size/node |
|  40,000 | parts/1Gdof | Partitioning for max model size on one node |

Each finite element model type can be categorized by size for each type of compute node.

|     |    Model size     | Concurrent models | Performance scaling | Notes |
| --- | ----------------- | --- | --- | ----------- |
|  XS | < 20 kdof         | 40 | linear | Entire model fits in per-core L3 cache |
|   S | 20 kdof to 1 Mdof | 1 to 40 | superlinear | Model solve data fits in total L3 cache |
|   M | 1 Mdof to 10 Mdof |   1 | superlinear | Model runs faster than large model performance |
|   L | 10 Mdof to 1 Gdof |   1 | linear | Each model fits on one node |
|  XL | > 1 Gdof          |   1 |  | Each model runs across more than one node |

These are nominal approximations. Femera provides performance profiling self-tests to determine optimal model concurrency and partition sizes.

