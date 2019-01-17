// Copyright IBM Corp. 2015,2016. All Rights Reserved.
// Node module: modern-syslog
// This file is licensed under the MIT License.
// License text available at https://opensource.org/licenses/MIT

'use strict';

var Writable = require('stream').Writable;
var core = require('./build/Release/core.node');
var inherits = require('util').inherits;
var fmt = require('util').format;

// Direct access to the core binding
exports.core = core;

// Constants
exports.option = core.option;
exports.facility = core.facility;
exports.level = core.level;

// High-level API, remove the redundant 'log' from method names.
exports.version = require('./package.json').version;
exports.log = log;
exports.upto = upto;
exports.curmask = curmask;
exports.setMask = setMask;
exports.Stream = Stream;

function log(ident, option, facility, level, msg, callback) {
  core.syslog(ident, option, toFacility(facility), toLevel(level), msg, callback);
}

// Low-level API
exports.setmask = core.setlogmask;
exports.toLevel = toLevel;
exports.toFacility = toFacility;
exports.logMask = logMask;
exports.logUpto = logUpto;

// Invert constants, so its easy to look the string up by the value.
// Expose keys globally, for backwards compatibility with node-syslog.
function expose(obj) {
  for (var key in obj) {
    var val = obj[key];
    exports[key] = val;
    obj[val] = key;
  }
}

expose(exports.option);
expose(exports.facility);
expose(exports.level);

// setlogmask() is too painful to use, most systems have a LOG_UPTO(level)
// macro, we'll just export upto() directly, its what most users will want.
function upto(level) {
  return core.setlogmask(logUpto(level));
}

// Linux allows calling setmask(0) to get the current mask, but OS X does not,
// so make a curmask() to smooth this over.

function curmask() {
  var cur = core.setlogmask(0);
  core.setlogmask(cur);
  return cur;
}

function setMask(level, upto) {
  var mask;

  if (upto)
    mask = logUpto(level);
  else
    mask = logMask(level);

  core.setlogmask(mask);
}


// Writable stream for syslog.

function Stream(ident, option, level, facility) {
  if (!(this instanceof Stream))
      return new Stream(level, facility);

  this.ident = ident;
  this.option = option;
  this.facility = toFacility(facility);
  this.level = toLevel(level);

  Writable.apply(this);
}

inherits(Stream, Writable);

Stream.prototype._write = function(chunk, encoding, callback) {
  core.syslog(this.ident, this.option, this.facility, this.level, chunk, callback);
};

// Low-level API

function toLevel(level) {
  if (typeof level === 'string' && level in core.level)
    return core.level[level];

  return level;
}

function toFacility(facility) {
  if (typeof facility === 'string' && facility in core.facility)
    return core.facility[facility];

  return facility;
}

function logMask(level) {
  return 1 << toLevel(level);
}

function logUpto(level) {
  return (1 << (toLevel(level) + 1)) - 1;
}
