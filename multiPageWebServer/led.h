/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
 */

const char *HTML_CONTENT_LED = R""""(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>LED Page</title>
</head>
<body>
    <h1>LED Page</h1>
    <font size="+5">
    <p>Licht is <span style="color: red;">LED_STATE_MARKER</span></p>
    <ul>
    <li><a href="/led?0">Licht uit</a></li>
    <li><a href="/led?1">Licht aan</a></li>
    <li><a href="/temperature">Temperatuur, vochtigheid woonkamer</a></li>
    </ul>
    </font>
</body>
</html>
)"""";
