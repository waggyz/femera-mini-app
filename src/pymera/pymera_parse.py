#!/usr/bin/env python3
import numpy as np
import json

def fmr_find_user_keys(obj):
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
                    result.update(fmr_find_user_keys(value))
            elif isinstance(value, (dict, list)):
                result.update(fmr_find_user_keys(value))
    elif isinstance(obj, list):
        for item in obj:
            result.update(fmr_find_user_keys(item))
    return result

def parse_sims_from_file(sims, filename):
    #sims = Sims(self)
    with open(filename, 'r') as file:
        sims_json = json.load(file)#TODO use json()
    if 'fmr:Sims' in sims_json:
        sims_json = sims_json['fmr:Sims']
        sims.parameter = fmr_find_user_keys(sims_json)
        
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
