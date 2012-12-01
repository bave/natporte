Copyright (c) 2012, Tomoya Inoue <inoue.tomoya@gmail.com>
All rights reserved.

Redistribution and use in all source and/or binary forms, with or without
modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  3. Neither the name of the project nor the
     names of its contributors may be used to endorse or promote products
     derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL the project BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


And, this software includes a certain number of redistribution source codes.
In part of structure.h has a rb-tree macro function which is a redistribution
source code. It must retain the following copyright notice and clauses.

/*
 * Copyright 2002 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

This program/software is Large Scale Network Address translation for verification test.

  - runnning under the certain operating systems are available to use divert socket

  - add kernel options
    - must
      - options IPDIVERT
      - options IPFIREWALL
    - either way..
      - options IPFIREWALL_FORWARD
      - optionsIPFIREWALL_DEFAULT_TO_ACCEPT
      - options IPFIREWALL_VERBOSE
      - options IPFIREWALL_VERBOSE_LIMIT=1000

  - use library/software
    - natporte
      - c/c++
      - libboost(later 1.48.0)
      - google-perftools for libtcmalloc(later 1.8.3)
      - compile options
        -DDEBUG
        -DWAIT_TIME_DELETE
        -DOPT_HOSTID
        -DIP_CHECKSUM
        -DDIFF_CKSUM

    - poolserver
      - nodejs
      - npm
        - npm install opts
        - npm install forever -g

  - using pool server
    Usage: node /usr/home/t-inoue/git/natporte/bin/poolserver [options]
    - show help command
        -h, --help
    - set udp session timeout [sec] (default:10)
        -u, --udptimeout <value>
    - set tcp session timeout [sec] (default:3600)
        -t, --tcptimeout <value>
    - set maximum connections (default:100)
        -m, --max <value>
    - set end port number (default:65534)
        -e, --end <value>
    - set start port number (default:4097)
        -s, --start <value>
    - show interval debug messages
        -i, --idebug
    - show debug messages
        -d, --debug
    - show natporte's pool server version
        -v, --version

  - using natporte
    - Usage: ./natporte -w ifname -l ifname                                       
        - w: Wan interface name   [must]                         
        - l: Lan interface name   [must]                         
        - a: Addrpool port number [default:(tcp)8668]            
        - d: Divert port number   [default:(divert)8668]         
        - r: send back to Reset when tcp session already limited 
        - b: ring Buffer size using by pool server communitacion 
        - n: force Negated the DF flag effect                    
        - h: adding Hostid (compile option)
        - v: Verbose mode (compile option)

