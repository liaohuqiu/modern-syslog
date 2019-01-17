// Copyright IBM Corp. 2015,2016. All Rights Reserved.
// Node module: modern-syslog
// This file is licensed under the MIT License.
// License text available at https://opensource.org/licenses/MIT

var fmt = require('util').format;
var syslog = require('../');
var tap = require('tap');

function accept(m) {
  tap.test(fmt('core syslog accepts %j', m), function(t) {
    t.plan(1);
    syslog.core.syslog('test', syslog.core.option.LOG_PID, syslog.core.facility.LOG_LOCAL0, syslog.core.level.LOG_DEBUG, m, function() {
      t.assert(true, 'called back');
    });
  });
}

accept('string');
accept(Buffer('buffer'));
accept(undefined);
accept(null);
accept({some: 5});
accept(function fn() {});
