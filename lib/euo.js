var util = require('util')
  , events = require('events')
  , uodll = require('../build/Release/uodll')  
  , variables = require('./variables.js')
  , commands = require('./commands.js')

function UO() {
  this._handle = uodll.getHandle()
  this._monitors = {}
  this._previousValues = {}
}
util.inherits(UO, events.EventEmitter)

UO.prototype.open = function() {
  this.CliNr = 1
  return Boolean(this.CliNr)
}

UO.prototype.close = function() {
  if (this._handle) {
    this.stopMonitor('all')
    uodll.call(['Call', 'Close', this._handle])
  }
}

UO.prototype.monitor = function(type) {
  var that = this
  switch(type) {
    case 'journal':
      var prev = that.ScanJournal(0)
        that.jref = prev.ref
        that._monitors[type] = setInterval(function(){
          var curr = that.ScanJournal(that.jref)
          if (curr.ref !== that.jref) {
            while (curr.count > 0) {
              that.emit('journal', that.GetJournal(--curr.count))
            }
            that.jref = curr.ref
          }
        })
      break
    case 'all':
      variables.forEach(function(e, i, a) {
        that.monitor(e)
      })
      that.monitor('journal')
      break
    default:
      if(variables.indexOf(type) !== -1) {
        that._previousValues[type] = this[type]
        that._monitors[type] = setInterval(function() {
          var value = that[type]
          if (that._previousValues[type] !== value) {
            that.emit(type, value, that._previousValues[type])
            that._previousValues[type] = value
          }
        }, 50)
      }
      break
  }
}

UO.prototype.stopMonitor = function(type) {
  if(type === 'all') {
    for(i in this._monitors) {
      clearInterval(this._monitors[i])
    }
  } else if (this._monitors[type]) {
    clearInterval(this._monitors[type])
  }
}

variables.forEach(function(variable, i) {
  Object.defineProperty(UO.prototype, variable, {
    get: function() {
      var ret =  uodll.call(this._handle, ['Get', variable])
      return (ret && ret.length === 1) ? ret[0] : ret
    }
    , set: function(value) {
      return uodll.call(this._handle, ['Set', variable, value])
    }
  })
})

commands.forEach(function(command, i) {
  var func = function() {
    var args = ['Call', command]
    for (var i = 0; i < arguments.length; i++) {
      args.push(arguments[i])
    }

    var res = uodll.call(this._handle, args), result
    if (res && res.length === 1)
      return res

    switch(command) {
        case 'GetCont':
          result = {
              'name': res[0]
            , 'x': res[1]
            , 'y': res[2]
            , 'width': res[3]
            , 'height': res[4]
            , 'gumpkind': res[5]
            , 'id': res[6]
            , 'type': res[7]
            , 'hp': res[8]
          }
          break
        case 'GetItem':
          result = {
              'id': res[0]
            , 'type': res[1]
            , 'kind': res[2]
            , 'container_id': res[3]
            , 'x': res[4]
            , 'y': res[5]
            , 'z': res[6]
            , 'reputation': res[7]
            , 'color': res[8]
          }
          break
        case 'GetJournal':
          result = {
              'text': res[0]
            , 'color': res[1]
          }
          break
        case 'GetShop':
          result = {
              'result': res[0]
            , 'position': res[1]
            , 'count': res[2]
            , 'id': res[3]
            , 'type': res[4]
            , 'max': res[5]
            , 'price': res[6]
            , 'name': res[7]
          }
          break
        case 'GetSkill':
          result = {
              'adjusted': res[0]
            , 'real': res[1]
            , 'cap': res[2]
            , 'lock': res[3]
          }
          break
        case 'ScanJournal':
          result = {
              'ref': res[0]
            , 'count': res[1]
          }
          break
        default:
          result = res
          break
    }
    return result
  }
  Object.defineProperty(UO.prototype, command, { value: func })
})

exports.createClient = function(cliNr) {
  var client = new UO()
  if (cliNr) client.open(cliNr)
  return client
}