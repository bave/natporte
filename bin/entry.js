#!/usr/bin/env node

// entry management class which is used by per protocol.

// constructer
function Entry(protocol, wan_addrs, startPort, endPort, max_connections)
{
    this.startPort = startPort;
    this.endPort = endPort;
    this.maxConnection = max_connections;
    this.protocol = protocol;
    this.pool_addrs = [];
    this.db = {};
    this.pool_ports = {};

    this.tool = require('./nettool.js');

    this.addPoolAddresses(wan_addrs);


    /*
    var nettool = require('./nettool.js');
    this.pool_addrs = nettool.check_AddrPool(pool_addrs);
    */

    /*
    if (process.platform != "freebsd") {
        var interfaces = require('os').networkInterfaces();
        var wan_string = "interfaces"+"."+wanIF;
        var wan_info   = eval(wan_string);
        if (wan_info != undefined) {
            for (var i=0; i<wan_info.length; i++) {
                if (wan_info[i]["family"] == "IPv4") {
                    this.pool_addrs.push(wan_info[i]["address"]);
                }
            }
        }
    } else {
        var interfaces = require('./node_modules/Release/netutils_js').networkInterfaces();
        var wan_string = "interfaces"+"."+wanIF;
        var wan_info   = eval(wan_string);
        if (wan_info != undefined) {
            for (var i=0; i<wan_info.length; i++) {
                if (wan_info[i]["family"] == "AF_INET") {
                    this.pool_addrs.push(wan_info[i]["address"]);
                }
            }
        }
    }

    for (var i=0; i<this.pool_addrs.length; i++) {
        this.pool_ports[this.pool_addrs[i]] = [];
        for (var j=startPort; j<=endPort; j++)
        {
            this.pool_ports[this.pool_addrs[i]].push(j);
        }
    }
    */
    //console.log(this.pool_ports);

};

Entry.prototype.delDBEntoryByPoolAddr = function(addrs)
{
    for (var i in this.db) {
        for (var j=0; j<this.db[i].length; j++) {
            if (this.db[i][j]['wan_ip'] == addrs) { 
                this.db[i].splice(j, 1);
                j--;
            }
        }
    }
};

Entry.prototype.delPoolAddresses = function(addrs)
{
    if (typeof(addrs) == 'string') {

        if (this.tool.isIPv4(addrs) == false) {
            return undefined;
        }

        var index = this.pool_addrs.indexOf(addrs);
        if (index != -1) {
            this.pool_addrs.splice(index, 1);
            delete this.pool_ports[addrs];
            this.delDBEntoryByPoolAddr(addrs);
        }


    } else {

        for (var i=0; i<addrs.length; i++) {

            if (this.tool.isIPv4(addrs[i]) == false) {
                continue;
            }

            var index = this.pool_addrs.indexOf(addrs[i]);
            if (index != -1) {
                this.pool_addrs.splice(index, 1);
                delete this.pool_ports[addrs[i]];
                this.delDBEntoryByPoolAddr(addrs[i]);
            }

        }

    }
    return undefined;
};

Entry.prototype.addPoolAddresses = function(wan_addrs)
{
    if (typeof(wan_addrs) == 'string') {

        if (this.tool.isIPv4(wan_addrs) == false) {
            return undefined;
        }

        var ret = false;
        for (var j=0; j<this.pool_addrs.length; j++) {
            if (wan_addrs == this.pool_addrs[j]) {
                ret = true;
                break;
            }
        }
        if (ret == true) {
            return undefined;
        }

        this.pool_addrs.push(wan_addrs);
        this.pool_ports[wan_addrs] = [];
        for (var j=this.startPort; j<=this.endPort; j++)
        {
            this.pool_ports[wan_addrs].push(j);
        }

    } else {

        for (var i=0; i<wan_addrs.length; i++) {

            if (this.tool.isIPv4(wan_addrs[i]) == false) {
                continue;
            }

            var ret = false;
            for (var j=0; j<this.pool_addrs.length; j++) {
                if (wan_addrs[i] == this.pool_addrs[j]) {
                    ret = true;
                    break;
                }
            }
            if (ret == true) {
                continue;
            }

            this.pool_addrs.push(wan_addrs[i]);
            this.pool_ports[wan_addrs[i]] = [];
            for (var j=this.startPort; j<=this.endPort; j++)
            {
                this.pool_ports[wan_addrs[i]].push(j);
            }

        }

    }
    return;
}


