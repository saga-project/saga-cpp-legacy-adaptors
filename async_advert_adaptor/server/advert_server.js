/*
   advert_server.js
   SAGA::Advert
   
   Created by Hans Christian Wilhelm on 2011-07-16.
   Copyright 2011. All rights reserved.
*/

// =============================================================================
// = node.js modules                                                           =
// =============================================================================

var net         = require('net');
var advertNode  = require('./advert_node.js');
var advertPeers = require('./advert_peers.js');
var murmur      = require('./murmur.js');


// ============================================================================
// = Local variables                                                          =
// ============================================================================

var rootNode  = advertNode.createAdvertNode("", true, null);
var nodeIndex = {};
var peers     = advertPeers.createAdvertPeers();

// ========================
// = Initialize nodeIndex =
// ========================

nodeIndex[murmur.getHash("")] = rootNode;

// ============================================================================
// = Helper functions                                                         =
// ============================================================================

function getPathArray (pathString)
{
  var nodePath = pathString.split("/");
  
  return nodePath.filter(function (x) {
    if (x != "")
    {
      return true;
    }
    
    else 
    {
      return false;
    }
  });
}

function deleteIndex ()
{
  nodeIndex = {};
}

function rebuildIndex (node, path)
{
  path.push(node.nodeName);
  nodeIndex[murmur.getHash(path.join("/"))] = node;
  
  for (var i in node.nodeArray)
  {
    rebuildIndex (node.nodeArray[i], path);
  }
}

// ============================================================================
// = Advert server                                                            =
// ============================================================================

var server = net.createServer(function (socket) {

  // =========================
  // = change event callback =
  // =========================
  
  var changeCallback = function (node)
  {
    socket.write(JSON.stringify(node) + "\r\n");
  }

	/* ============= */
	/* = Handshake = */
	/* ============= */
	
	socket.write("AsyncAdvertServer\r\n");
	
	/* ===================== */
	/* = Callback on close = */
	/* ===================== */

  socket.on("close", function (had_error) {
    if (socket.node != undefined)
    {
      socket.node.removeListener("change", changeCallback);
    }
    
  });
  
  // ================================
  // = Timeout close silent sockets =
  // ================================
  
  //socket.setTimeout(1000 * 180, function() {
  //  socket.destroy();
  //});
  
  /* ==================== */
  /* = Callback on Data = */
  /* ==================== */

	socket.on("data", function (data) {	

	  var message;
	  
	  // ====================================================
	  // = Check for Valid Json data, else close the socket =
	  // ====================================================
	  
	  try 
	  {
      message = JSON.parse(data);
    }
    
    catch(error)
    {
      socket.destroy();
	    return;
    }
    
    // ==================
    // = Message exists =
    // ==================
    
    if (message.command == "exists")
    {
      var path = getPathArray(message.path);
      var hash = murmur.getHash(path.join("/"));
      
      if (hash in nodeIndex)
      {
        socket.write(JSON.stringify(true) + "\r\n");
      }
      
      else 
      {
        socket.write(JSON.stringify(false) + "\r\n");
      } 
    }
    
    // ==================
    // = Message create =
    // ==================
    
    if (message.command == "create")
    {
      var path = getPathArray(message.path); 
      var hash = murmur.getHash(path.join("/"));
      
      if (socket.node != undefined)
      {
        socket.node.removeListener('change', changeCallback);
      }
      
      if (hash in nodeIndex)
      {
        var node = nodeIndex[hash];
        
        socket.node = node;
        socket.write(JSON.stringify(node) + "\r\n");
        
        node.on("change", changeCallback);
      }
      
      else
      {
        var parentPath = path.slice(0,-1);
        var parentHash = murmur.getHash(parentPath.join("/"));
        
        if (parentHash in nodeIndex)
        {
          var parentNode  = nodeIndex[parentHash];
          var childNode   = advertNode.createAdvertNode(path.slice(-1)[0], message.isDir, parentNode);
          
          nodeIndex[murmur.getHash(path.join("/"))] = childNode;
          
          socket.node = childNode;
          socket.write(JSON.stringify(childNode) + "\r\n");

          childNode("change", changeCallback);
          parentNode.addNode(childNode);
        }
        
        else
        {
          socket.write(JSON.stringify({}) + "\r\n");
        }
      }
      
    }

    // =========================
    // = Message createParents =
    // =========================

    if (message.command == "createParents")
    {
      var path = getPathArray(message.path);  
      var hash = murmur.getHash(path.join("/"));

      if (socket.node != undefined)
      {
        socket.node.removeListener('change', changeCallback);
      }

      if (hash in nodeIndex)
      {
        var node = nodeIndex[hash];
              
        socket.node = node;
        socket.write(JSON.stringify(node) + "\r\n");
        
        node.on("change", changeCallback);
      }
      
      else 
      {
        var currentNode = rootNode;
        
        for (var i in path)
        {
          var node = currentNode.exists(path[i]);
          
          if (node == undefined)
          {
            node = advertNode.createAdvertNode(path[i], message.isDir, currentNode);
            
            console.log(path.slice(0,i + 1).join("/"));
            
            nodeIndex[murmur.getHash(path.slice(0,i + 1).join("/"))] = node;
            currentNode.addNode(node);
            currentNode = node;
          }
          
          else 
          {
            currentNode = node;
          }
        }
        
        socket.node = currentNode;
        socket.write(JSON.stringify(currentNode) + "\r\n");
        
        currentNode.on("change", changeCallback);
      }
      
    }
    
    /* ================ */
    /* = Message open = */
    /* ================ */
    
    if (message.command == "open")
    {
      var path = getPathArray(message.path);
      var hash = murmur.getHash(path.join("/"));
      
      if (socket.node != undefined)
      {
        socket.node.removeListener('change', changeCallback);
      }
      
      if (hash in nodeIndex)
      {
        var node = nodeIndex[hash];
        
        socket.node = node;
        socket.write(JSON.stringify(node) + "\r\n");
        
        node.on("change", changeCallback);
      }
      
      else 
      {
        socket.write(JSON.stringify({}) + "\r\n");
      }
    }
    
    // ==================
    // = Message remove =
    // ==================
    
    if (message.command == "remove")
    {
      var path = getPathArray(message.path);
      var hash = murmur.getHash(path.join("/"));
      
      if (hash in nodeIndex)
      {
        var node        = nodeIndex[hash];
        var parentNode  = node.parentNode;
        
        if (parentNode != null)
        {
          parentNode.removeNode(node);
          node.deleteNode();
          
          deleteIndex();
          rebuildIndex(rootNode, []);
        }
        
        else
        {
          node.deleteNode();
          
          deleteIndex();
          rebuildIndex(rootNode, []);
        }
        
      }
    }

    /* ======================== */
    /* = Message setAttribute = */
    /* ======================== */
    
    if (message.command == "setAttribute")
    {
      if (socket.node != null)
      {
        socket.node.addAttribute(message.key, message.value);
      }
    }
    
    /* =========================== */
    /* = Message removeAttribute = */
    /* =========================== */
    
    if (message.command == "removeAttribute")
    {
      if (socket.node != null)
      {
        socket.node.removeAttribute(messsage.key);
      }
    }

  });
});

server.listen(8124,'0.0.0.0');
