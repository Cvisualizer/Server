var exec = require('child_process').exec,child;
var fs = require('fs');
var http = require('http');
var async=require('async');

var code = "int a,b; int g(int a,int b){return a * b * 10;} int f(){int a;a = 5;return a * a;}int main(){a = b = 10;printf g(5, 5);printf(f());printf(a);return(0);}";
var source={};

function writeFile(){
    fs.writeFileSync('hoge.c', code);
    console.log("readed!");
}

function execCompiler(file){
  //child_process.exec関数を利用する
}

function parseCode(){
  writeFile();
  //execCompiler('hoge.c');
}
 
var server=http.createServer(function (request, response) {
  response.writeHead(200, {'Content-Type': 'text/plain'});
  response.end();
});

var io=require('socket.io').listen(server);
  io.sockets.on('connection',function(socket){
  //console.log(source.info.max);
  socket.on("message",function(data){
    code=data;
    console.log("connected!");
    parseCode();
     async.series([
      function (callback){
         exec('./Compiler '+'hoge.c',
        // exec関数は非同期関数なのでcallbackを取り、そこでstdout, stderrを取る
        function (error, stdout, stderr) {
        //console.log('stdout: ' + stdout);
        //console.log('stderr: ' + stderr);
        source = (new Function("return " + stdout))();
        //console.log(dataset);
        if (error !== null) {
          console.log('exec error: ' + error);
        }
        console.log(source);
        callback();
      });
       },
       function (callback){
        console.log("compiled!");
        callback();
       },
       function (callback){
        socket.emit('greeting',source,function(data){});
        console.log('emmit to client!');
       }
    ])
    //console.log(source);
  });
   //fs.writeFileSync('hoge.json', source);
});

server.listen(8124);
 
console.log('Server running at http://127.0.0.1:8124/');
