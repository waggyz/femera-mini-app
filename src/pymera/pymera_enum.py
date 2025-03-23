#!/usr/bin/env python3
from enum import Enum, auto
import os, csv

# Enums =======================================================================
class fmr_Enums(Enum): #TODO read from data-types.csv
    # Enums used by Femera (and in JSON)
    fmr_Module = auto() # Femera classes
    fmr_Data_type = auto()
    fmr_Grid_structure = auto()
    fmr_Shape_type = auto()
    fmr_Cell_type = auto()
    fmr_Elem_type = auto()
    fmr_Mtrl_physics = auto()
    fmr_Conditioner = auto()
    fmr_Solver = auto()

class fmr_Module(Enum):
    SIMS = auto()
    GEOM = auto()
    GRID = auto()
    MTRL = auto()
    BCS = auto()
    SOLVE = auto()
    RUNS = auto()
    POST = auto()

class fmr_Grid_structure(Enum):
    UNSTRUCTURED = auto()
    CARTESIAN = auto() # isotropic scaling (scalar)
    RECTILINEAR = auto() # scaled x,y,z
    GENERAL_LINEAR = auto() # 3x3 matrix
    CYLINDRICAL = auto() # x,y,z axis
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
    end
};
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

fmr_Data_type = create_enum_from_csv(
    os.path.join(os.getcwd(),'data','src','data-type.csv'),
                'Data_type', start_at=3)