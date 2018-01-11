#!/usr/bin/env python3
# Simple Python version of x3270if

"""Python interface to x3270 emulators"""

import io
import os
import socket
import sys
import subprocess
import time

_backslashChars = '\\"'
_quoteChars = _backslashChars + ' ,()'

def Quote(arg):
    """Quote an argument in action syntax

       Args:
          arg (str): Argument to format
       Returns:
          str: Formatted argument
    """
    # Backslashes and double quotes need backslashes in front of them.
    # Other syntax markers (space, comma, paren) just need the argument in
    #  double quotes.
    if (not any(ch in arg for ch in _quoteChars)): return arg
    return '"' + ''.join('\\' + ch if ch in _backslashChars else ch for ch in arg) + '"'

class ActionFailException(Exception):
    """x3270if action failure"""
    def __init__(self,msg):
        Exception.__init__(self,msg)

class StartupException(RuntimeError):
    """x3270if unable to start s3270"""
    def __init__(self,msg):
        RuntimeError.__init__(self,msg)

class _x3270if():
    """Abstract x3270if base class"""
    def __init__(self,debug=False):
        """Initialize an instance

           Args:
              debug (bool): True to trace debug info to stderr.
       """

        # Debug flag
        self._debug = debug

        # Last prompt
        self._prompt = ''

        # File streams to/from the emulator
        self._to3270 = None
        self._from3270 = None

    def __del__(self):
        self.Debug('_x3270if deleted')

    @property
    def prompt(self):
        """Gets the last emulator prompt
           str: Last emulator prompt

        """
        return self._prompt

    def Run(self,cmd,*args):
        """Send an action to the emulator

           Args:
              cmd (str): Action name
                 If 'args' is omitted, this is the entire command and the text
                 will be passed through unmodified.
              args (iterable): Arguments
           Returns:
              str: Command output
                 Mulitiple lines are separated by newline characters.
           Raises:
              x3270if.ActionFailException: Action failed.
              EOFException: Emulator exited unexpectedly.
        """
        if (not isinstance(cmd, str)):
            raise Exception("First argument must be a string")
        self.Debug("args is {0}, len is {1}".format(args, len(args)))
        if (args == ()):
            argstr = cmd
        elif (len(args) == 1 and not isinstance(args[0], str)):
            # One argument that can be iterated over.
            argstr = cmd + '(' + ','.join(Quote(arg) for arg in args[0]) + ')'
        else:
            # Multiple arguments.
            argstr = cmd + '(' + ','.join(Quote(arg) for arg in args) + ')'
        self._to3270.write(argstr + '\n')
        self._to3270.flush()
        self.Debug('Sent ' + argstr)
        result = ''
        prev = ''
        while (True):
            text = self._from3270.readline().rstrip('\n')
            if (text == ''): raise EOFError('Emulator exited')
            self.Debug("Got '" + text + "'")
            if (text == 'ok'):
                self._prompt = prev
                break
            if (text == 'error'):
                self._prompt = prev
                raise ActionFailException(result)
            if (result == ''): result = prev.lstrip('data: ')
            else: result = result + '\n' + prev.lstrip('data: ')
            prev = text
        return result

    def Debug(self,text):
        """Debug output

           Args:
              text (str): Text to log. A Newline will be added.
        """
        if (self._debug): sys.stderr.write('[33m' + text + '[0m\n')

