Async Advert Server Install guide. 

1. Install nodeJS

Download nodeJS sources from http://nodejs.org/#download
and build it with 

./configure --prefix=/install/path 
make 
make install

Update your $PATH environment variable
it is commonly done in your .bashrc or .bash_profile file.

export PATH=$PATH:/install/path/bin


2. Setup the Async Advert server 

Check out the current server sources form the SVN repository.

svn co https://svn.cct.lsu.edu/repos/saga-adaptors/async_advert_adaptor/server/

Now we need to install the needed nodeJS modules. 
First we will start with mongoose and mongodb native driver.

cd server 
npm install mongoose
npm install mongodb --mongodb:native

this should create a node_modules directory in the server dirctory, 
so you should find 

$HOME/server/node_modules

Now we need to install ZMQ from source. Download the zmq version 2.1
tarball from http://www.zeromq.org/intro:get-the-software

./configure --prefix=/install/path 
make 
make install


Install the zmq binding for nodeJs.
Go back to your /server path and do 

export PKG_CONFIG_PATH=/install/path/lib/pkgconfig 
npm install zmq

Don't forget to expand your LD_LIBRARY_PATH to 

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/install/path/zmq/lib


Lets test if all nodeJS modules are working

node
require('mongoose')
require('zmq')
process.exit()

You shouldn't see any error's ;) 

3. Install MongoDB

Download MongoDB for your system http://www.mongodb.org/downloads 
and unpack it. 

Create a directory where you want to store the MongoDB database

mkdir ./mongodb_database

Running MongoDB as a Daemon

./mongod --fork --logpath /log/path/mongodb.log--dbpath /mongodb_database

4. Running the Async Advert server

cd /server
node advert_zmq_mongodb

