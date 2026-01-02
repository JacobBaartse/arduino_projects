
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
    <font size="+1">
    <p>Licht is:</br><span style="color: red;">LAMP_STATE_MARKER</span></p>
    <ul>
    <li><a href="/lamp?1aan">1 aan</a> <a href="/lamp?1uit"> uit</a></li>
    <li><a href="/lamp?2aan">2 aan</a> <a href="/lamp?2uit"> uit</a></li>
    <li><a href="/lamp?3aan">3 aan</a> <a href="/lamp?3uit"> uit</a></li>
    </br>
    <li><a href="/lamp?4aan">4 aan</a> <a href="/lamp?4uit"> uit</a></li>
    <li><a href="/lamp?5aan">5 aan</a> <a href="/lamp?5uit"> uit</a></li>
    <li><a href="/lamp?6aan">6 aan</a> <a href="/lamp?6uit"> uit</a></li>
    </br>
    <li><a href="/lamp?7aan">7 aan</a> <a href="/lamp?7uit"> uit</a></li>
    <li><a href="/lamp?8aan">8 aan</a> <a href="/lamp?8uit"> uit</a></li>
    <li><a href="/lamp?9aan">9 aan</a> <a href="/lamp?9uit"> uit</a></li>
    </br>
    <li><a href="/lamp?0aan">0 aan</a> <a href="/lamp?0uit"> uit</a></li>
    </br>
    <li><a href="/lamp?D">Alles aan</a> <a href="/lamp?E"> uit</a></li>
    <li><a href="/">Home</a></li>
</br>
</br>
</br>
</br>
    <li><a href="/lamp?W">wis programmering</a></li>
    </ul>
    </font>
</body>
</html>
)"""";