class WorkerConnection(_x3270if):
    """Connection to the emulator from a worker script invoked via the Script() action"""
    def __init__(self,debug=False):
        """Initialize the object.

           Args:
              debug (bool): True to log debug information to stderr.

           Raises:
              StartupException: Insufficient information in the environment to
              connect to the emulator.
        """
        _x3270if.__init__(self, debug)
        self._socket = None
        self._infd = -1
        self._outfd = -1

        # Socket or pipes
        port = os.getenv('X3270PORT')
        if (port != None):
            # Connect to a TCP port.
            self._socket = socket.create_connection(['127.0.0.1',int(port)])
            self._to3270 = self._socket.makefile('w', encoding='utf-8')
            self._from3270 = self._socket.makefile('r', encoding='utf-8')
            self.Debug('Connected')
            emulatorEncoding = self.Run('Query(LocalEncoding)')
            if (emulatorEncoding != 'UTF-8'):
                self._to3270 = self._socket.makefile('w',
			encoding=emulatorEncoding)
                self._from3270 = self._socket.makefile('r',
			encoding=emulatorEncoding)
        else:
            # Talk to pipe file descriptors.
            infd = os.getenv('X3270INPUT')
            outfd = os.getenv('X3270OUTPUT')
            if (infd == None or outfd == None):
                raise StartupException("No X3270PORT, X3270INPUT or X3270OUTPUT defined")
            self._infd = int(infd)
            self._to3270 = io.open(self._infd, 'wt', encoding='utf-8',
                    closefd=False)
            self._outfd = int(outfd)
            self._from3270 = io.open(self._outfd, 'rt', encoding='utf-8',
                    closefd=False)
            self.Debug('Pipes connected')
            emulatorEncoding = self.Run('Query(LocalEncoding)')
            if (emulatorEncoding != 'UTF-8'):
                self._to3270 = io.open(self._infd, 'wt',
                        encoding=emulatorEncoding, closefd=False)
                self._from3270 = io.open(self._outfd, 'rt',
                        encoding=emulatorEncoding, closefd=False)

    def __del__(self):
        if (self._socket != None): self._socket.close();
        if (self._infd != -1): os.close(self._infd)
        if (self._outfd != -1): os.close(self._outfd)
        _x3270if.__del__(self)
        self.Debug('WorkerConnection deleted')

class NewEmulator(_x3270if):
    """Starts a new copy of s3270"""
    def __init__(self,debug=False,extra_args=[]):
        """Initialize the object.

           Args:
              debug (bool): True to log debug information to stderr.
              extra_args(list of str, optional): Extra arguments
                 to pass in the s3270 command line.
           Raises:
              StartupException: Unable to start s3270.
        """
        _x3270if.__init__(self, debug)
        self._socket = None
        self._s3270 = None

        # Create a temporary socket to find a unique local port.
        tempsocket = socket.socket()
        tempsocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        tempsocket.bind(('127.0.0.1', 0))
        port = tempsocket.getsockname()[1]
        self.Debug('Port is {0}'.format(port))

        # Create the child process.
        try:
            args = ['s3270' if os.name != 'nt' else 'ws3270.exe',
                    '-utf8',
                    '-minversion', '3.6',
                    '-scriptport', str(port),
                    '-scriptportonce'] + extra_args
            oserr = None
            try:
                self._s3270 = subprocess.Popen(args,
                        stderr=subprocess.PIPE,universal_newlines=True)
            except OSError as err:
                oserr = str(err)

            if (oserr != None): raise StartupException(oserr)

            # It might take a couple of tries to connect, as it takes time to
            # start the process. We wait a maximum of half a second.
            tries = 0
            connected = False
            while (tries < 5):
                try:
                    self._socket = socket.create_connection(['127.0.0.1', port])
                    connected = True
                    break
                except:
                    time.sleep(0.1)
                    tries += 1
            if (not connected):
                errmsg = 'Could not connect to emulator'
                self._s3270.terminate()
                r = self._s3270.stderr.readline().rstrip('\r\n')
                if (r != ''): errmsg += ': ' + r
                raise StartupException(errmsg)

            self._to3270 = self._socket.makefile('w', encoding='utf-8')
            self._from3270 = self._socket.makefile('r', encoding='utf-8')
            self.Debug('Connected')
        finally:
            del tempsocket

    def __del__(self):
        if (self._s3270 != None): self._s3270.terminate()
        if (self._socket != None): self._socket.close();
        _x3270if.__del__(self)
        self.Debug('NewEmulator deleted')
