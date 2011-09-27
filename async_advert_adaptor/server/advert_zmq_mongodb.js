// 
//  zmq_test.js
//  SAGA
//  
//  Created by Hans Christian Wilhelm on 2011-09-20.
//  Copyright 2011. All rights reserved.
// 

// ====================
// = Node.js includes =
// ====================

var zmq       = require("zmq");
var mongoose  = require('mongoose');

// =============================
// = Mongoose schema and model =
// =============================

var AdvertNodeSchema = new mongoose.Schema({
  path        : { type: String, unique: true}, 
  name        : String,
  dir         : Boolean,
  nodes       : [],
  attributes  : {},
  data        : String
});

var AdvertNode = mongoose.model('AdvertNode', AdvertNodeSchema);

// ======================
// = Connect to MongoDB =
// ======================

var mongoDB = "mongodb://localhost/advert";

mongoose.connect(mongoDB);

mongoose.connection.on("open", function (){
  console.log("Connection opened to " + mongoDB);
});

mongoose.connection.on("close", function() {
  console.log("Connection closed to " + mongoDB);
});

// ============================================
// = Check if there is a root node in MongoDB =
// ============================================

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

// ===================================
// = Helper functions                =
// ===================================

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

// ===================================
// = ZeroMQ Sockets ZMQ_PUB, ZMQ_REP =
// ===================================

var publisher = zmq.createSocket("pub");
publisher.bind("tcp://*:5556", function () {});

var responder = zmq.createSocket("rep");

responder.bind("tcp://*:5557", function () {
  
  responder.on('error', function(error) {
    console.log(error);
  });
  
  responder.on('message', function (command, data) {
    
    var message = JSON.parse(data);
    
    // ==================
    // = Command exists =
    // ==================
    
    if (command.toString('utf8') == "exists")
    {
      var pathArray   = getPathArray(message.path);
      var pathString  = "/" + pathArray.join("/");
      
      AdvertNode.findOne({path: pathString}, function (error, node) {
        if(node != null)
        {
          responder.send("true");
        }
        
        else
        {
          responder.send("false");
        }
      });
    }
    
    // ================
    // = Command open =
    // ================
    
    if (command.toString('utf8') == "open")
    {
      var pathArray   = getPathArray(message.path);
      var pathString  = "/" + pathArray.join("/");
      
      AdvertNode.findOne({path: pathString}, function (error, node) {
        if (node == null)
        {
          responder.send("error", JSON.stringify({}));
        }
        
        else 
        {
          responder.send("ok", JSON.stringify(node));
        }
      });
    }
    
    // ==================
    // = Command create =
    // ==================
    
    if (command.toString('utf8') == "create")
    {
      var pathArray   = getPathArray(message.path);
      var pathString  = "/" + pathArray.join("/");
      
      AdvertNode.findOne({path: pathString}, function (error, node) {
        if (node != null)
        {
          responder.send("ok", JSON.stringify(node));
        }
        
        else 
        {
          var parentPathArray   = pathArray.slice(0, -1);
          var parentPathString  = "/" + parentPathArray.join("/");
          
          AdvertNode.findOne({path: parentPathString}, function (error, node) {
            if (node != null)
            {
              var newNode   = new AdvertNode();
              newNode.path  = pathString;
              newNode.name  = pathArray.slice(-1);
              newNode.dir   = message.dir;

              node.nodes.push({ name: newNode.name, dir: newNode.dir });
              
              node.save(function (error) {
                newNode.save(function (error) {
                  responder.send("ok", JSON.stringify(newNode));
                });
                
                publisher.send(parentPathString, "updated", JSON.stringify(node));
              });
            }
            
            else 
            {
              responder.send("error");
            }
          });
        }
      }); 
    }
    
    // =========================
    // = Command createParents =
    // =========================
    
    if (command.toString('utf8') == "createParents")
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
                responder.send("ok", JSON.stringify(node));
              }
              
              else 
              {
                var newNode   = new AdvertNode();
                newNode.path  = nodePath;
                newNode.name  = pathArray.slice(i - 1, i);
                newNode.dir   = message.dir;

                parentNode.nodes.push({name :newNode.name, dir: newNode.dir});
                
                parentNode.save(function (error) {
                  newNode.save(function (error) {
                    responder.send("ok", JSON.stringify(newNode));
                  });
                  
                  publisher.send(parentPath, "updated", JSON.stringify(node));
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
                newNode.dir   = true;
                
                parentNode.nodes.push({ name: newNode.name, dir: newNode.dir });
                
                parentNode.save(function (error) {
                  newNode.save(function (error) {
                    createParents(i + 1);
                  });
                  
                  publisher.send(parentPath, "updated", JSON.stringify(node));
                });
              }
              
            });        
          }); 
        }
      }
      
      createParents(0);
    }
    
    // ==================
    // = Command remove =
    // ==================
    
    if (command.toString('utf8') == "remove")
    {
      var pathArray   = getPathArray(message.path);
      var pathString  = "/" + pathArray.join("/"); 
      
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
                publisher.send(pathString, "removed");
              });
            }
          });
        }
      }
      
      AdvertNode.findOne({path: pathString}, function (error, node) {
        if (node != null)
        {
          recursiveRemove(node);
          
          if (node.path != "/")
          {
            node.remove(function (error) {
              publisher.send(pathString, "removed");
            });
            
            var parentNodeArray   = pathArray.slice(0,-1);
            var parentPathString  = "/" + parentNodeArray.join("/");
            
            AdvertNode.findOne({path: parentPathString}, function (error, parentNode) {
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
                 publisher.send(parentPathString, "updated", JSON.stringify(node));
               }); 
                
              }
            });
          }
        }
        
        responder.send("ok");
      }); 
    }
    
    // ========================
    // = Command setAttribute =
    // ========================
    
    if (command.toString('utf8') == "setAttribute")
    {
      var pathArray   = getPathArray(message.path);
      var pathString  = "/" + pathArray.join("/");
      
      AdvertNode.findOne({path: pathString}, function (error, node) {
        if (node != null)
        {
          if (node.attributes == null)
          {
            node.attributes = {};
          }
          
          node.attributes[message.key] = message.value;
          
          node.commit('attributes');
          node.save(function (error) {
            responder.send("ok", JSON.stringify(node));
            publisher.send(pathString, "updated", JSON.stringify(node));
          });
        }
      });  
    }
    
    // ===========================
    // = Command removeAttribute =
    // ===========================
    
    if (command.toString('utf8') == "removeAttribute")
    {
      var pathArray = getPathArray(message.path);
      var pathString  = "/" + pathArray.join("/");
      
      AdvertNode.findOne({path: pathString}, function (error, node) {
        if (node != null)
        {
          delete node.attributes[message.key];
          
          node.commit('attributes');
          node.save(function (error) {
            responder.send("ok", JSON.stringify(node));
            publisher.send(pathString, "updated", JSON.stringify(node));
          });
        }
      });
    }
  });
});