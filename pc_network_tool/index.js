#!/usr/bin/env node

// setup the required modules
var xml2json = require('xml2json'),
    fs = require('fs'),
    dom0_client = require('./dom0_client.js'),
    magic_numbers = require('./magic_numbers.js'),
    sleep = require('node-sleep').sleep,
    readline = require('readline')
    ;
var client,
    clientConnected = false;

// XXX: Config
var tasksFile = 'tasks.xml';
var server_details = {
  'host': '192.168.0.14',
  'port': '3001'
};
var monitor_client_details = {
  'host' : '192.168.0.254',
  'port' : '3002'
};

// read the XML data from the tasks.xml file
var xml = fs.readFileSync(tasksFile);
var tasks = xml2json.toJson(xml);
tasks = JSON.parse(tasks);
processed_tasks = cleanup(tasks);
tasks = processed_tasks.tasks;
binaries = processed_tasks.binaries;

// client showcase

var rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});
rl.setPrompt('> ');

printHelp();

rl.on('line', function(cmd){
  switch(cmd){
    case 'q':
      if(isClientConnected())
        client.close();
      process.exit();
      break;
    case 'connect':
      client = new dom0_client();
      client.init({server_details: server_details, mon_client: monitor_client_details});
      client.on('connected', function(){
        console.log('connected');
        clientConnected = true;
      });
      break;
    case 'lua':
      if(isClientConnected())
        client.sendLua("print 'hello';");
      break;
    case 'task':
      if(isClientConnected())
        client.sendTaskDescription(tasks);
      break;
    case 'binary':
      if(isClientConnected())
        client.sendBinary(binaries);
      break;
    case 'start':
      if(isClientConnected())
        client.start();
      break;
    default:
      printHelp();
  };
  rl.prompt();
})


/*
 * Helper function for xml parsing
 */

function printHelp(){
  console.log('Following functions are available');
  console.log('q - quit');
  console.log('connect - start client connection');
  console.log('lua - send lua Hello');
  console.log('task - send task description');
  console.log('binary - send binary');
  console.log('start - start tasks');
  console.log('------------------------------------');
  rl.write('');
  rl.prompt();
}
function cleanup(tasks){
  tasks = tasks.taskset.periodictask;
  binaries = [];
  for(var task in tasks){
    if (binaries.indexOf(tasks[task].pkg) === -1)
      binaries.push(tasks[task].pkg);
    delete tasks[task].ucfirmrt;
    delete tasks[task].uawmean;
  }
  return {
    tasks: tasks,
    binaries: binaries
  }
}

function isClientConnected(){
  if(clientConnected) return true;
  else { console.log('client not connected'); return false;}
}

/*
 * TASK EXAMPLE
 *
 * {
  id: 1,
  executiontime: 3,
  criticaltime: 3,
  priority: 1,
  period: 4,
  offset: 0
}*/
