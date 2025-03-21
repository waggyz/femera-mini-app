#!/usr/bin/env python3
import numpy as np
import ctypes as ct
import csv, json
from enum import Enum
from warnings import warn

import os
libpath = os.path.join(os.getcwd(),'build','stage','i7-12800H','lib')

# Load the shared library
if os.name == 'posix':
    fmr = ct.CDLL(libpath + '/libfemerac.so')
elif os.name == 'nt':
    # lib = ctypes.CDLL('./jobs.dll')
    raise OSError("Unsupported operating system")
else:
    raise OSError("Unsupported operating system")

# Enums =======================================================================

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
                    enum_members[name] = value
        else:
            row_i = start_at
            for row in reader:
                if len(row) >= 1:  # Ensure we have at least a name.
                    name = row[0]
                    enum_members[name] = row_i
                    row_i += 1
    # Create and return the Enum class dynamically.
    return Enum(enum_name, enum_members)

def find_user_keys(obj):
    """
    Recursively traverses a nested dictionary or list and returns a dictionary
    containing the keys that are not prefixed with "fmr:" and their
    corresponding values.
    
    Parameters:
        obj (dict or list): The object to traverse.
        
    Returns:
        dict: A dictionary containing the keys that are not prefixed with "fmr:"
        and their corresponding values.
    """
    result = {}
    if isinstance(obj, dict):
        for key, value in obj.items():
            if not key.startswith("fmr:") and isinstance(value, dict):
                #TODO resolve datatype of value key.
                nominal = next(iter(value.values()))
                if isinstance(nominal, (list, tuple, set)):
                    if len(nominal) > 1:
                        nominal = np.ascontiguousarray(nominal)
                elif not isinstance(nominal, (int, float, str)):
                    nominal = None #str(next(iter(value.keys())))# datatype
                result[key] = nominal
                if isinstance(value, (dict, list)):
                    result.update(find_user_keys(value))
            elif isinstance(value, (dict, list)):
                result.update(find_user_keys(value))
    elif isinstance(obj, list):
        for item in obj:
            result.update(find_user_keys(item))
    return result
#==============================================================================
#TODO Need specifiers for
#    fmr::Exit_int, fmr::Dim_int, fmr::Enum_int, fmr::Local_int, fmr::Global_int
#    ...
# Define the argument and return types for C interface functions.
fmr.new_jobs.restype = ct.c_void_p
fmr.delete_jobs.argtypes = [ct.c_void_p]

fmr.jobs_init.argtypes = [ct.c_void_p]
fmr.jobs_exit.argtypes = [ct.c_void_p]
fmr.jobs_did_init.restype = ct.c_bool
fmr.jobs_did_init.argtypes = [ct.c_void_p]

fmr.get_version.restype = ct.c_char_p
fmr.get_version.argtypes = [ct.c_void_p]
fmr.get_jobs_name.restype = ct.c_char_p
fmr.get_jobs_name.argtypes = [ct.c_void_p]

fmr.get_verbosity.restype = ct.c_ubyte
fmr.get_verbosity.argtypes = [ct.c_void_p]
fmr.set_verbosity.restype = ct.c_ubyte
fmr.set_verbosity.argtypes = [ct.c_void_p, ct.c_ubyte]

fmr.get_sims_name.restype = ct.c_char_p
fmr.get_sims_name.argtypes = [ct.c_void_p]
fmr.set_sims_name.argtypes = [ct.c_void_p, ct.c_char_p]
fmr.get_sims_version.restype = ct.c_char_p
fmr.get_sims_version.argtypes = [ct.c_void_p]
fmr.set_sims_version.argtypes = [ct.c_void_p, ct.c_char_p]


