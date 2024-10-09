
const char *HTML_CONTENT_LAMP = R""""(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>LAMP Page</title>
</head>
<body>
    <h1>LAMP Page</h1>
    <font size="+5">
    <p>Licht is:</br><span style="color: red;">LAMP_STATE_MARKER</span></p>
    <ul>
    <li><a href="/lamp?1">Licht aan</a> <a href="/lamp?2"> uit</a></li>
    <li><a href="/lamp?3">Licht aan</a> <a href="/lamp?4"> uit</a></li>
    <li><a href="/lamp?5">Licht aan</a> <a href="/lamp?6"> uit</a></li>
    <li><a href="/lamp?7">Alles aan</a> <a href="/lamp?8"> uit</a></li>
    <li><a href="/">Home</a></li>
    </ul>
    </font>
</body>
</html>
)"""";
