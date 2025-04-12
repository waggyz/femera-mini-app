#!/usr/bin/env python3
from pymera.enumerators import Data, Application, \
    Grid_structure, Cell_type, \
    Mtrl_physics, Condition_method, Solve_method, Reduce, Part_method
#    Sims, Geom,
from pymera.pymera_libfemerac import libfemerac

import numpy as np
import csv
from warnings import warn

#------------------------------------------------------------------------------
# Class to represent a Sims object.
class Sims:
    def __init__(self, jobs, name=None, version=None, application=None):
        """
        Initializes a Sims object with the given parameters.

        Parameters:
            jobs (Jobs_t): The Jobs object to associate with the Sims object.
            name (str, optional): The name of the Sims object. Defaults to
                the Femera library Sims name.

        Attributes:
            jobs (Jobs_t): The Jobs object associated with the Sims object.
            name (str): The name of the Sims object.
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
        self.jobs=jobs
        self.task_index = libfemerac.fmr_add_sims(jobs.obj)
        if name is not None:
            self.set_name(name)
        if version is not None:
            self.set_version(version)
        if application is not None:
            self.set_application(application)
        #
        # sims_py.set(Application.UQ) #TODO
        # sims_py.set(Sims.NAME, "Fred") #TODO
        # self.set(Sims.App
        #
        self.nominal = {}# dictionary of nominal values
        self.parameter = {}# dictionary of parameters
        #
        self.geometries = []
        self.grids = []
        self.partitions = []
        self.materials = []
        self.bcs = []
        self.solvers = []
        self.post_processors = []
        self.runs = []
        #
    def __enter__(self):
        return(self)
    def __exit__(self, exception_type, exception_value, traceback):
        self.init()
    def __del__(self):
        pass
        #if fmr.fmr_sims_did_init(self.jobs.obj):
        #    fmr.fmr_sims_exit(self.jobs.obj)
        #fmr.fmr_delete_sims(self.jobs.obj)

    def get_name(self):
        return libfemerac.fmr_get_sims_name(self.jobs.obj, self.task_index).decode(
            'utf-8', errors='replace')
    def set_name(self, name):
        if name is not None:
            libfemerac.fmr_set_sims_name(self.jobs.obj, self.task_index,
                                  name.encode('utf-8'))
        
    def get_version(self):
        return libfemerac.fmr_get_sims_version(self.jobs.obj, self.task_index).decode(
            'utf-8', errors='replace')
    def set_version(self, vers):
        if vers is not None:
            libfemerac.fmr_set_sims_version(self.jobs.obj, self.task_index,
                                    vers.encode('utf-8'))
    def get_application(self):
        return Application(
            libfemerac.fmr_get_sims_application(self.jobs.obj, self.task_index))
    def set_application(self, app):
        return Application(
            libfemerac.fmr_set_sims_application(self.jobs.obj, self.task_index,
                                          app.value))
    
    def add_parameter(self, name, type=None, values=None, nominal=None):
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

    def set_parameter(self, name, values=None, nominal=None):#TODO same as add_?
        if isinstance(nominal, (list, tuple, set)):
            if len(nominal) > 1:
                nominal = np.ascontiguousarray(nominal)
        self.nominal[name] = nominal
        if isinstance(values, (list, tuple, set)):
            if len(values) > 1:
                values = np.ascontiguousarray(values)
        self.parameter[name] = values
    #def set_parameter(self, component, parameter, value):
    #    if component not in self.parameters:
    #        self.parameters[component] = {}
    #    self.parameters[component][parameter] = value
    
    def get_parameter(self, component, parameter):
        return self.parameters[component][parameter]

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
        
        The function sets the parameter "sample_n" to the minimum number of
        values of all parameters.
        """
        with open(filename, 'r') as f:
            reader = csv.reader(f)
            if has_names: # first line has parameter names
                names = next(reader)
            else: # Make placeholder names: param_1, param_2,, param_3,...
                line = next(reader)# need a line to get length
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
                    self.nominal[name] = float(val)#TODO Check type of each val?
            for row_col in reader:
                for name, col in zip(names, row_col):
                    self.parameter[name].append(float(col))
            self.parameter["sample_n"] = min(len(self.parameter[name])
                                              for name in names)
            if(self.parameter["sample_n"] > 1):
                # Convert lists to contiguous numpy arrays
                for key in self.parameter:
                    self.parameter[key] = np.ascontiguousarray(
                        self.parameter[key])
            if(self.parameter["sample_n"] != max(len(self.parameter[name])
                                                for name in names)):
                warn('\n pym Jobs WARN Not all parameters have the same number '
                     +'of values. '
                     +'Something might be wrong with your CSV file.')

    def get_post(self, name):
        # Placeholder for getting post-processing results
        # In a real implementation, this would return actual data.
        return np.ones(self.parameter["sample_n"])

    def add_geometry(self, name, shape):
        #self.geometries.append({"name": name, "shape": shape})
        pass

    def add_grid(self, name, for_geometry=None, type=None, method=None, elem=None):
        '''if for_geometry is None and self.geometries:
            for_geometry = self.geometries[-1]["name"]
        self.grids.append({
            "name": name,
            "for": for_geometry,
            "type": type,
            "method": method,
            "elem": elem
        })'''
        pass

    def set_bcs(self, name, at, set, to):
        '''self.bcs = []
        self.bcs.append({
            "name": name,
            "at": at,
            "set": set,
            "to": to
        })'''
        pass

    def add_bcs(self, name, at):
        '''self.bcs.append({
            "name": name,
            "at": at
        })'''
        pass

    def set_material(self, name, physics):
        '''self.materials.append({
            "name": name,
            "physics": physics
        })'''
        pass

    def set_preconditioner(self, name=None, method=None):
        '''self.preconditioner = {
            "name" : name,
            "method": method
        }'''
        pass

    def set_solver(self, name, method,
                   analysis='fmr:solve:static', load_step_n=1, rtol=1e-6):
        '''self.solve = {
            "name": name,
            "method": method,
            "analysis": analysis,
            "load_step_n": load_step_n,
            "rtol": rtol
        }'''
        pass

    def add_post(self, name, at, sum, count=-1):
        #if(count<0):
        #    self.count=self.parameter["sample_n"]
        '''self.post_processes.append({
            "name": name,
            "at": at,
            "sum": sum,
            "count": count
        })'''
        pass

    def get_post(self, name):
        # Placeholder for getting post-processing results
        # In a real implementation, this would return actual data.
        return np.ones(self.parameter["sample_n"])

    def init(self):
        # Placeholder for initialization logic
        pass

    def run(self):
        # Placeholder for run logic
        pass

    def exit(self):
        # Placeholder for cleanup logic
        pass

