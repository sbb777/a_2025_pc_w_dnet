# encoding: utf-8
########################################################################################
# Copyright (c) Hilscher Gesellschaft fuer Systemautomation mbH. All Rights Reserved.
########################################################################################
# $Id: hilscher_windows.py 757 2023-01-20 09:01:46Z AMesser $:
#
# Provides helper code for windows operating system
########################################################################################
import waflib.Context
import waflib.Logs
import tempfile
import os
import sys

exec_cmd_orig = waflib.Context.Context.exec_command

# Windows has a limitation in command line length and we
# may need to put the command line in a file and call the
# executable with @file to pass the command line parameters
winver = tuple(sys.getwindowsversion()[0:2])

# This is Windows XP or Server 2003 (5.1, 5.2) or
# Vista / 7, which can handle command lines of 8191 chars
if winver > (5,0):
    max_commandline_len = 8191
# NT4, 2000 can handle a max command line length of 2047
else:
    max_commandline_len = 2047

# a similar function exists in waf itself, but it doesnt work
# properly in our use cases. Therefore it is overwritten herer
def exec_command(self, cmd, **kw):
    """Overwrites waf.Context.Context.exec_command to support long command lines"""
    global max_commandline_len, exec_cmd_orig

    native_path = None

    # compute length of commandline
    if isinstance(cmd,str):
        cmd = cmd.strip()
        cmd_length = len(cmd)
    else:
        cmd_length = len(" ".join(('"' + x + '"') for x in cmd))

    if cmd_length > max_commandline_len:
        waflib.Logs.debug('runner: longcmd_line: %r' % cmd)

        (fd, path) = tempfile.mkstemp('.lnk', dir=os.path.abspath(kw.get('cwd','.')), text=True)

        if isinstance(cmd,str):
            if cmd[0] == '"':
                cmd, dummy, params = cmd[1:].partition('"')
                cmd    = '"' + cmd
            else:
                cmd, dummy, params = cmd.partition(' ')

            params = params.strip()
        else:
            # process all parameters in command string:
            args = (x.decode() for x in cmd[1:])
            # escape backslashes
            args = (x.replace('\\','\\\\') for x in args)
            # escape quotation marks
            args = (x.replace('"','\\"') for x in args)

            args = tuple(args)

            params = ('"' + '" "'.join(args) + '"')
            cmd = '"' + cmd[0] + '"'

        waflib.Logs.debug('runner: longcmd_cmd: %r' % cmd)
        waflib.Logs.debug('runner: longcmd_params: %r' % params)

        os.write(fd, params + "\n")
        os.close(fd)

        native_path = os.path.basename(path)
        cmd = cmd + " @" + native_path
    else:
        waflib.Logs.debug('runner: cmd_line: %r' % cmd)

    try:
        retval = exec_cmd_orig.__get__(self, self.__class__)(cmd, **kw)
    finally:
        if native_path:
            os.unlink(os.path.abspath(path))

    return retval

waflib.Context.Context.exec_command = exec_command

def options(conf):
    pass