Entry.prototype.clone = function(obj)
{
    return JSON.parse(JSON.stringify(obj));
};

Entry.prototype.printPoolAddrs = function()
{
    console.log(this.pool_addrs);
    return this.pool_addrs;
};

Entry.prototype.printPoolPorts = function()
{
    console.log(this.pool_ports);
    return this.pool_ports;
};

Entry.prototype.printDB = function()
{
    console.log(this.db);
    return this.db;
};

Entry.prototype.countDB = function()
{
    var count = 0;
    for (var i in this.db) {
        if (this.db.hasOwnProperty(i)) {
            count += this.db[i].length;
        }
    }
    return count;
};

Entry.prototype.sameEntryCheck = function(lan_ip, lan_port)
{
    if (this.db.hasOwnProperty(lan_ip)) {
        entry_list = this.db[lan_ip];
        //console.log(entry_list.length);
        //console.log(this.db);
        for (var i=0; i<entry_list.length; i++) {
            if (entry_list[i]['lan_ip'] == lan_ip && entry_list[i]['lan_port'] == lan_port)
            {
                return entry_list[i];
            }
        }
    } 
    return undefined;
};

Entry.prototype.assignConnection = function(lan_ip, lan_port)
{
    var entry;
    var wan_ip;
    var wan_port;

    if (this.tool.isIPv4(lan_ip)) {
        entry = this.sameEntryCheck(lan_ip, lan_port);
        if (entry != undefined) {
            //console.log("return exist entry");
            //console.log(entry);
            return this.clone(entry);
        }

        wan_ip = this.assignWanIP(lan_ip);
        wan_port = this.assignWanPort(wan_ip);

        if (wan_port == undefined) {
            return undefined;
        }

        if (typeof(lan_port) == 'number') {
        } else {
            lan_port = parseInt(lan_port);
        }

    } else {
        return undefined;
    }

    var result = {};
    if (this.db.hasOwnProperty(lan_ip)) {
        // exist client
        if (this.db[lan_ip].length < this.maxConnection) {
            // add connection table
            result['wan_ip']   = wan_ip; 
            result['wan_port'] = wan_port;
            result['lan_ip']   = lan_ip
            result['lan_port'] = lan_port;
            this.db[lan_ip].push(result);
        } else {
            // max connection over
            return undefined;
        }
    } else {
        // add new client
        this.db[lan_ip] = [];
        result['wan_ip']   = wan_ip; 
        result['wan_port'] = wan_port;
        result['lan_ip']   = lan_ip
        result['lan_port'] = lan_port;
        this.db[lan_ip].push(result);
    }

    return this.clone(result);
};

/*
Entry.prototype.assignConnection = function(lan_ip, lan_port)
{
    //失敗作
    var wan_ip;
    var wan_port;

    if (this.tool.isIPv4(lan_ip)) {
        var wan_ip   = this.assignWanIP(lan_ip);
        var wan_port = this.assignWanPort();
    } else {
        return undefined;
    }

    if (wan_port == undefined) {
        return undefined;
    }

    if (typeof(lan_port) == 'number') {
    } else {
        lan_port = parseInt(lan_port);
    }

    var result = {};
    if (this.db.hasOwnProperty(lan_ip)) {
        // exist client
        console.log(this.db[lan_ip].length);
        if (this.db[lan_ip].length < this.maxConnection) {
            // add connection table
            result['wan_ip']   = wan_ip; 
            result['wan_port'] = wan_port;
            result['lan_ip']   = lan_ip
            result['lan_port'] = lan_port;

            //console.log("sameCheck");
            //console.log(this.sameEntryCheck(result));

            if (this.sameEntryCheck(result) == false) {
                this.db[lan_ip].push(result);
            } else {
            }
        } else {
            // max connection over
            return undefined;
        }
    } else {
        // add new client
        this.db[lan_ip] = [];
        result['wan_ip']   = wan_ip; 
        result['wan_port'] = wan_port;
        result['lan_ip']   = lan_ip
        result['lan_port'] = lan_port;
        this.db[lan_ip].push(result);
    }
    return result;
};
*/

