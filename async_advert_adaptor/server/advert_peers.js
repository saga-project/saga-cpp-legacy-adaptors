// 
//  advert_peers.js
//  SAGA
//  
//  Created by Hans Christian Wilhelm on 2011-08-10.
//  Copyright 2011 All rights reserved.
// 

// =================
// = AdvertPeers  =
// =================

function AdvertPeers ()
{
  this.peerList = [];
  
  this.addPeer = function (socket)
  {
    this.peerList.push(socket);
	}
  
  this.removePeer = function (socket)
  {
    var i = this.peerList.indexOf(socket);
    this.peerList.splice(i, 1);
  }
  
  this.getNode = function (socket)
  {
    var i = this.peerList.indexOf(socket);
    return this.peerList[i].node;
  }
  
  this.notifyPeers = function (node)
  {
    for (var i in this.peerList)
    {
      if (this.peerList[i].node == node)
      {
        var peerNode    = this.peerList[i].node;
        var peerSocket  = this.peerList[i];
        
        peerSocket.write(JSON.stringify(peerNode) + "\r\n");
      }
    }
  }
}

// ===========
// = Exports =
// ===========

exports.createAdvertPeers = function () {
  return new AdvertPeers();
}