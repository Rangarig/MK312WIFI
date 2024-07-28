var Socket;
if(window.location.protocol.startsWith("file"))
	Socket = new WebSocket('ws://' + '192.168.0.190'+ ':81/');
else {
	Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
}

Socket.onmessage = (event) => {
	const resp = event.data.toString();
	console.log("<== " + resp);
};

function exec(obj, val) {
	const cmd = obj.id + (typeof(val) == 'undefined' ?"":"=" + val);
	console.log("==> " + cmd);
	Socket.send(cmd);
}

function levels(obj, val) {
	DisableADC.checked=true;
	Socket.send("DisableADC=1");
	exec(obj, val);
}

var LevelA = document.getElementById('LevelA');
var LevelB = document.getElementById('LevelB');
var AdjustLevel = document.getElementById('AdjustLevel');
var DisableADC = document.getElementById('DisableADC');

modeClickHandler();
function modeClickHandler() {
	var span = document.getElementsByTagName('span');

	for(var x = 0; x < span.length; x++) {
		span[x].onclick = function() {

			for(var g = 0; g < span.length; g++) {
				span[g].style.background = '';
			}
			this.style.background = '#00a000';

			currentEffect.innerText = this.innerText;

			var send = 'Mode=' + this.getAttribute('name');
		    console.log(send);
		    Socket.send(send);
		}
	}
}

function displayHelp() {
	help.style.display = 'block';
	interrog.style.display = 'none';
}

function closeHelp() {
	help.style.display = 'none';
	interrog.style.display = 'block';
}
