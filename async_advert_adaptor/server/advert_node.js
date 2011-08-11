// 
//  advert_node.js
//  SAGA
//  
//  Created by Hans Christian Wilhelm on 2011-08-04.
//  Copyright 2011. All rights reserved.
// 

// ========================
// = include EventEmitter =
// ========================

var eventEmitter  = require('events').EventEmitter;

// ========================================
// = AdvertNode inherit from EventEmitter =
// ========================================

function AdvertNode (nodeName, isDir, parentNode)
{
  this.nodeName               = nodeName;
  this.isDir                  = isDir;
  this.attributeArray         = [];
  this.dataArray              = [];
  this.nodeArray              = [];
  this.parentNode             = parentNode;
  
  
  eventEmitter.call(this);
  
  this.addNode = function (advertNode)
  {
    this.nodeArray.push(advertNode);
    this.emit('change', this);
  }
  
  this.removeNode = function (node)
  {
    var i = this.nodeArray.indexOf(node);
    delete this.nodeArray[i];
    
    this.emit('change', this);
  }
  
  this.deleteNode = function ()
  {
    for (var i in this.nodeArray)
    {
      this.nodeArray[i].deleteNode();
    }
    
    this.attributeArray         = [];
    this.dataArray              = [];
    this.nodeArray              = [];
    this.parentNode             = undefined;  
    
    this.emit('change', {});
    this.removeAllListeners('change');
  }
  
  this.addAttribute = function (advertAttribute)
  {
    this.attributeArray.push(advertAttribute);
    this.emit('change', this);
  }
  
  this.removeAttribute = function (advertAttribute)
  {
    var i = this.attributeArray.indexOf(advertAttribute);
    
    if (i != -1)
    {
      this.attributeArray.splice(i, 1);
      this.emit('change', this);
    }
  }
  
  this.toJSON = function ()
  {
    var value = {};
    
    value.nodeName        = this.nodeName;
    value.isDir           = this.isDir;
    value.attributeArray  = this.attributeArray;
    value.dataArray       = this.dataArray;
    value.nodeArray       = [];
    
    for (var i in this.nodeArray)
    {
      value.nodeArray.push(this.nodeArray[i].nodeName);
    }
    
    return value;
  }
  
  this.exists = function (nodeName)
  {
    var result = undefined;
    
    for (var i in this.nodeArray)
    {
      if (this.nodeArray[i].nodeName == nodeName)
      {
        result = this.nodeArray[i];
      }
    }
   
    return result;
  }
}

AdvertNode.prototype.__proto__ = eventEmitter.prototype;

// ==================
// = module exports =
// ==================

exports.createAdvertNode = function (nodeName, isDir, parentNode) {
  return new AdvertNode(nodeName, isDir, parentNode);
};
