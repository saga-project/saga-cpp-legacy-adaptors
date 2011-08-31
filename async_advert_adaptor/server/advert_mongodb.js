// 
//  advert_mongodb.js
//  SAGA
//  
//  Created by Hans Christian Wilhelm on 2011-08-18.
//  Copyright 2011. All rights reserved.
// 

// =============================================================================
// = Node.js includes                                                          =
// =============================================================================

var net       = require('net');
var mongoose  = require('mongoose');

// =============================================================================
// = Mongoose Schema and Model                                                 =
// =============================================================================

var AdvertNodeSchema = new mongoose.Schema({
  path        : { type: String, unique: true}, 
  name        : String,
  dir         : Boolean,
  nodes       : [],
  attributes  : {},
  data        : String
});

var AdvertNode = mongoose.model('AdvertNode', AdvertNodeSchema);

// =============================================================================
// = Connect Mongoose to MongoDB                                               =
// =============================================================================

var mongoDB = "mongodb://localhost/advert";

mongoose.connect(mongoDB);

mongoose.connection.on("open", function (){
  console.log("Connection opened to " + mongoDB);
});

mongoose.connection.on("close", function() {
  console.log("Connection closed to " + mongoDB);
});

// =============================================================================
// = Check if we have a root Node                                              =
// =============================================================================

AdvertNode.findOne({path:"/"}, function (error, node){
  
  console.log(JSON.stringify(node));
  
  if (node == null)
  {
    var node = new AdvertNode();
    
    node.path = "/";
    node.dir  = true;
    
    node.save();
  }
});

// =============================================================================
// = Sockets watch list                                                        =
// =============================================================================

var socketWatchList = {} // {hash: [socket, ... ], ...}

// =============================================================================
// = Helper functions                                                          =
// =============================================================================

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

// =============================================================================
// = Socket helper functions                                                   =
// =============================================================================

function registerSocket(id, socket)
{
  if (id in socketWatchList)
  {
    if (!(socket in socketWatchList[id]))
    {
      socketWatchList[id].push(socket);
    }
  }
  
  else
  {
    socketWatchList[id] = [socket];
  }
}

function unregisterSocket(id, socket)
{
  if (id in socketWatchList)
  {
    var index = socketWatchList[id].indexOf(socket);
    
    if (index != -1)
    {
      socketWatchList[id].splice(index, 1);
    }
  }
}

function removeSocket(socket)
{
  for (var i in socketWatchList)
  {
    var index = socketWatchList[i].indexOf(socket);
    
    if (index != -1)
    {
      socketWatchList[i].splice(index, 1);
    }
  }
}

function notifySockets(id)
{
  if (id in socketWatchList)
  {
    AdvertNode.findById(id, function (error, node) {
      
      for (var i in socketWatchList[id])
      {
        responseUpdated(socketWatchList[id][i], node);
      }
      
    });
  }
}

function removeNotifySockets(node)
{
  if (node.id in socketWatchList)
  {
    for (var i in socketWatchList[node.id])
    {
      responseRemoved(socketWatchList[node.id][i], node);
    }
    
    delete socketWatchList[node.id];
  }
}
// =============================================================================
// = Response helper functions                                                 =
// =============================================================================

function responseExists (socket, value)
{
  var response = {
    command : "exists", 
    data    : value
  }
  
  var responseString = JSON.stringify(response);
  responseString += "\r\n";
  
  try 
  {
    socket.write(responseString);
  }
  
  catch (error) { socket.destroy(); }
}

function responseUpdated (socket, node) 
{
  var response = {
    command : "updated",
    data    : node
  }
  
  var responseString = JSON.stringify(response);
  responseString += "\r\n";
  
  try 
  {
    socket.write(responseString);
  }
  
  catch (error) { socket.destroy(); }
}

function responseRemoved (socket, node)
{
  var response = {
    command : "removed",
    data    : node.path
  }
  
  var responseString = JSON.stringify(response);
  responseString += "\r\n";
  
  try 
  {
    socket.write(responseString);
  }
  
  catch (error) { socket.destroy(); }
}