# Create a class to represent the Jobs object in Python.
class Jobs:
    Data_type = create_enum_from_csv(
        os.path.join(os.getcwd(),'data','src','data-type.csv'),
                    'Data_type', start_at=3)
    def __init__(self):
        self.obj = fmr.new_jobs()

    def __del__(self):
        if fmr.jobs_did_init(self.obj):
            fmr.jobs_exit(self.obj)
        fmr.delete_jobs(self.obj)
    
    def new_sims(self, name='fmr:user:sims', runs_n=1):
        return Sims(self, name=name, runs_n=runs_n)

    def new_sims_from_file(self, filename):
        sims = Sims(self)
        with open(filename, 'r') as file:
            sims_json = json.load(file)#TODO use json()
        if 'fmr:Sims' in sims_json:
            sims_json = sims_json['fmr:Sims']
            sims.parameter = find_user_keys(sims_json)
            if 'fmr:Data_type:Name' in sims_json:
                self.set_sims_name(sims_json['fmr:Data_type:Name'])
            if 'fmr:Data_type:Version' in sims_json:
                self.set_sims_version(sims_json['fmr:Data_type:Version'])
        else:
            print('Found no fmr:Sims in ' + filename)
            return
        #if 'fmr:Data_type:Name' in sims_json:
        #    sims.name=sims_json['fmr:Data_type:Name']
        #self.new_sims(name=sims_json['name'], runs_n=sims_json['runs_n'])
        return sims

    def init(self):
        fmr.jobs_init(self.obj)

    def exit(self):
        fmr.jobs_exit(self.obj)

    def did_init(self):
        return fmr.jobs_did_init(self.obj)
    
    def get_version(self):
        return fmr.get_version(self.obj).decode('utf-8', errors='replace')
    
    def get_name(self):
        return fmr.get_jobs_name(self.obj).decode('utf-8', errors='replace')
    
    def get_verbosity(self):
        return fmr.get_verbosity(self.obj)
    def set_verbosity(self, verbosity=3):
        # Clamp values to range of ct.c_ubyte.
        if(verbosity<0):
            verbosity=0
        elif(verbosity>255):
            verbosity=255
        return fmr.set_verbosity(self.obj, verbosity)
    #TODO move to Sims class ==================================================
    def get_sims_name(self):
        return fmr.get_sims_name(self.obj).decode('utf-8', errors='replace')
    def set_sims_name(self, name):
        fmr.set_sims_name(self.obj, name.encode('utf-8'))
        return
    
    def get_sims_version(self):
        return fmr.get_sims_version(self.obj).decode('utf-8', errors='replace')
    def set_sims_version(self, name):
        fmr.set_sims_version(self.obj, name.encode('utf-8'))
        return
    #==========================================================================

