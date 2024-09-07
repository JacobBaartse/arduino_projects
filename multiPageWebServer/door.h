/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server-multiple-pages
 */

const char *HTML_CONTENT_DOOR = R""""(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>Door Page</title>
</head>
<body>
    <h1>Door Page</h1>
    <p>Door State: <span style="color: red;">DOOR_STATE_MARKER</span></p>
</body>
</html>
)"""";
