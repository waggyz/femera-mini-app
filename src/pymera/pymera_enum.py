#!/usr/bin/env python3
from enum import Enum, auto
import os, csv

# Enums =======================================================================
class Module(Enum):# fmr:Sims, fmr:Geom, fmr:Grid, fmr:Mtrl,...
    NONE = auto()
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
    END = auto()

class Cell_type(Enum):
    NONE = auto()
    ERROR = auto()
    UNKNOWN = auto()
    BLOCK_2PRISM = auto()
    BLOCK_5TET = auto()
    BLOCK_6TET = auto()
    BLOCK_6PYRAMID = auto()
    END = auto()

class Part_method(Enum):
    NONE = auto()
    ERROR = auto()
    UNKNOWN = auto()
    BLOCK_2PRISM = auto()
    SINGLE = auto()
    RECTILINEAR = auto()
    BY_VOLUME = auto()

class Mtrl_physics(Enum):
    NONE = auto()
    ERROR = auto()
    UNKNOWN = auto()
    ELASTIC_ISOTROPIC = auto()
    ELASTIC_ORTHOTROPIC = auto()
    PLASTIC_KINEMATIC = auto()
    THERMAL_ISOTROPIC = auto()
    THERMAL_OTHOTROPIC = auto()
    END = auto()

class Conditioner(Enum):
    NONE = auto()
    ERROR = auto()
    UNKNOWN = auto()
    JACOBI = auto()
    BLOCK_JACOBI = auto()
    END = auto()

class Solver(Enum):
    NONE = auto()
    ERROR = auto()
    UNKNOWN = auto()
    PCR = auto()
    PCG = auto()
    NCG = auto()

class Reduce(Enum):
    NONE = auto()
    ERROR = auto()
    UNKNOWN = auto()
    MIN = auto()
    MAX = auto()
    SUM = auto()
    MAX_ABS = auto()
    MIN_ABS = auto()
    SUM_SQUARES = auto()
    END = auto()

class Grid_structure(Enum):
    NONE = auto()
    ERROR = auto()
    UNKNOWN = auto()
    UNSTRUCTURED = auto()
    CARTESIAN = auto() # isotropic (scalar) scaling
    RECTILINEAR = auto() # scaled x,y,z
    GENERAL_LINEAR = auto() # 3x3 transformation matrix
    CYLINDRICAL = auto() # rotate around x, y, or z axis
    SPHERICAL = auto()
    ELLIPTICAL = auto()
    SPHEROIDAL = auto()  # Oblate, Prolate
    CONFORMAL_MAP = auto()
    CURVILINEAR = auto() # orthogonal, non-orthogonal
    BODY_FITTED = auto()
    END = auto()
'''
C++
enum class fmr::grid::Grid_structure {
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
    END
};
'''
# Dictionary of enums ---------------------------------------------------------
fmr_enum ={
    'fmr:Grid_structure': Grid_structure,
    'fmr:Cell_type': Cell_type,
    'fmr:Mtrl_physics': Mtrl_physics,
    'fmr:Conditioner': Conditioner,
    'fmr:Solver': Solver,
    'fmr:Reduce': Reduce,
    'fmr:Part_method': Part_method
}
'''
    # Enums used by Femera (and in JSON)
    Module = auto() # Femera classes
    Data_type = auto()
    Sims_type = auto()
    Grid_structure = auto()
    Shape_type = auto()
    Cell_type = auto()
    Elem_type = auto()
    Mtrl_physics = auto()
    Conditioner = auto()
    Solver = auto()
'''
#==============================================================================
def create_enum_from_csv(file_path, enum_name, start_at=None):
    """
    Create an Enum class dynamically from a CSV file.

    Parameters:
        file_path (str): The path to the CSV file.
        enum_name (str): The name of the Enum class to be created.
        start_at (int, optional): The number to start with when creating the
        Enum members. If None, the CSV file must contain the member's
        corresponding number in the second column. Defaults to None.

    Returns:
        Enum: The dynamically created Enum class.

    Notes:
        - The CSV file should have at least two columns: name and value.
        - If start_at is not provided, the Enum members will be created from the
          first row to the last.
        - If start_at is provided, the Enum members will be created starting
          from the specified row number.
    """
    # Read the CSV file.
    with open(file_path, 'r') as csv_file:
        reader = csv.reader(csv_file)
        # Assuming the first row contains headers. Skip it.
        next(reader, None)
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
            row_i = start_at
            for row in reader:
                if len(row) >= 1:  # Ensure we have at least a name.
                    name = row[0]
                    enum_members[name.upper()] = row_i
                    row_i += 1
    # Create and return the Enum class dynamically.
    return Enum(enum_name, enum_members)

Data_type = create_enum_from_csv(
    os.path.join(os.getcwd(),'data','src','data-type.csv'),
                'Data_type', start_at=3)