var ws;
initiateWebSocketConnection();
function initiateWebSocketConnection() {
	ws = new WebSocket('ws://' + window.location.hostname + ':81/');

	ws.onclose = function () {
		initiateWebSocketConnection()
	}

	ws.onerror = function () {
		var body = document.getElementsByTagName('body')[0];
		body.style.background = '#ff0000';
	}

	ws.onopen = function () {
		var body = document.getElementsByTagName('body')[0];
		body.style.background = 'black';
	}

	ws.onmessage = (event) => {
		const resp = event.data.toString();
		console.log("<== " + resp);
	};
}

function wsSend(msg) {
	console.log("==> " + msg);
	ws.send(msg);
}

function exec(obj, val) {
	const cmd = obj.id + (typeof(val) == 'undefined' ?"":"=" + val);
	wsSend(cmd);
}

function levels(obj, val) {
	wsSend("DisableADC=1")
	DisableADC.state = true;
	DisableADC.style.background = '#00a000';
	CutLevels.state = false;
	CutLevels.style.background = '';
	exec(obj, val);
}

var LevelA = document.getElementById('LevelA');
var LevelB = document.getElementById('LevelB');
var AdjustLevel = document.getElementById('AdjustLevel');
var DisableADC = document.getElementById('DisableADC');
var CutLevels = document.getElementById('CutLevels');

buttonsClickHandler();
function buttonsClickHandler() {
	var modeButtons = document.getElementsByClassName('modeButton');
	for(var x = 0; x < modeButtons.length; x++) {
		modeButtons[x].onclick = function() {
			for(var g = 0; g < modeButtons.length; g++) {
				modeButtons[g].style.background = '';
			}
			var send = 'Mode=' + this.getAttribute('name');
			wsSend(send);
			this.style.background = '#00a000';
			currentEffect.innerText = this.innerText;
		}
	}

	var buttons = document.getElementsByClassName('clickButton');
	for(var x = 0; x < buttons.length; x++) {
		buttons[x].onclick = function() {
			send = this.getAttribute('name');
			wsSend(send);
			const myTimeout = setTimeout(function(button) {button.style.background = '';}, 2000, this);
			this.style.background = '#00a000';
		}
	}

	var buttons = document.getElementsByClassName('toggleButton');
	for(var x = 0; x < buttons.length; x++) {
		buttons[x].state = false;
	}

	DisableADC.onclick = function() {
		this.state = !this.state;
		msg = 'DisableADC=' + (this.state?'1':'0')

		if (this.state == true) {
			wsSend(msg);
			wsSend("LevelA="+LevelA.value);
			wsSend("LevelB="+LevelB.value);
		}
		else {
			wsSend(msg);
		}
		CutLevels.state = false;
		CutLevels.style.background = '';
		this.style.background = this.state?'#00a000':'';
	}

	CutLevels.onclick = function() {
		this.state = !this.state;
		msg = 'CutLevels=' + (this.state?'1':'0')

		if (this.state == false) {
			if (DisableADC.state == true) {
				wsSend("LevelA="+LevelA.value);
				wsSend("LevelB="+LevelB.value);
			}
			else {
				wsSend(msg);
			}
		}
		else {
			wsSend(msg);
		}
		this.style.background = this.state?'#a00000':'';
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
