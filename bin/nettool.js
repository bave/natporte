#!/usr/bin/env node

module.exports = {

    chk_ack: function(flags)
    {
        if (flags & 0x10) return true;
        return false;
    },

    chk_syn: function(flags)
    {
        if (flags & 0x02) return true;
        return false;
    },

    chk_fin: function(flags)
    {
        if (flags & 0x01) return true;
        return false;
    },

    print_flags: function(flags)
    {
        retval = ""
        if (flags & 0x01) retval += "FIN.";
        if (flags & 0x02) retval += "SYN.";
        if (flags & 0x04) retval += "PST.";
        if (flags & 0x08) retval += "PSH.";
        if (flags & 0x10) retval += "ACK.";
        if (flags & 0x20) retval += "URG.";
        return retval;
    },

    inet_ntoa: function(n)
    {
        var octets = [];
        for (var i = 3; i >= 0; i--, n >>>= 8) {
            octets[i] = n & 0xff;
        }
        return octets.join('.');

    },

    inet_aton: function(a)
    {
        var octets = a.split(/\./);
        while (octets.length < 4) {
            octets.splice(1, 0, 0);
        }
        var n = 0;
        for (var i = 0; i<4; i++) {
            n = (n*256) + parseInt(octets[i]);
        }
        return n;
    },

    slash2mask: function(n)
    {
        return this.inet_ntoa(n ? 0xffffffff-((1<<(32-n))-1) : 0);
    },

    mask2slash: function(a)
    {
        var n = this.inet_aton(a);
        var m = 32;
        for(;m && !(n & 1); m--) n >>>= 1;
        return m;
    },

    netrange: function(addr, mask)
    {
        var na = this.inet_aton(addr);
        var nm = this.inet_aton(typeof(mask) == 'number' ? this.slash2mask(mask) : mask);
        var start = na & nm;
        var mx    = ~nm;
        return [this.inet_ntoa(start), this.inet_ntoa(start+mx)];
    },

    isIPv4: function(addr)
    {
        if (typeof(addr) == 'string') {
            re = new RegExp(/^(\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(\d|[01]?\d\d|2[0-4]\d|25[0-5])$/);
            if (addr.match(re)) {
                return true;
            }else{
                return false;
            }
        } else if (typeof(addr) == 'number') {
            var addr_string = this.inet_ntoa(addr);
            re = new RegExp(/^(\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(\d|[01]?\d\d|2[0-4]\d|25[0-5])\.(\d|[01]?\d\d|2[0-4]\d|25[0-5])$/);
            if (addr_string.match(re)) {
                return true;
            }else{
                return false;
            }
        }
    },

    isTCP: function(protocol)
    {
        var tcp_number  = 6;

        if (typeof(protocol) == 'number') {
            if (protocol == tcp_number) {
                return true;
            }
        }
            
        if (typeof(protocol) == 'string') {
            if (protocol == 'TCP' || protocol == 'tcp') {
                return true;
            }
        }
        return false;
    },

    isUDP: function(protocol)
    {
        var udp_number = 17;

        if (typeof(protocol) == 'number') {
            if (protocol == udp_number) {
                return true;
            }
        }
            
        if (typeof(protocol) == 'string') {
            if (protocol == 'UDP' || protocol == 'udp') {
                return true;
            }
        }
        return false;
    },

    check_AddrPool: function(pool_addr)
    {
        var result_pool_addr = [];
        if (typeof(pool_addr) == 'string') {
            if (this.isIPv4(pool_addr)) {
                //console.log("string_ipv4");
                result_pool_addr.push(pool_addr)
            } else {
                //console.log("not string_ipv4");
                ;
            }
        } else if (typeof(pool_addr) == 'number') {
            var addr = this.inet_ntoa(pool_addr);
            if (this.isIPv4(addr)) {
                //console.log("number_ipv4");
                result_pool_addr.push(addr)
            } else {
                //console.log("not number_ipv4");
                ;
            }
        } else if (typeof(pool_addr) == 'object') {
            for (var i=0;i<pool_addr.length; i++) {
                var per_pool_addr = pool_addr[i];
                if (typeof(per_pool_addr) == 'string') {
                    if (this.isIPv4(per_pool_addr)) {
                        //console.log("string_ipv4");
                        result_pool_addr.push(per_pool_addr)
                    } else {
                        //console.log("not string_ipv4");
                        ;
                    }
                } else if (typeof(per_pool_addr) == 'number') {
                    var addr = this.inet_ntoa(per_pool_addr);
                    if (this.isIPv4(addr)) {
                        //console.log("number_ipv4");
                        result_pool_addr.push(addr)
                    } else {
                        //console.log("not number_ipv4");
                        ;
                    }
                }
            }
        }
        return result_pool_addr;
    }

    /*
    getIPaddress: function()
    {
        var ignoreRE = /^(127\.0\.0\.1|::1|fe80(:1)?::1(%.*)?)$/i;

        var exec = require('child_process').exec;
        var cached;
        var command;
        var filterRE;

        switch (process.platform) {
        case 'win32':
        //case 'win64': // TODO: test
            command = 'ipconfig';
            filterRE = /\bIP(v[46])?-?[^:\r\n]+:\s*([^\s]+)/g;
            // TODO: find IPv6 RegEx
            break;
        case 'darwin':
            command = 'ifconfig';
            filterRE = /\binet\s+([^\s]+)/g;
            // filterRE = /\binet6\s+([^\s]+)/g; // IPv6
            break;
        default:
            command = 'ifconfig';
            filterRE = /\binet\b[^:]+:\s*([^\s]+)/g;
            // filterRE = /\binet6[^:]+:\s*([^\s]+)/g; // IPv6
            break;
        }

        return function (callback, bypassCache) {
            if (cached && !bypassCache) {
                callback(null, cached);
                return;
            }
            // system call
            exec(command, function (error, stdout, sterr) {
                cached = [];
                var ip;
                var matches = stdout.match(filterRE) || [];
                //if (!error) {
                for (var i = 0; i < matches.length; i++) {
                    ip = matches[i].replace(filterRE, '$1')
                    if (!ignoreRE.test(ip)) {
                        cached.push(ip);
                    }
                }
                //}
                callback(error, cached);
            });
        };
    }
    */
};

/*
var addr = '3232238082';
var addr_str = nettool.inet_ntoa(addr);
console.log(addr_str);
var addr2 = nettool.inet_aton(addr_str);
console.log(addr2);

// IP header parse sample
var type   = data[0] >> 4;
var iphl   = data[0] & 0x0F;
var tos    = data[1];
var len    = data.readUInt16BE(2);
var ttl    = data[8];
var proto  = data[9];
var chksum = data.readUInt16BE(10);
var srcIP  = nettool.inet_ntoa(data.readUInt32BE(12));
var dstIP  = nettool.inet_ntoa(data.readUInt32BE(16));
console.log(type);
console.log(iphl);
console.log(tos);
console.log(len);
console.log(ttl);
console.log(proto);
console.log(chksum);
console.log(srcIP);
console.log(dstIP);

// TCP header parse sample
var srcPort = data.readUInt16BE(20);
var dstPort = data.readUInt16BE(22);
var flags   = data.readUInt16BE(34);
console.log(srcPort);
console.log(dstPort);
console.log(nettool.print_flags(flags));
*/

