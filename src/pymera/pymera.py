#!/usr/bin/env python3
import pymera_enum

import numpy as np
import ctypes as ct
import csv, json
from warnings import warn

import os
libpath = os.path.join(os.getcwd(),'build','stage','i7-12800H','lib')

# Load the shared library
if os.name == 'posix':
    fmr = ct.CDLL(libpath + '/libfemerac.so')#TODO change fmr to jobs?
elif os.name == 'nt':
    # lib = ctypes.CDLL('./jobs.dll')
    raise OSError("Unsupported operating system")
else:
    raise OSError("Unsupported operating system")

# C/Python ctypes interface ===================================================
# Define the argument and return ctypes for C interface functions.

fmr_Dim_int = ct.c_ubyte
fmr_Exit_int = ct.c_int32
fmr_Enum_int = ct.c_int32
fmr_Local_int = ct.c_uint32
fmr_Global_int = ct.c_uint64

# Pass Jobs_t* as ct.c_void_p
fmr.fmr_new_jobs.restype = ct.c_void_p
fmr.fmr_delete_jobs.argtypes = [ct.c_void_p]

fmr.fmr_jobs_init.argtypes = [ct.c_void_p]
fmr.fmr_jobs_exit.argtypes = [ct.c_void_p]
fmr.fmr_jobs_did_init.restype = ct.c_bool
fmr.fmr_jobs_did_init.argtypes = [ct.c_void_p]

fmr.fmr_get_version.restype = ct.c_char_p
fmr.fmr_get_version.argtypes = [ct.c_void_p]
fmr.fmr_get_jobs_name.restype = ct.c_char_p
fmr.fmr_get_jobs_name.argtypes = [ct.c_void_p]

fmr.fmr_get_verbosity.restype = fmr_Dim_int
fmr.fmr_get_verbosity.argtypes = [ct.c_void_p]
fmr.fmr_set_verbosity.restype = fmr_Dim_int
fmr.fmr_set_verbosity.argtypes = [ct.c_void_p, fmr_Dim_int]

fmr.fmr_get_sims_n.restype = fmr_Local_int
fmr.fmr_get_sims_n.argtypes = [ct.c_void_p]
fmr.fmr_add_sims.argtypes = [ct.c_void_p]
fmr.fmr_add_sims.restype = fmr_Local_int

fmr.fmr_get_sims_name.restype = ct.c_char_p
fmr.fmr_get_sims_name.argtypes = [ct.c_void_p, fmr_Local_int]
fmr.fmr_set_sims_name.argtypes = [ct.c_void_p, fmr_Local_int, ct.c_char_p]

fmr.fmr_get_sims_version.restype = ct.c_char_p
fmr.fmr_get_sims_version.argtypes = [ct.c_void_p, fmr_Local_int]
fmr.fmr_set_sims_version.argtypes = [ct.c_void_p, fmr_Local_int, ct.c_char_p]

#==============================================================================
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

# Python classes ==============================================================
# Create a class to represent a Jobs object in Python.
class Jobs:
    Data_type = pymera_enum.create_enum_from_csv(
        os.path.join(os.getcwd(),'data','src','data-type.csv'),
                    'Data_type', start_at=3)
    def __init__(self):
        self.obj = fmr.fmr_new_jobs()
    def __del__(self):
        if fmr.fmr_jobs_did_init(self.obj):
            fmr.fmr_jobs_exit(self.obj)
        fmr.fmr_delete_jobs(self.obj)
    
    def get_sims_n(self):
        return fmr.fmr_get_sims_n(self.obj)
    
    #NOTE there is no method to remove Sims. It would mess up the indexing.
    
    def add_sims(self, sims_name=None):
        return Sims(self, name=sims_name)

    def add_sims_from_file(self, filename):
        sims = Sims(self)
        with open(filename, 'r') as file:
            sims_json = json.load(file)#TODO use json()
        if 'fmr:Sims' in sims_json:
            sims_json = sims_json['fmr:Sims']
            sims.parameter = find_user_keys(sims_json)
            
            if 'fmr:Data:Name' in sims_json:
                sims.set_name(sims_json['fmr:Data:Name'])
            if 'fmr:Data:Version' in sims_json:
                sims.set_version(sims_json['fmr:Data:Version'])
            
        else:
            print('Found no fmr:Sims in ' + filename)
            return
        #if 'fmr:Data:Name' in sims_json:
        #    sims.name=sims_json['fmr:Data:Name']
        #self.add_sims(name=sims_json['name'], runs_n=sims_json['runs_n'])
        return sims

    def init(self):
        fmr.fmr_jobs_init(self.obj)

    def exit(self):
        fmr.fmr_jobs_exit(self.obj)

    def did_init(self):
        return fmr.fmr_jobs_did_init(self.obj)
    
    def get_version(self):
        return fmr.fmr_get_version(self.obj).decode('utf-8', errors='replace')
    
    def get_name(self):
        return fmr.fmr_get_jobs_name(self.obj).decode('utf-8', errors='replace')
    
    def get_verbosity(self):
        return fmr.fmr_get_verbosity(self.obj)

    def set_verbosity(self, verbosity=3):
        # Clamp values to range of ct.c_ubyte.
        if(verbosity<0):
            verbosity=0
        elif(verbosity>255):
            verbosity=255
        return fmr.fmr_set_verbosity(self.obj, verbosity)

#------------------------------------------------------------------------------
# Create a class to represent a Sims object in Python.
class Sims:
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
    def __init__(self, jobs, name=None):
        if not isinstance(jobs, Jobs):
            warn('\n pym sims WARN Sims must be created from Jobs.\n'
                 + '               '
                 + 'Try one of the add_sims* methods of a Jobs object.')
            return None
        self.jobs=jobs
        self.task_index = fmr.fmr_add_sims(jobs.obj)
        self.set_name(name)
        #
        self.nominal = {}
        self.parameter = {}
        self.sample_n = None
        #
        self.runs_n = 1 #TODO remove?
        #
        self.geometries = []
        self.grids = []
        self.bcs = []
        self.materials = []
        self.preconditioner = None
        self.solve = None
        self.post_processes = []
        self.partition_n = 1
        #self.json_nominal = {}
        #self.json_parameter = {}
    def __del__(self):
        pass
        #if fmr.fmr_sims_did_init(self.jobs.obj):
        #    fmr.fmr_sims_exit(self.jobs.obj)
        #fmr.fmr_delete_sims(self.jobs.obj)

    def get_name(self):
        return fmr.fmr_get_sims_name(self.jobs.obj, self.task_index).decode(
            'utf-8', errors='replace')
    def set_name(self, name):
        if name is not None:
            fmr.fmr_set_sims_name(self.jobs.obj, self.task_index,
                                  name.encode('utf-8'))
        
    def get_version(self):
        return fmr.fmr_get_sims_version(self.jobs.obj, self.task_index).decode(
            'utf-8', errors='replace')
    def set_version(self, vers):
        if vers is not None:
            fmr.fmr_set_sims_version(self.jobs.obj, self.task_index,
                                     vers.encode('utf-8'))
    
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
                warn('\n pym Jobs WARN Not all parameters have the same number '
                     +'of values. '
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