# Create a class to represent the Sims object in Python.
class Sims:
    def __init__(self, jobs, name=None, runs_n=1):
        """
        Initializes a Sims object with the given parameters.

        Parameters:
            jobs (Jobs_t): The Jobs object to associate with the Sims object.
            name (str, optional): The name of the Sims object. Defaults to
                the Femera library Sims name.
            runs_n (int, optional): The number of runs for the Sims object.
                Defaults to 1.

        Attributes:
            jobs (Jobs_t): The Jobs object associated with the Sims object.
            name (str): The name of the Sims object.
            runs_n (int): The number of runs for the Sims object.
            geometries (list): The list of geometries associated with the Sims
                object.
            grids (list): The list of grids associated with the Sims object.
            bcs (list): The list of boundary conditions associated with the Sims
                object.
            materials (list): The list of materials associated with the Sims
                object.
            preconditioner (object or None): The preconditioner associated with
                the Sims object. Defaults to None.
            solve (object or None): The solver associated with the Sims object.
                Defaults to None.
            post_processes (list): The list of post-processing functions
                associated with the Sims object.
            parameters (dict): The parameters associated with the Sims object.
            partition_n (int): The partition number associated with the Sims
                object. Defaults to 1.
        """
        self.jobs = jobs
        self.runs_n = runs_n
        self.geometries = []
        self.grids = []
        self.bcs = []
        self.materials = []
        self.preconditioner = None
        self.solve = None
        self.post_processes = []
        self.partition_n = 1
        self.nominal = {}
        self.parameter = {}
        #self.json_nominal = {}
        #self.json_parameter = {}
        self.sample_n = None

    def add_parameter(self, name, nominal=None, values=None):
        """
        Adds a parameter to the Sims object.

        Parameters:
            name (str): The name of the parameter.

            nominal (Optional[Union[int, float, str, np.ndarray]]): The nominal
            value(s) of the parameter. If a single value is provided, it will be
            stored as a scalar. If multiple values are provided, they will be
            stored as a numpy contiguous array. Defaults to None.
            
            values (Optional[Union[int, float, str, np.ndarray]]): The values of
            the parameter. If a single value is provided, it will be stored as a
            scalar. If multiple values are provided, they will be stored as a
            numpy contiguous array. Defaults to None.

        Returns:
            None
        """
        if isinstance(nominal, (list, tuple, set)):
            if len(nominal) > 1:
                nominal = np.ascontiguousarray(nominal)
        self.nominal[name] = nominal
        if isinstance(values, (list, tuple, set)):
            if len(values) > 1:
                values = np.ascontiguousarray(values)
        self.parameter[name] = values

    def add_parameter_file(self, filename, has_names=True, has_nominals=False):
        """
        Adds parameters from a CSV file to the parameter dictionary and the
        nominal dictionary. The function takes the following arguments:
        
        - filename (str): The path to the CSV file containing the parameters.
        - has_names (bool, optional): Whether the first line of the CSV file
          contains parameter names. Defaults to True.
        - has_nominals (bool, optional): Whether the next line of the CSV file
          contains nominal values. Defaults to False.
        
        The function reads the CSV file and populates the parameter and nominal
        dictionaries with the values. If has_names is True, the function assumes
        that the first line of the CSV file contains the parameter names and
        creates an empty list for each parameter. If has_nominals is True, the
        function assumes that the second line of the CSV file contains the
        nominal values and sets each nominal value to None. The function then
        appends each value from the remaining lines of the CSV file to the
        corresponding parameter list. If the number of values for a parameter is
        greater than 1, the function converts the list to a contiguous numpy
        array.
        
        The function sets the sample_n attribute to the minimum number of values
        of all parameters.
        """
        with open(filename, 'r') as f:
            reader = csv.reader(f)
            if has_names: # first line has parameter names
                names = next(reader)
            else: # Make placeholder names: param_1, param_2,, param_3,...
                line = next(reader)
                names =  [f'param_{i+1}' for i in range(len(line))]
                f.seek(0)# Return to top.
            # Make dictionaries having keys for names and empty arrays (for
            # parameter) or None (for nominal) for values.
            for name in names:
                self.parameter[name] = []
            self.nominal = {name: None for name in names}
            # The next (second) line has the nominal values (if provided)
            if has_nominals:
                nominals = next(reader)
                for name, val in zip(names, nominals):
                    self.nominal[name] = float(val)#TODO Check type for each val?
            for row_col in reader:
                for name, col in zip(names, row_col):
                    self.parameter[name].append(float(col))
            self.sample_n = min(len(self.parameter[name]) for name in names)
            if(self.sample_n > 1):
                # Convert lists to contiguous numpy arrays
                for key in self.parameter:
                    self.parameter[key] = np.ascontiguousarray(
                        self.parameter[key])
            if(self.sample_n!=max(len(self.parameter[name]) for name in names)):
                warn('Not all parameters have the same number of values. '
                     +'Something might be wrong with your CSV file.')

    def get_post(self, name):
        # Placeholder for getting post-processing results
        # In a real implementation, this would return actual data.
        return np.ones(self.runs_n)

    def set_partition_n(self, n=1):
        self.partition_n = n

    def add_geometry(self, name, shape):
        self.geometries.append({"name": name, "shape": shape})

    def add_grid(self, name, for_geometry=None, type=None, method=None, elem=None):
        if for_geometry is None and self.geometries:
            for_geometry = self.geometries[-1]["name"]
        self.grids.append({
            "name": name,
            "for": for_geometry,
            "type": type,
            "method": method,
            "elem": elem
        })

    def set_bcs(self, name, at, set, to):
        self.bcs = []
        self.bcs.append({
            "name": name,
            "at": at,
            "set": set,
            "to": to
        })

    def add_bcs(self, name, at):
        self.bcs.append({
            "name": name,
            "at": at
        })

    def set_material(self, name, physics):
        self.materials.append({
            "name": name,
            "physics": physics
        })

    def set_preconditioner(self, name=None, method=None):
        self.preconditioner = {
            "name" : name,
            "method": method
        }

    def set_solver(self, name, method,
                   analysis='fmr:solve:static', load_step_n=1, rtol=1e-6):
        self.solve = {
            "name": name,
            "method": method,
            "analysis": analysis,
            "load_step_n": load_step_n,
            "rtol": rtol
        }

    def add_post(self, name, at, sum, count=-1):
        #if(count<0):
        #    self.count=self.runs_n
        self.post_processes.append({
            "name": name,
            "at": at,
            "sum": sum,
            "count": count
        })

    def get_post(self, name):
        # Placeholder for getting post-processing results
        # In a real implementation, this would return actual data.
        return np.ones(self.runs_n)

    def set_parameter(self, component, parameter, value):
        if component not in self.parameters:
            self.parameters[component] = {}
        self.parameters[component][parameter] = value
    
    def get_parameter(self, component, parameter):
        return self.parameters[component][parameter]

    def init(self):
        # Placeholder for initialization logic
        pass

    def run(self):
        # Placeholder for run logic
        pass

    def exit(self):
        # Placeholder for cleanup logic
        pass


