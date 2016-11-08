var qjson;

function load() {
	var xhr = new XMLHttpRequest();

	xhr.open("GET", "http://localhost/qjson", true);
	xhr.onreadystatechange = function() {
		if (xhr.readyState == xhr.DONE) {
			alert(xhr.responseXML);
			//create_problems();
		}
    }
	xhr.send();
}

function create_problems() {
	var count = new Array();
	var sign = 0;
	var problems = document.getElementById('ossqa');

	while (count.length < 25) {
		sign = 0;
		var index = Math.round(Math.random() * 1000) % 311;
		for (var i = 0; i < count.length; ++i) {
			if (count[i] === index)
				sign = 1;
		}
		if (sign == 0) {
			count.push(index);
		}
	}
}

function test() {
	alert(1);
}
