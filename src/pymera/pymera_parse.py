#!/usr/bin/env python3
from pymera_enum import fmr_Data_type, fmr_Grid_structure

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
            if key=='fmr:Data:Grid_structure':
                result[key] = fmr_Grid_structure[
                        value.split('Grid_structure:',1)[1]]
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

def fmr_parse_sims_from_file(sims, filename):
    #sims = Sims(self)
    with open(filename, 'r') as file:
        sims_json = json.load(file)#TODO use json()
    if 'fmr:Sims' in sims_json:
        sims_json = sims_json['fmr:Sims']
        sims.parameter = fmr_find_user_keys(sims_json)
        #
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
'''
def fmr_flatten_json(data, prefix=''):
    result = {}
    for key, value in data.items():
        new_key = f"{prefix}|{key}" if prefix else key
        if isinstance(value, dict):
            result.update(fmr_flatten_json(value, new_key))
        elif isinstance(value, list):
            for i, item in enumerate(value):
                if isinstance(item, dict):
                    result.update(fmr_flatten_json(item, f"{new_key}|{i}"))
                else:
                    result[f"{new_key}|{i}"] = item
        else:
            result[new_key] = value
    return result
'''


def fmr_is_terminal_list(lst):
    return all(isinstance(item, (float, int, bool)) for item in lst)

def fmr_flatten_json(data, prefix='', sep='\x1f'):
    # \x1f (\31): ASCII unit separator
    result = {}
    for key, value in data.items():
        new_key = f"{prefix}{sep}{key}" if prefix else key
        if isinstance(value, dict):
            result.update(fmr_flatten_json(value, new_key, sep))
        elif isinstance(value, list):
            if fmr_is_terminal_list(value):
                result[new_key] = value
            else:
                for i, item in enumerate(value):
                    if isinstance(item, dict):
                        result.update(fmr_flatten_json(
                            item, f"{new_key}{sep}{i}", sep))
                        #result.update(fmr_flatten_json(item, f"{new_key}[{i}]", sep))
                    else:
                        result[f"{new_key}{sep}{i}"] = item
                        #result[f"{new_key}[{i}]"] = item
        else:
            result[new_key] = value
    return result
'''
def fmr_flatten_json(data, prefix='', index=0):
    result = {}
    if isinstance(data, dict):
        for key, value in data.items():
            new_key = f"{prefix}[{index}]|{key}" if prefix else key
            if isinstance(value, dict):
                result.update(fmr_flatten_json(value, new_key))
            elif isinstance(value, list):
                if fmr_is_terminal_list(value):
                    result[new_key] = value
                else:
                    for i, item in enumerate(value):
                        result.update(fmr_flatten_json(item, new_key, i))
            else:
                result[new_key] = value
    elif isinstance(data, list):
        for i, item in enumerate(data):
            result.update(fmr_flatten_json(item, prefix, i))
    else:
        result[f"{prefix}[{index}]"] = data
    return result
'''
def fmr_flatten_file(filename):
    with open(filename, 'r') as file:
        json_data = json.load(file)
    return fmr_flatten_json(json_data)