function responseError (socket, path)
{
  var response = {
    command : "error",
    data    : path
  }
  
  var responseString = JSON.stringify(response);
  responseString += "\r\n";
  
  try 
  {
    socket.write(responseString);
  }
  
  catch (error) { socket.destroy(); }
}

// =============================================================================
// = Server                                                                    =
// =============================================================================

var server = net.createServer(function (socket) {
  
  // ====================================
  // = Handshake                        =
  // ====================================
  
  socket.write("AsyncAdvertServer\r\n");
  
  // ===================================
  // = Callback on close               =
  // ===================================
  
  socket.on("close", function (had_error) {
    removeSocket(socket);
  });
  
  // ===================================
  // = callback on data                =
  // ===================================
  
  socket.on("data", function (data) {
  
    var message;
    
    // =========================
    // = check for valid JSON  =
    // =========================
    
    try
    {
      message = JSON.parse(data);
    }
    
    catch (error)
    {
      socket.destroy();
      return;
    }
    
    // ==================
    // = Message Exists =
    // ==================
    
    if (message.command == "exists")
    {
      var pathArray = getPathArray(message.path);
      
      var pathString = "/"; 
      pathString += pathArray.join("/");
    
      AdvertNode.findOne({path: pathString}, function (error, node) {
        if (node != null)
        {
          responseExists(socket, true);
        }
        
        else
        {
          responseExists(socket, false);
        }
      });
      
    }
    
    // ================
    // = Message open =
    // ================
    
    if (message.command == "open")
    {
      var pathArray = getPathArray(message.path);
      
      AdvertNode.findOne({path: "/" + pathArray.join("/")}, function (error, node) {
        if(node == null)
        {
          responseError(socket, message.path);
        }
        
        if (node != null)
        {
          responseUpdated(socket, node);
          registerSocket(node.id, socket);
        }
      });
    }
    
    // ==================
    // = Message create =
    // ==================
    
    if (message.command == "create")
    {
      var pathArray = getPathArray(message.path);
      
      AdvertNode.findOne({path: "/" + pathArray.join("/")}, function (error, node) {
        
        if (node != null)
        {
          responseUpdated(socket, node);
          registerSocket(node.id, socket);
        }
        
        else 
        {
          var parentPathArray = pathArray.slice(0,-1);
          
          AdvertNode.findOne({path: "/" + parentPathArray.join("/")}, function (error, node) {
            
            if (node == null)
            {
              responseError(socket, message.path);
            }
            
            if (node != null)
            {
              var newNode   = new AdvertNode();
              newNode.path  = "/" + pathArray.join("/");
              newNode.name  = pathArray.slice(-1);
              newNode.dir   = message.dir;
              
              newNode.save(function (error) {
                responseUpdated(socket, newNode);
                registerSocket(newNode.id, socket);
              });
              
              node.nodes.push({ name: newNode.name, dir: newNode.dir });
              
              node.save(function (error) {
                notifySockets(node.id);
              });
            }
            
          });
        }
      });
      
    }

    // =========================
    // = Message createParents =
    // =========================
    
    if (message.command == "createParents")
    {
      var pathArray = getPathArray(message.path);
      
      var createParents = function (i)
      {
        if (i == pathArray.length)
        {
          var parentPath  = "/" + pathArray.slice(0, i - 1).join("/");
          var nodePath    = "/" + pathArray.slice(0, i).join("/");
          
          AdvertNode.findOne({path: parentPath}, function (error, parentNode) {
            AdvertNode.findOne({path: nodePath}, function (error, node) {
              
              if (node != null)
              {
                responseUpdated(socket, node);
                registerSocket(node.id, socket);
              }
              
              else 
              {
                var newNode   = new AdvertNode();
                newNode.path  = nodePath;
                newNode.name  = pathArray.slice(i - 1, i);
                newNode.dir   = message.dir;
                
                parentNode.nodes.push({name :newNode.name, dir: newNode.dir});
                
                parentNode.save(function (error) {
                  notifySockets(parentNode.id);
                });
                
                newNode.save(function (error) {
                  responseUpdated(socket, newNode);
                  registerSocket(newNode.id, socket);
                });
              }
              
            });
          });
        
          return;
        }
        
        else 
        {
          var parentPath  = "/" + pathArray.slice(0, i - 1).join("/");
          var nodePath    = "/" + pathArray.slice(0, i).join("/");
          
          AdvertNode.findOne({path: parentPath}, function (error, parentNode) {
            AdvertNode.findOne({path: nodePath}, function (error, node) {
              
              if (node != null)
              {
                createParents(i + 1);
              }
              
              else 
              {
                var newNode   = new AdvertNode();
                newNode.path  = nodePath;
                newNode.name  = pathArray.slice(i - 1, i);
                newNode.dir   = message.dir;
                
                parentNode.nodes.push({ name: newNode.name, dir: newNode.dir });
                
                parentNode.save(function (error) {
                  notifySockets(parentNode.id);
                });
                
                newNode.save(function (error) {
                  createParents(i + 1);
                });
              }
              
            });        
          }); 
        }
      }
      
      createParents(0);
    }
    
    // ==================
    // = Message remove =
    // ==================
    
    if (message.command == "remove")
    {
      var pathArray = getPathArray(message.path);
      
      var recursiveRemove = function (node)
      {
        for (var i in node.nodes)
        {
          var pathString = node.path + "/" + node.nodes[i].name;
          
          AdvertNode.findOne({path: pathString}, function (error, node) {
            if (node != null)
            {
              recursiveRemove(node)
              
              node.remove(function (error) {
                removeNotifySockets(node)
              });
            }
          });
        }
      }
      
      // ==============================
      // = Never remove the root node =
      // ==============================
      
      AdvertNode.findOne({path: "/" + pathArray.join("/")}, function (error, node) {
        if (node != null)
        {
          recursiveRemove(node);
          
          if (node.path != "/")
          {
            node.remove(function (error) {
              removeNotifySockets(node);
            });
            
            var parentNode = pathArray.slice(0,-1);
            
            AdvertNode.findOne({path: "/" + parentNode.join("/")}, function (error, parentNode) {
              if (parentNode != null)
              {
                for (var i in parentNode.nodes)
                {
                  if (parentNode.nodes[i].name == node.name)
                  {
                    parentNode.nodes.splice(i, 1);
                  }
                }
               
               parentNode.save(function (error) {
                 notifySockets(parentNode.id);
               }); 
                
              }
            });
          }
        }
      });
      
    }
    
    // =================
    // = Message close =
    // =================
    
    if (message.command == "close")
    {
      var pathArray = getPathArray(message.path);
      
      AdvertNode.findOne({path: "/" + pathArray.join("/")}, function (error, node) {
        if (node != null)
        {
          unregisterSocket(node.id, socket);
        }
      });   
    }
    
    // ========================
    // = Message setAttribute =
    // ========================
    
    if (message.command == "setAttribute")
    {
      var pathArray = getPathArray(message.path);
      
      AdvertNode.findOne({path: "/" + pathArray.join("/")}, function (error, node) {
        if (node != null)
        {
          if (node.attributes == null)
          {
            node.attributes = {};
          }
          
          node.attributes[message.key] = message.value;
          
          node.commit('attributes');
          node.save(function (error) {
            notifySockets(node.id);
          });
        }
      });
    }
    
    // ===========================
    // = Message removeAttribute =
    // ===========================
    
    if (message.command == "removeAttribute")
    {
      var pathArray = getPathArray(message.path);
      
      AdvertNode.findOne({path: "/" + pathArray.join("/")}, function (error, node) {
        if (node != null)
        {
          delete node.attributes[message.key];
          
          console.log(node.attributes);
          
          node.commit('attributes');
          node.save(function (error) {
            notifySockets(node.id);
          });
        }
      });
    }
    
  });
  
});

// =============================================================================
// = Start the server                                                          =
// =============================================================================

server.listen(8080,'0.0.0.0');
