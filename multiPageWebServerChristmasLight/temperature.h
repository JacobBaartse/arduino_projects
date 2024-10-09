
const char *HTML_CONTENT_TEMPERATURE = R""""(<!DOCTYPE html>
<html>
<head>
<title>Woonkamer</title>
<meta name="viewport" content="width=device-width, initial-scale=0.7, maximum-scale=0.7">
<meta charset="utf-8">
<link rel="icon" href="https://diyables.io/images/page/diyables.svg">
<style>
body { font-family: "Georgia"; text-align: center; font-size: width/2pt;}
h1 { font-weight: bold; font-size: width/2pt;}
h2 { font-weight: bold; font-size: width/2pt;}
button { font-weight: bold; font-size: width/2pt;}
</style>
<script>
var cvs_width = 220, cvs_height = 550;

function init() {
  var canvas = document.getElementById("cvs");
  canvas.width = cvs_width;
  canvas.height = cvs_height + 50;

  var ctx = canvas.getContext("2d");

  ctx.translate(cvs_width/2, cvs_height - 80);

  update_view(TEMPERATURE_HUMID_MARKER);
}

function update_view(temp, humid) {
  var canvas = document.getElementById("cvs");
  var ctx = canvas.getContext("2d");

  var radius = 70;
  var offset = 5;
  var width = 45;
  var height = 330;

  ctx.clearRect(-cvs_width/2, -350, cvs_width, cvs_height);
  ctx.strokeStyle="blue";
  ctx.fillStyle="blue";

  //2-step Degree
  var x = -width/2;
  ctx.lineWidth=2;
  for (var i = 0; i <= 50; i+=2) {
    var y = -(height - radius)*i/50 - radius - 5;
    ctx.beginPath();
    ctx.lineTo(x, y);
    ctx.lineTo(x - 20, y);
    ctx.stroke();
  }

  //10-step Degree
  ctx.lineWidth=5;
  for (var i = 0; i <= 50; i+=10) {
    var y = -(height - radius)*i/50 - radius - 5;
    ctx.beginPath();
    ctx.lineTo(x, y);
    ctx.lineTo(x - 25, y);
    ctx.stroke();

    ctx.font="20px Georgia";
    ctx.textBaseline="middle";
    ctx.textAlign="right";
    ctx.fillText(i.toString(), x - 35, y);
  }

  // shape
  ctx.lineWidth=16;
  ctx.beginPath();
  ctx.arc(0, 0, radius, 0, 2 * Math.PI);
  ctx.stroke();

  ctx.beginPath();
  ctx.rect(-width/2, -height, width, height);
  ctx.stroke();

  ctx.beginPath();
  ctx.arc(0, -height, width/2, 0, 2 * Math.PI);
  ctx.stroke();

  ctx.fillStyle="#e6e6ff";
  ctx.beginPath();
  ctx.arc(0, 0, radius, 0, 2 * Math.PI);
  ctx.fill();

  ctx.beginPath();
  ctx.rect(-width/2, -height, width, height);
  ctx.fill();

  ctx.beginPath();
  ctx.arc(0, -height, width/2, 0, 2 * Math.PI);
  ctx.fill();
  ctx.fillStyle="#ff1a1a";

  ctx.beginPath();
  ctx.arc(0, 0, radius - offset, 0, 2 * Math.PI);
  ctx.fill();

  temp = Math.round(temp * 100) / 100;
  var y = (height - radius)*temp/50.0 + radius + 5;
  ctx.beginPath();
  ctx.rect(-width/2 + offset, -y, width - 2*offset, y);
  ctx.fill();

  ctx.fillStyle="red";
  ctx.font="bold 34px Georgia";
  ctx.textBaseline="middle";
  ctx.textAlign="center";
  ctx.fillText(temp.toString() + "°C", 0, 100);
  ctx.fillText("Vochtigheid:", 0, -420);
  ctx.fillText(humid.toString() + "%", 0, -390);
}

window.onload = init;
</script>
</head>

<body>
<h1>Woonkamer</h1>
        <font size="+5">
<ul>
        <li><a href="/">Home</a></li>
</ul>
<canvas id="cvs"></canvas>
</body>
</html>)"""";
