#!/usr/bin/env python3
from enum import Enum, auto
import os, csv

# Enums =======================================================================
class Component(Enum):# fmr:Sims, fmr:Geom, fmr:Grid, fmr:Mtrl,...
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    SIMS = auto()
    GEOM = auto()
    GRID = auto()
    PART = auto()
    MTRL = auto()
    BCS = auto()
    SOLVE = auto()
    RUNS = auto()
    POST = auto()
    PLUGIN = auto()
    USER = auto()
    END = auto()

class Application(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    HANDLER = auto()
    RESEARCH = auto()
    ENGINEERING = auto()
    PARAMETER_SWEEP = auto()
    SENSITIVITY = auto()
    UQ = auto()
    ML = auto()
    END = auto()

class Simulation_size(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    AUTO = auto()
    XS = auto()
    S = auto()
    M = auto()
    L = auto()
    XL = auto()
    END = auto()

class Sims_file_format(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    AUTO = auto()
    JSON = auto()
    CSV = auto()
    END = auto()

class Geometry_shape(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    RECTANGLE = auto()
    BLOCK = auto()
    CYLINDER = auto()
    SPHERE = auto()
    SPHERE_SHELL = auto()
    END = auto()

class Cell_type(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    BLOCK_2PRISM = auto()
    BLOCK_5TET = auto()
    BLOCK_6TET = auto()
    BLOCK_6PYRAMID = auto()
    END = auto()

class Elem_type(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    TRI3 = auto()
    TRI6 = auto()
    TRI10 = auto()
    TET4 = auto()
    TET10 = auto()
    TET20 = auto()
    END = auto()

class Node_at(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    ORIGIN = auto()
    NODE_X_MIN = auto()
    NODE_Y_MIN = auto()
    NODE_Z_MIN = auto()
    NODE_X_MAX = auto()
    NODE_Y_MAX = auto()
    NODE_Z_MAX = auto()
    END = auto()

class Part_method(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    SINGLE_PART = auto()
    BY_GEOMETRY = auto()
    BY_GRID = auto()
    RECTILINEAR = auto()
    # EXTERNAL = auto() #TODO ?
    # METIS = auto() #TODO ?
    # PARMETIS = auto()
    END = auto()

class Mtrl_physics(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    ELASTIC = auto()
    PLASTIC_KINEMATIC = auto()
    THERMAL = auto()
    THERMOELASTIC_1WAY = auto() # 1- and 2-way coupling
    THERMOELASTIC_2WAY = auto()
    END = auto()

class Mtrl_symmetry(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    GENERAL = auto()
    ORTHOTROPIC = auto()
    CUBIC = auto()
    ISOTROPIC = auto()
    END = auto()

class Condition_method(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    JACOBI = auto()
    BLOCK_JACOBI = auto()
    END = auto()

class Solve_method(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    PCR = auto()
    PCG = auto()
    NCG = auto()
    END = auto()

class Analysis(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    STATIC = auto()
    DYNAMIC = auto()
    MODAL = auto()
    END = auto()

class Reduce(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    MIN = auto()
    MAX = auto()
    MAX_ABS = auto() # maximum of absolute values (suprenum norm, Linf norm)
    MIN_ABS = auto()
    PRODUCT = auto()
    SUM = auto()
    MEAN = auto()
    STDEV = auto()
    VARIANCE = auto()
    SKEWNESS = auto()
    KURTOSIS = auto()
    COUNT_NONZERO = auto() # count of non-zero elements (L0 norm)
    L1_NORM = auto()
    L2_NORM = auto()
    SUM_SQUARES = auto()
    SUM_RECIPROCALS = auto()# parallel Ohm's law
    HARMONIC_MEAN=auto() # The reciprocal of the arithmetic mean of reciprocals.
    GEOMETRIC_MEAN =auto() # The nth root of the product of n numbers.
    ENTROPY = auto()
    ALL = auto() # all are true
    ANY = auto() # any are true
    END = auto()

class File_format(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    TEXT = auto()
    CSV = auto()
    END = auto()
 
class Schedule(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    PREALLOCATE = auto()
    FIFO = auto()
    LIFO = auto()
    LARGE_TO_SMALL = auto()
    END = auto()

class Precision(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    SINGLE = auto()
    DOUBLE = auto()
    END = auto()

class CPU_SIMD(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    # REFERENCE = auto() # Reference implementation #TODO?
    SCALAR = auto()
    SSE = auto()
    AVX = auto()
    AVX2 = auto()
    AVX512 = auto()
    END = auto()

class Grid_structure(Enum):
    NONE = 0
    ERROR = auto()
    UNKNOWN = auto()
    UNSTRUCTURED = auto()
    # 1D-3D grids
    CARTESIAN = auto() # isotropic (scalar) scaling (1D-3D)
    RECTILINEAR = auto() # scaled x,y,z (1D-3D)
    GENERAL_LINEAR = auto() # 1x1-3x3 transformation matrix (1D-3D)
    CURVILINEAR = auto() # orthogonal, non-orthogonal (1D-3D)
    BODY_FITTED = auto() # (1D-3D)
    # 2D grids
    CONFORMAL_MAP = auto() # (2D?)
    POLAR = auto() # (2D)
    ELLIPTICAL = auto() # (2D)
    # 3D grids
    CYLINDRICAL = auto() # rotate around x, y, or z axis (3D)
    SPHERICAL = auto() # (3D)
    SPHEROIDAL = auto()  # oblate, prolate (3D)
    #
    END = auto()
'''
C++
enum class fmr::grid::Grid_structure {
    None,
    Error,
    Unknown,
    Unstructured,
    Cartesian,// isotropic scaling (scalar)
    Rectilinear,// scaled x,y,z
    General_linear,// 3x3 matrix
    Cylindrical,// x,y,z axis
    Spherical,
    Elliptical,
    Spheroidal,  // Oblate, Prolate
    Conformal_map,
    Curvilinear, // orthogonal, non-orthogonal
    Body_fitted,
    end
};
'''
#==============================================================================
def create_enum_from_csv(file_path, enum_name, start_at=None,
                         add_std_members=False):
    if add_std_members: #NOTE overrides start_at
        start_at = 3
    # Read the CSV file.
    with open(file_path, 'r') as csv_file:
        reader = csv.reader(csv_file)
        # Assuming the first row contains headers. Skip it.
        # next(reader, None) #NOTE Do not assume header row.
        #
        # Create a dictionary to hold enum members.
        enum_members = {}
        if(start_at == None):
            # Iterate through rows and add them to the enum_members dictionary.
            for row in reader:
                if len(row) >= 2:  # Ensure we have at least a name and a value.
                    name, value = row[0], row[1]
                    enum_members[name.upper()] = value
        else:
            if add_std_members:
                enum_members['NONE'] = 0
                enum_members['ERROR'] = 1
                enum_members['UNKNOWN'] = 2
            row_i = start_at
            for row in reader:
                if len(row) >= 1:  # Ensure we have at least a name.
                    name = row[0]
                    enum_members[name.upper()] = row_i
                    row_i += 1
            if add_std_members:
                enum_members['END'] = row_i
    # Create and return the Enum class dynamically.
    return Enum(enum_name, enum_members)

Data = create_enum_from_csv(
    os.path.join(os.getcwd(),'data','src','data-type.csv'),
        'Data', add_std_members=True)

if False:
    # The following define which data types apply to each class.
    class Sims(Enum):
        NONE = 0
        ERROR = auto()
        UNKNOWN = auto()
        NAME = Data.NAME
        VERSION = Data.VERSION
        APPLICATION = Data.APPLICATION
        END = auto()
    class Geom(Enum):
        NONE = 0
        ERROR = auto()
        UNKNOWN = auto()
        NAME = Data.NAME
        VERSION = Data.VERSION
        FILE_NAME = Data.FILE_NAME
        GEOM_D = Data.GEOM_D
        GEOMETRY_SHAPE = Data.GEOMETRY_SHAPE
        BOUNDING_BOX = Data.BOUNDING_BOX
        PRECISION = Data.PRECISION
        END = auto()

# Dictionary of enums ---------------------------------------------------------
fmr_enum_list ={
    'fmr:Component': Component,
    'fmr:Simulation_size': Simulation_size,
    'fmr:Geom_shape': Geometry_shape,
    'fmr:Grid_structure': Grid_structure,
    'fmr:Node_at': Node_at,
    'fmr:Cell_type': Cell_type,
    'fmr:Elem_type': Elem_type,
    'fmr:Mtrl_physics': Mtrl_physics,
    'fmr:Mtrl_symmetry': Mtrl_symmetry,
    'fmr:Condition_method': Condition_method,
    'fmr:Solve_method': Solve_method,
    'fmr:Reduce': Reduce,
    'fmr:Part_method': Part_method,
    'fmr:Application': Application,
    'fmr:File_format': File_format,
    'fmr:Analysis': Analysis,
    'fmr:Schedule': Schedule,
    'fmr:Precision': Precision,
    'fmr:CPU_SIMD': CPU_SIMD,
    'fmr:Data_type': Data
}