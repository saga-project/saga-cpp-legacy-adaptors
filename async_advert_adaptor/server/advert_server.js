/*
   advert_server.js
   SAGA::Advert
   
   Created by Hans Christian Wilhelm on 2011-07-16.
   Copyright 2011. All rights reserved.
*/

/* =================== */
/* = Node.js modules = */
/* =================== */

var net   = require('net');
var path  = require('path');

/* ============================================= */
/* = Initial structure to hold the advert data = */
/* ============================================= */

function advertNode (nodeName, isDir)
{
  this.nodeName     = nodeName;
  this.nodeList     = [];
  this.attributes   = {};
  this.isDir        = isDir;
  
  this.addNode = function (node)
  {
    this.nodeList.push(node);
    peers.notifyPeers(this);
  }
  
  this.removeNode = function (node)
  {
    for (var i in this.nodeList)
    {
      if (this.nodeList[i].nodeName == node.nodeName)
      {
        this.nodeList.splice(i,1);
      }
    }
  }
  
  this.nodeExist = function (nodeName)
  {
    var result = false;
    
    for (var i in this.nodeList)
    {
      if (this.nodeList[i].nodeName == nodeName)
      {
        result = true;
        break;
      }
    }
    
    return result;
  }
  
  this.getNode = function (nodeName)
  {
    for (var i in this.nodeList)
    {
      if (this.nodeList[i].nodeName == nodeName)
      {
        return this.nodeList[i];
      }
    }
  }
  
  this.getFlatNode = function ()
  {
    var flatNode = {"nodeName":this.nodeName, "nodeList":[], "attributes":this.attributes, "isDir":this.isDir};
  
    for (var i in this.nodeList)
    {
      flatNode.nodeList.push({"nodeName": this.nodeList[i].nodeName, "isDir": this.nodeList[i].isDir});
    }
    
    return flatNode;
  }
  
  this.setAttribute = function (key, value)
  {
    this.attributes[key] = value;
    peers.notifyPeers(this);
  }
  
  this.removeAttribute = function (key)
  {
    delete this.attributes[key];
    peers.notifyPeers(this);
  }
}

var rootNode = new advertNode("root", true);

/* =============================================== */
/* = Clients to be notified if something changes = */
/* =============================================== */

function advertPeers ()
{
  this.peerList = [];
  
  this.addPeer = function (socket, node)
  {
    this.peerList.push({"socket":socket, "node":node});
    socket.write(JSON.stringify(node.getFlatNode()) + "\r\n");
	}
  
  this.removePeer = function (socket)
  {
    for (var i in this.peerList)
    {
      if (this.peerList[i].socket == socket)
      {
        this.peerList.splice(i, 1);
      }
    }
  }
  
  this.getNode = function (socket)
  {
    for (var i in this.peerList)
    {
      if (this.peerList[i].socket == socket)
      {
        return this.peerList[i].node;
      }
    }
  }
  
  this.notifyPeers = function (node)
  {
    for (var i in this.peerList)
    {
      if (this.peerList[i].node == node)
      {
        var peerNode    = this.peerList[i].node;
        var peerSocket  = this.peerList[i].socket;
        
        peerSocket.write(JSON.stringify(peerNode.getFlatNode()) + "\r\n");
      }
    }
  }
}

var peers = new advertPeers();

/* ================================= */
/* = Advert communication protocol = */
/* ================================= */

var advertCommands = 
  {
    openNode        : "openNode",
    removeNode      : "removeNode",
    closeNode       : "closeNode",
    setAttribute    : "setAttribute",
    removeAttribute : "removeAttribute"
  };
  
var advertFlags = 
  {
    none          : "none",
    create        : "create",
    createParents : "createParents"
  };

  
  
/* ===================== */
/* = Create the server = */
/* ===================== */

var server = net.createServer(function (socket) {
  
  socket.setNoDelay(true);
	
	/* ============= */
	/* = Handshake = */
	/* ============= */
	
	socket.write("AsyncAdvertServer\r\n");
	
	/* ===================== */
	/* = Callback on close = */
	/* ===================== */

  socket.on("close", function (had_error) {
    peers.removePeer(socket);
  });
  
  //socket.setTimeout(1000 * 30, function() {
  //  peers.removePeer(socket);
  //  socket.destroy();
  //});
  
  /* ==================== */
  /* = Callback on Data = */
  /* ==================== */

	socket.on("data", function (data) {
	  
	console.log(String(data));	
	
	  var message;
	  
	  try 
	  {
      message = JSON.parse(data);
    }
    
    catch(error)
    {
      socket.destroy();
      console.log("JSON Error");
	return;
    }

    /* ================ */
    /* = Message open = */
    /* ================ */
    
    if (message.command == "open")
    {
      peers.removePeer(socket);
      
      var nodePath = message.path.split("/");
      var currentNode = rootNode;

      for (var i in nodePath)
      {
        if (nodePath[i] == "")
        {
          continue;
        }

        if (currentNode.nodeExist(nodePath[i]))
        {   
          currentNode = currentNode.getNode(nodePath[i]);
        }

        else 
        {
          var newNode = new advertNode(nodePath[i], true);

          currentNode.addNode(newNode);
          currentNode = newNode;
        }
      }
      
      peers.addPeer(socket, currentNode);
    }

    /* ======================== */
    /* = Message setAttribute = */
    /* ======================== */
    
    if (message.command == "setAttribute")
    {
      var currentNode = peers.getNode(socket);
      currentNode.setAttribute(message.key, message.value);
    }
    
    /* =========================== */
    /* = Message removeAttribute = */
    /* =========================== */
    
    if (message.command == "removeAttribute")
    {
      console.log("removeAttribute");
      
      var currentNode = peers.getNode(socket);
      currentNode.removeAttribute(message.key);
    }

  });
});

server.listen(8124,'0.0.0.0');
