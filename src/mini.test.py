#!/usr/bin/python
# -*- coding: utf8 -*-
import os
import sys
sys.path.append('tools/testy')
import testy
import check_code_graffiti as cg
import re

def check_mini_exe():
  if len(sys.argv) >1:
    #NOTE sys.argv[ 0 ] contains the path and name of this file.
    exe = sys.argv[ 1 ]# First argument is executable name.
    if len(sys.argv) > 2:
      build_dir=sys.argv[ 2 ]# optional second argument is build directory.
    else:
      build_dir = os.path.dirname( exe )
    #
    suite = testy.Suite('Test command line options for '+exe,
      because='README describes command line option behavior.',
      quiet_check_pass=True )
    #
    ver_file = build_dir+'/VERSION'
    build_ver = testy.Exe(['cat',ver_file]).run()#, 'Version file contains text').run()
    ver = build_ver.stdout
    if len(ver) > 2:
      build_ver.will('PASS',ver_file+' contains '+str(len(ver))+' characters.')
    else:
      build_ver.will('FAIL',ver_file+' is too short.')
    build_ver.done(suite)
    #
    if re.match('^[a-z0-9+\.-]+$', ver ) :
      build_ver.will('PASS','version contains only valid characters.')
    else:
      build_ver.will('FAIL','contains characters NOT in alphanum-+.')
    build_ver.done(suite)
    #
    semver2=('^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)'
      +'(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)'
      +'(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))'
      +'?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$')# https://semver.org/
    if re.match( semver2, ver ) :
      build_ver.will('PASS',
        'version complies with Semantic Versioning 2.0, https://semver.org/')
    else:
      build_ver.will('FAIL',
        'version violates Semantic Versioning 2.0, https://semver.org')
    build_ver.done(suite)
    #
    check_v = testy.Exe([ exe,'-v']).run()#, '-v: version is '+ver).run()
    if check_v.stdout == ver :
      check_v.will('PASS','-v: version is '+check_v.stdout+'.')
    else:
      check_v.will('FAIL','-v: version is NOT '+build_ver.stdout+'.')
    check_v.done(suite)
    #
    check = testy.Exe([ exe,'--version']).run()#, '--version same as -v').run()
    if check.stdout == check_v.stdout:
      check.will('PASS','--version is '+check.stdout+'.')
    else:
      check.will('FAIL','--version is NOT '+check_v.stdout+'.')
    check.done(suite)
    #
    check = testy.Exe([ exe,'-dv']).run()
    if len(check.stdout) > len(check_v.stdout):
      check.will('PASS','-dv: detail version is longer than -v.')
    else:
      check.will('FAIL','-dv: has no additional details.')
    check.done(suite)
    #
    help_file = 'data/mini-help.en.txt'
    check_help = testy.Exe(['cat',help_file]).run()#,'Help file contains some text').run()
    if len(check_help.stdout) > 8:
      check_help.will('PASS',help_file+': contains '
        +str(len(check_help.stdout))+' characters.')
    else:
      check_help.will('FAIL',help_file+': is too short.')
    check_help.done(suite)
    #
    check = testy.Exe(['cut','-c1-80',help_file]).run()#,
      #'Help fits in 80 char width').run()
    if check.stdout == check_help.stdout:
      check.will('PASS',help_file+': text fits in 80 character width.')
    else:
      check.will('FAIL',help_file+': text does NOT fit in 80 character width.')
    check.done(suite)
    #
    check_h = testy.Exe([ exe,'-h']).run()#, '-h: help').run()
    if check_help.stdout in check_h.stdout :
      check_h.will('PASS','-h: stdout contains '+help_file+'.')
    else:
      check_h.will('FAIL','-h: stdout does NOT contain '+help_file+'.')
    check_h.done(suite)
    #
    check_ddhelp = testy.Exe([ exe,'--help' ]).run()
    if check_help.stdout in check_ddhelp.stdout :
      check_ddhelp.will('PASS','--help contains '+help_file+'.')
    else:
      check_ddhelp.will('FAIL','--help does NOT contain '+help_file+'.')
    check_ddhelp.done(suite)
    #
    license_file = 'LICENSE'
    check_lic = testy.Exe(['cat',license_file]).run()#,
      #'LICENSE file contains some text').run()
    if len(check_lic.stdout) > 8:
      check_lic.will('PASS',license_file+': contains '
        +str(len(check_lic.stdout))+' characters.')
    else:
      check_lic.will('FAIL',license_file+': is empty.')
    check_lic.done(suite)
    #
    if False:
      check = testy.Exe(['cut','-c1-80',license_file]).run()#,
        #'License fits in 80 char width').run()
      if check.stdout == check_lic.stdout:
        check.will('PASS',license_file+': text fits in 80 character width.')
      else:
        check.will('FAIL',license_file+': text does NOT fit in 80 character width.')
      check.done(suite)
      #
      check_l = testy.Exe([ exe,'-l']).run()#, '-l: license').run()
      if check_lic.stdout in check_l.stdout :
        check_l.will('PASS','-l: stdout contains '+license_file+'.')
      else:
        check_l.will('FAIL','-l: stdout does NOT contain '+license_file+'.')
      check_l.done(suite)
      #
      check = testy.Exe([ exe,'--license: same as -l' ]).run()#,
        #'--license: same as -l').run()
      if check.stdout == check_l.stdout :
        check.will('PASS','--license same as -l.')
      else:
        check.will('FAIL','--license differs from -l.')
      check.done(suite)
    #
    check = testy.Exe([ exe,'-v0']).run()#, '-v0: verbosity 0 (no stdout)').run()
    if check.stdout == "":
      check.will('PASS','-v0: verbosity 0 has no stdout.')
    else:
      check.will('FAIL','-v0: contains something in stdout.')
    check.done(suite)
    #
    check = testy.Exe([ exe,'-v0','-d1']).run()
    if len(check.stderr) > 0:
      check.will('PASS','-v0 -d1: stderr contains text.')
      check.done(suite)
      if 'warn' in check.stderr.lower():
        check.will('PASS','-v0 -d1: stderr contains a warning.')
      else:
        check.will('FAIL','-v0 -d1: does NOT contain a warning.')
      check.done(suite)
    else:
      check.will('FAIL','-v0 -d1: nothing in stderr.')
      check.done(suite)
    #
    check = testy.Exe([ exe,'-v999']).run()
    if len(check.stderr) > 0:
      check.will('PASS','-v999: stderr contains text.')
      check.done(suite)
      if 'warn' in check.stderr.lower():
        check.will('PASS','-v999: stderr contains a warning.')
      else:
        check.will('FAIL','-v999: does NOT contain a warning.')
      check.done(suite)
    else:
      check.will('FAIL','-v999: nothing in stderr.')
      check.done(suite)
    #
    if False:#NOTE MPI requires zero for clean exit.
      # Handling unrecognized options
      check = testy.Exe([ exe,'-)']).run()#,
        #'-): unrecognized option returns nonzero').run()
      if check.retval != 0:
        check.will('PASS','-): unrecognized option returned '
          +str(check.retval)+' ('+check.retstr+').')
      else:
        check.will('FAIL','-): unrecognized option returned 0.')
      check.done(suite)
    #
    check = testy.Exe([ exe,'-^']).run()#,
      #'-^: unrecognized option error explained on stderr').run()
    if check.stderr == "":
      check.will('FAIL','-^: unrecognized option stderr is empty.')
    else:
      check.will('PASS','-^: stderr: '+check.stderr.partition('\n')[0]+'[...]')
    check.done(suite)
    #
    check = testy.Exe([ exe,"-\033"]).run()#,
    if check.stderr == "":
      check.will('FAIL','-\\033: unrecognized option stderr is empty.')
    else:
      check.will('PASS','-\\033: stderr: '+check.stderr.partition('\n')[0]+'[...]')
    check.done(suite)
    #
    check = testy.Exe([ exe,'-?']).run()#,
      #'-?: unrecognized option includes help text in stdout').run()
    if check_help.stderr in check.stderr:
      check.will('PASS','-?: unrecognized option stderr contains help text.')
    else:
      check.will('FAIL',
        '-?: unrecognized option stderr does NOT contain help text.')
    check.done(suite)
    #
    info_file = build_dir+'/build-data/build-info.txt'
    check_info = testy.Exe(['cat',info_file]).run()#,
      #'Build info contains Femera.').run()
    if 'Femera' in check_info.stdout :
      check_info.will('pass',info_file+': contains \"Femera\".')
    else:
      check_info.will('FAIL',info_file+': does NOT contain \"Femera\.')
    check_info.done(suite)
    #
    if 'mini' in check_info.stdout.lower() :
      check_info.will('pass',info_file+': contains \"mini\".')
    else:
      check_info.will('FAIL',info_file+': does NOT contain \"mini\".')
    check_info.done(suite)
    #
    check = testy.Exe(['cut','-c1-80',info_file]).run()#,
      #'Build info fits in 80 char width').run()
    if check.stdout == check_info.stdout:
      check.will('PASS','build info fits in 80 character width.')
    else:
      check.will('FAIL','build info does NOT fit in 80 character width.')
    check.done(suite)
    #
    check = testy.Exe([ exe,'-v1']).run()
    if check_info.stdout.splitlines()[0] in check.stdout:
      check.will('PASS','-v1: stdout contains build info line 1.')
    else:
      check.will('FAIL','-v1: stdout does NOT contain build info line 1.')
    check.done(suite)
    #
    check = testy.Exe([ exe,'-v1']).run()
    if check_info.stdout.splitlines()[1] in check.stdout:
      check.will('PASS','-v1: stdout contains build info line 2.')
    else:
      check.will('FAIL','-v1: stdout does NOT contain build info line 2.')
    check.done(suite)
    #
    suite.done()
  #
  return

if __name__ == "__main__":
  ''' Femera mini-app executable tests '''
  # Check myself for graffiti
  cg.check_graffiti( sys.argv[0] )
  #
  # Check the executable
  check_mini_exe()
