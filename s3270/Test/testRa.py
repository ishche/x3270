#!/usr/bin/env python3
#
# Copyright (c) 2021-2024 Paul Mattes.
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
# s3270 RA tests

import unittest
from subprocess import Popen, PIPE, DEVNULL
import requests
import Common.Test.cti as cti
import Common.Test.playback as playback


def get_cursor(port: int):
    rs = requests.get(f'http://127.0.0.1:{port}/3270/rest/json/Query(cursor1)').json()['result'][0].split()
    return (int(rs[1]), int(rs[3]))

def kw_split(result):
    ret = {}
    for line in result:
        kv = line.split(': ')
        ret[kv[0]] = kv[1]
    return ret

def ebcdic(s: str) -> bytes:
    '''Convert ASCII to EBCDIC'''
    dd = Popen(['dd', 'conv=ebcdic'], stdin=PIPE, stdout=PIPE, stderr=DEVNULL)
    return dd.communicate(s.encode())[0]

class TestS3270Ra(cti.cti):

    # s3270 RA test
    def test_s3270_ra(self):

        pport, socket = cti.unused_port()
        with playback.playback(self, 's3270/Test/ra_test.trc', pport) as p:
            socket.close()

            # Start s3270.
            sport, socket = cti.unused_port()
            s3270 = Popen(cti.vgwrap(['s3270', '-httpd', str(sport),
                    f'c:127.0.0.1:{pport}']), stdin=DEVNULL, stdout=DEVNULL)
            self.children.append(s3270)
            self.check_listen(sport)
            socket.close()

            # Fill in the screen.
            p.send_records(1)

            # Check the field we land on.
            rs = kw_split(requests.get(f'http://127.0.0.1:{sport}/3270/rest/json/ReadBuffer(field)').json()['result'])
            c = rs['Contents'].split()
            self.assertEqual('SF(c0=e0)', c[0])
            self.assertEqual('SA(45=f2)', c[1]) # this is the critical bit
            self.assertEqual(['30' for i in range(0, 8)], c[2:])

        # Wait for the processes to exit.
        requests.get(f'http://127.0.0.1:{sport}/3270/rest/json/Quit()')
        self.vgwait(s3270)

if __name__ == '__main__':
    unittest.main()
