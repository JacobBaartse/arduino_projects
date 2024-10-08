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
    <li><a href="/led?1">Licht aan</a> <a href="/led?2"> uit</a></li>
    <li><a href="/led?3">Licht aan</a> <a href="/led?4"> uit</a></li>
    <li><a href="/led?5">Licht aan</a> <a href="/led?6"> uit</a></li>
    <li><a href="/led?7">Alles uit</a></li>
    <li><a href="/">Home</a></li>
    </ul>
    </font>
</body>
</html>
)"""";
