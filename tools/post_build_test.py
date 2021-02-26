#!/usr/bin/python
# -*- coding: utf8 -*-
import os
import sys
sys.path.append('tools/testy')
import testy
import check_code_graffiti as cg

def testy_exe_test( *args, **opts ):
  if len(sys.argv) >1:
    # sys.argv[ 0 ] contains the path and name of this file.
    src_dir = sys.argv[ 1 ]# first argument is source directory.
    if len(sys.argv) > 2:
      build_dir = sys.argv[ 2 ]# second argument is build directory.
    else:
      build_dir = src_dir
    #
    #==========================================================================
    suite = testy.Suite('Source code statistics', quiet_check_pass=True)
    #
    check = testy.Exe(['find src \( -name \'*.?pp\' \) '
        +'-printf x | wc -c'],
      shell=True).run()
    if check.stdout == '0':
      check.will('FAIL', because='cannot find C++ source code.')
    else:
      check.will('INFO', because='found '+check.stdout
        +' Femera C++ source code files...')
    check.done( suite )
    #
    check = testy.Exe(['find src \( -name \'*gtst.?pp\' \) '
        +'-printf x | wc -c'],
      shell=True).run()
    if check.stdout == '0':
      check.will('FAIL', because='cannot find C++ test code.')
    else:
      check.will('INFO', because='...including '+check.stdout
        +' test code files.')
    check.done( suite )
    #
    check = testy.Exe(['(find src \( -name \'*.?pp\' \)'
        +' -print0 | xargs -0 cat) | wc -l'],
      shell=True).run()
    if check.stdout == '0':
      check.will('FAIL', because='found no lines of C++ test code.')
    else:
      check.will('INFO', because='counted '+check.stdout
        +' lines of C++ code...')
    check.done( suite )
    #
    check = testy.Exe(['(find src \( -name \'*gtst.?pp\' \)'
        +' -print0 | xargs -0 cat) | wc -l'],
      shell=True).run()
    if check.stdout == '0':
      check.will('FAIL', because='found no lines of C++ test code.')
    else:
      check.will('INFO', because='...including '+check.stdout
        +' lines of test code.')
    check.done( suite )
    #
    suite.done( summarize=False )
    #
    #--------------------------------------------------------------------------
    suite = testy.Suite('Post-build code graffiti tests',
      because='Clean source code before release.', quiet_check_pass=True)
    #
    fm='fixme'.upper()
    td='todo'.upper()
    check_dirs='src data docs examples extras tests tools'
    check_files='Makefile README* config*'
    #
    check = testy.Exe(['grep -rn '+td+' '+check_dirs+' '+check_files
      +' | tee '+build_dir+'/'+td+'.txt | wc -l'], shell=True).run()
    if check.stdout =='0':
      check.will('PASS', because='no '+td+' found in '+check_dirs+'...')
    else:
      check.will('todo', because='found '+check.stdout+' '+td
        +' in sources: cat '+build_dir+'/'+td+'.txt')
    check.done( suite )
    #
    check = testy.Exe(['grep -rn '+fm+' '+check_dirs+' '+check_files
      +' | tee '+build_dir+'/'+fm+'.txt | wc -l'], shell=True).run()
    if check.stdout =='0':
      check.will('PASS', because='no '+fm+' found in '+check_dirs+'...')
    else:
      check.will('todo', because='found '+check.stdout+' '+fm
        +' in sources: cat '+build_dir+'/'+fm+'.txt')
    check.done( suite )
    #
    suite.done( summarize=False )
    #
    #--------------------------------------------------------------------------
    check_dirs='Makefile README* config*'
    #
    suite = testy.Suite('Build error and warning summary',
      because='Clean build before release.', quiet_check_pass=True)
    check = testy.Exe(['(find '+build_dir+' -name \'*.err\' '
      +'| xargs grep -in " error:") | tee '+build_dir+'/ERROR.txt | wc -l'],
      shell=True).run()
    if check.stdout =='0':
      check.will('PASS', because='no errors found in .err logs.')
    else:
      check.will('FAIL', because='found '+check.stdout+' errors in .err logs:'
        +' cat '+build_dir+'/ERROR.txt' )
    check.done( suite )
    check = testy.Exe(['(find '+build_dir+' -name \'*.err\' '
      +'| xargs grep -in warn) | tee '+build_dir+'/'+'wARNING'.upper()
      +'.txt | wc -l'], shell=True).run()
    if check.stdout =='0':
      check.will('PASS', because='no warnings found in .err logs.')
    else:
      check.will('WARN', because='found '+check.stdout+' warnings in .err logs:'
        +' cat '+build_dir+'/'+'wARNING'.upper()+'.txt' )
    check.done( suite )
    #
    suite.done( summarize=False )
    #
    #--------------------------------------------------------------------------
    suite = testy.Suite('Post-build checksum tests',
      because='Update md5 checksums before release.', quiet_check_pass=True)
    # Check if the user has altered content in src/ data/, tools/, extras/,
    #       even if not in a git clone.
    #
    for md5_dir in ('data','extras','src','tools'):
      md5_file = md5_dir+'/.md5'
      check = testy.Exe(['cat',md5_file]).run()
      if check.retval ==0:
        check.will('PASS', because=md5_file+' found.')
        repo_md5 = check.stdout
      else:
        check.will('FAIL', because='did not find '+md5_file+'.')
        repo_md5 = None
      check.done( suite )
      #
      check = testy.Exe(['tools/md5-dir-content.sh '+md5_dir], shell=True).run()
      if check.retval ==0:
        local_md5 = check.stdout
        if local_md5 ==repo_md5:
          check.will('PASS', because=''+md5_dir+'/.md5 matches local md5.')
        else:
          check.will('note', because=''+md5_dir+'/.md5 does not match local md5: '
              +''+local_md5+''+'')
      else:
        check.will('FAIL', because='did make '+md5_dir+'/ md5.')
        local_md5 = None
      check.done( suite )
    #
    suite.done(summarize=False)
    #
    #--------------------------------------------------------------------------
    #
  return

if __name__ == "__main__":
  ''' Testy post-build tests '''
  # Check myself for graffiti
  cg.check_graffiti( sys.argv[0] )
  #
  testy_exe_test()
