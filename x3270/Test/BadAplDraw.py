#!/usr/bin/env python3
#
# Copyright (c) 2021-2022 Paul Mattes.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the names of Paul Mattes nor the names of his contributors
#       may be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY PAUL MATTES "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
# EVENT SHALL PAUL MATTES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# x3270 smoke tests

import unittest
from subprocess import Popen, PIPE, DEVNULL
import subprocess
import os
import stat
import tempfile
import filecmp
import TestCommon

class TestX3270BadAplDraw(unittest.TestCase):

    # x3270 bad APL draw test.
    # There was a bug that caused '-' characters in NVT mode to be drawn
    # incorrectly using anything but the 3270 font.
    def test_x3270_bad_apl_draw(self):

        # Start a tightvnc server.
        # The password file needs to be 0600 or Vnc will prompt for it again.
        os.chmod('x3270/Test/vnc/.vnc/passwd', stat.S_IREAD | stat.S_IWRITE)
        cwd=os.getcwd()
        os.environ['HOME'] = cwd + '/x3270/Test/vnc'
        os.environ['USER'] = 'foo'
        self.assertEqual(0, os.system('tightvncserver :2 2>/dev/null'))
        TestCommon.check_listen(5902)

        # Start 'playback' to read x3270's output.
        playback = Popen(["playback", "-w", "-p", "9961",
            "x3270/Test/badapl.trc"], stdin=PIPE, stdout=DEVNULL)
        TestCommon.check_listen(9961)

        # Start x3270.
        if 'DISPLAY' in os.environ:
            d = os.environ['DISPLAY']
        else:
            d = None
        os.environ['DISPLAY'] = ':2'
        x3270 = Popen(['x3270', '-efont', 'fixed',
            '-xrm', f'x3270.connectFileName: {os.getcwd()}/x3270/Test/vnc/.x3270connect',
            '-httpd', '127.0.0.1:9960',
            '127.0.0.1:9961'], stdout=DEVNULL)
        TestCommon.check_listen(9960)

        # Feed x3270 some data.
        playback.stdin.write(b's\ns\ns\ns\ns\n')
        playback.stdin.flush()
        # This needs to happen differently.
        # TestCommon.check_push(playback, 9960, 1)

        # Find x3270's window ID.
        widcmd = subprocess.run('xlsclients -l', shell=True,
                capture_output=True, check=True)
        for i in widcmd.stdout.decode('utf8').replace(' ', '').split('\n'):
            if i.startswith('Window'):
                wid=i.replace('Window','').replace(':', '')
            elif i.startswith('Command') and 'x3270' in i:
                break

        # Dump the window contents.
        (handle, name) = tempfile.mkstemp()
        os.close(handle)
        self.assertEqual(0, os.system(f'xwd -id {wid} -out "{name}"'))

        # Wait for the processes to exit.
        playback.stdin.close()
        playback.kill()
        playback.wait(timeout=2)
        x3270.kill()
        x3270.wait(timeout=2)
        self.assertEqual(0, os.system('tightvncserver -kill :2 2>/dev/null'))

        # Make sure the image is correct.
        self.assertTrue(filecmp.cmp(name, 'x3270/Test/badapl.xwd'))
        os.unlink(name)

        # Restore DISPLAY.
        if d != None:
            os.environ['DISPLAY'] = d

if __name__ == '__main__':
    unittest.main()