Entry.prototype.releaseConnection = function(lan_ip, lan_port, wan_ip, wan_port)
{
    if (this.tool.isIPv4(lan_ip)) {
        ;
    } else {
        return false;
    }

    if (this.tool.isIPv4(wan_ip)) {
        ;
    } else {
        return false;
    }

    if (typeof(lan_port) == 'number') {
        ;
    } else {
        lan_port = parseInt(lan_port);
    }

    if (typeof(wan_port) == 'number') {
        ;
    } else {
        lan_port = parseInt(lan_port);
    }

    table = this.db[lan_ip];
    for (var i=0; i<table.length; i++) {
        var entry = table[i];
        var table_lan_ip   = entry['lan_ip'];
        var table_lan_port = entry['lan_port'];
        var table_wan_ip   = entry['wan_ip'];
        var table_wan_port = entry['wan_port'];

        if (table_lan_ip == lan_ip) {
            if (table_wan_ip == wan_ip) {
                if (table_lan_port == lan_port) {
                    if (table_wan_port == wan_port) {
                        // delete connection table ----
                        table.splice(i, 1);
                        this.releaseWanPort(wan_ip, wan_port);
                        // ----------------------------
                        return true;
                    }
                }
            }
        }
    }

    return false;
};

Entry.prototype.assignWanIP = function(ip_src)
{
    var result = undefined;
    var mod = this.pool_addrs.length;
    if (this.tool.isIPv4(ip_src)) {
        var segment4 = parseInt(ip_src.split(".")[3]);
        var wan_index = segment4 % mod;
        result = this.pool_addrs[wan_index];
    }
    return result;
};

Entry.prototype.assignWanPort = function(ip)
{
    if (this.pool_ports[ip] == undefined) {
        return undefined;
    }
    return this.pool_ports[ip].shift();
};

Entry.prototype.releaseWanPort = function(ip, port)
{
    this.pool_ports[ip].push(port);
    return undefined;
};

module.exports = Entry;

// test code ----------------------------------------------------------
/*
var entry = new Entry("TCP", [], 1, 20, 10);

entry.addPoolAddresses('192.168.0.1');
entry.addPoolAddresses('192.168.0.2');
entry.addPoolAddresses('192.168.0.3');
entry.addPoolAddresses('192.168.0.4');
entry.addPoolAddresses('192.168.0.5');
entry.delPoolAddresses('192.168.0.1');
entry.delPoolAddresses(['192.168.0.2', '192.168.0.3', '192.168.0.4']);
result = entry.assignConnection("10.0.0.1", 1000);
result = entry.assignConnection("10.0.0.2", 1000);
result = entry.assignConnection("10.0.0.1", 1001);
result = entry.assignConnection("10.0.0.2", 1001);
//entry.delPoolAddresses('192.168.0.2');
//entry.delPoolAddresses('192.168.0.3');
entry.printPoolAddrs();
entry.printPoolPorts();
entry.printDB();
result = entry.releaseConnection(result['lan_ip'], result['lan_port'], result['wan_ip'], result['wan_port']);
entry.printPoolAddrs();
entry.printPoolPorts();
entry.printDB();
*/

/*
console.log("count:"+entry.countDB());
entry.printDB();
console.log("count:"+entry.countDB());
*/
// test code ----------------------------------------------------------
