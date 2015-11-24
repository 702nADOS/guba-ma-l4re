var exports = module.exports = {};


/*
 * Helper function that returns a 4 byte buffer
 * parameter: hex, a hex number
 * returns : buffer representation of hex in int32 Little Endian Format
 */
function getInt32LE(hex){
  var buf = new Buffer(4);
  buf.writeUInt32LE(hex, 0);
  return buf;
}

/*
 * Helper function to test if buffers are equal
 */
function areBuffersEqual(bufA, bufB) {
    var len = bufA.length;
    if (len !== bufB.length) {
        return false;
    }
    for (var i = 0; i < len; i++) {
        if (bufA.readUInt8(i) !== bufB.readUInt8(i)) {
            return false;
        }
    }
    return true;
}

/*
/* the dom0_client that we could require
 * and create/remove operations
 */

var net = require('net'),
    debug = require('./debug.js'),
    sleep = require('node-sleep').sleep,
    EventEmitter = require( "events" ).EventEmitter,
    fs = require('fs'),
    colors = require('colors')
    ;

var dom0_client = function() {
  this.server_details = {}; // JS Object, details of the client Server, host and port
  this.clientSocket = null; // variable that holds the socket connection to the client
  this.monitorSocket = null;
  this.connected = false;// BOOL, state of the connection
  this.magic_numbers = require('./magic_numbers.js'); // this file has all the communication protocol IDs
  this.send_queue = [];
};

// make the dom0 client inherit all the properties
// of eventEmitter
dom0_client.prototype = new EventEmitter();

/*
 * function init: initializes the communication and sets the channels
 * parameters:
 *  server_details : JSON object that has the details of the server
 *    port
 *    host
 *
 */
dom0_client.prototype.init = function(options) {
  var client = this;
  debug.log('Creating the client socket to talk to server');

  // create the client, connect to it
  this.clientSocket = net.createConnection(options.server_details);

  this.clientSocket.on('connect', function(){
    client.emit('connected');
  }.bind(this));

  this.clientSocket.on('data', function(data) {

    //console.log(getInt32LE(this.magic_numbers['go_send']));

    // if the client is ready to receive the next task binary
    // send the next binary from the queue
    //if(data.compare(getInt32LE(this.magic_numbers['go_send']))){
    if(areBuffersEqual(data, getInt32LE(this.magic_numbers['go_send']))){
      client.emit('sendNextBinary');
      console.log('Client is ready to get next task')
      this.sendNextTask();
      return;
    }
    // XXX doing the +'' to make the number ot a string. fancy JS Hack
    console.log((data.readInt32LE(0)+'').grey);

  }.bind(this));

};

/*
 * function sendControl : sends a control message to the server
 * parameters:
 *  message: HEX, part of the magic_numbers that defines which
 *          control message we are sending.
 */
dom0_client.prototype.sendControl = function() {
  debug.log('sending control message to server ');

  this.clientSocket.write(getInt32LE(this.magic_numbers.control));
};

/*
 * function sendLUA : sends a LUA message to the server
 * parameters:
 *  message: STRING, the lua message that should be sent
 *          to the server
 */
dom0_client.prototype.sendLua = function(message) {
  debug.log('sending lua meesage to the server '+ message);

  this.clientSocket.write(getInt32LE(this.magic_numbers.lua));
  console.log(getInt32LE);
  debug.log('send message length ');
  this.clientSocket.write(getInt32LE(message.length));
  console.log(getInt32LE(message.length));
  debug.log('send Message');
  this.clientSocket.write(message);

  // TODO: we should get a response if the LUA message was received
  // successfully or not.
  // XXX: Its only if it was received, nothing about if the LUA string
  // exectued right or not.
};

/*
 * function sendBinary: sends a binary to the server
 * parameters:
 *  binary: BUFFER, the binary that we should send to the server,
 *          instance of node Buffer class.
 */
dom0_client.prototype.sendBinary = function(binary) {
  debug.log('sending binary file to the server');

  this.sendControl();

  this.clientSocket.write(getInt32LE(this.magic_numbers.send_binary));
  this.clientSocket.write(getInt32LE(binary.length));

  debug.log('expecting an OK by this time');
  //sleep(3000);

  // TODO: wait for GO_SEND here. need to make these parts more elegant
  this.clientSocket.write(binary, function(){
    console.log("binary_sent");
  });
};

/**
 *  function sendTaskDescription: sends a task description to server
 *           and then continues to send the binaries one by one.
 *  parameters:
 *    tasks : JSON representation of the tasks.xml file,
 *            contains the list of tasks with some basic paramters
 */
dom0_client.prototype.sendTaskDescription = function(tasks){
  debug.log('sending taskDescription to the server');

  // Protocol :
  //    - Send the magic numbers to send task description
  //    - Send the length of the json task description
  //    - Send the task description
  //    - send the control agian
  //    - sent magic number to send binaries
  //    - send number of binaries to be sent
  //    - for each Binary
  //      * send binary name (4 byte)
  //      * send binary length
  //      * send binary
  //      * LOOP


  this.writeToClient(this.magic_numbers.task_desc);
  var tasks_string = JSON.stringify(tasks);

  console.log(tasks_string.length);

  this.writeToClient(tasks_string.length);
  this.writeToClient(tasks_string, false);
};

dom0_client.prototype.sendBinary = function(binaries){
  for(var binary in binaries){
    this.send_queue.push({
      name : binaries[binary],
      binary_path : "_bin/"+binaries[binary]
    });
  }
  //this.send_queue.push({
    //name : "namasteb",
    //binary_path : "_bin/namaste"
  //});

  this.writeToClient(this.magic_numbers.send_binaries);
  this.writeToClient(2);
  this.sendNextTask();
}
/**
 *  function sendNextTask: sends a task binary to the server
 */
dom0_client.prototype.sendNextTask = function(){
  if(this.send_queue.length > 0)
  {
    var task = this.send_queue.pop();

    var binary = fs.readFileSync(task.binary_path);

    console.log('Writing binary, ' + task.name);
    this.writeToClient(task.name, false);
    this.clientSocket.write(getInt32LE(binary.length));
    this.clientSocket.write(binary, function(){ console.log(task.name+" written"); }.bind(this));
  }
}

/**
 *  function sendTaskDescription: sends a task description to server
 *           and then continues to send the binaries one by one.
 *  parameters:
 *    tasks : JSON representation of the tasks.xml file,
 *            contains the list of tasks with some basic paramters
 */
dom0_client.prototype.start = function(){
  debug.log('starting tasks on the server');
  this.writeToClient(this.magic_numbers.start);
};

/*
 * function close : closes the socket connection
 *
 */
dom0_client.prototype.close = function(){
  if(this.clientSocket)
    this.clientSocket.end();
};

/**
 *  Helper function to write a hex to the client. hex is converted to Int32LittleEndian format
 *  parameter: hex
 *             LE : if false then not converted to Low endian format
 *
 */
dom0_client.prototype.writeToClient = function(hex, LE){
  if(typeof LE === 'undefined') LE = true;

  if (LE)
    this.clientSocket.write(getInt32LE(hex));
  else
    this.clientSocket.write(hex);

  //sleep(1000);
}

module.exports = dom0_client;
