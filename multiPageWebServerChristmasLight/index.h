
const char *HTML_CONTENT_HOME = R""""(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>Home Page</title>
</head>
<body>
    <h1>Domotica</h1>
        <font size="+5">
    <ul>
        <li><a href="/lamp">Lampen</a></li>
        <li><a href="/light">Licht meting</a></li>
        <li><a href="/temperature">Temperatuur, vochtigheid woonkamer</a></li>
        <li><a href="/battery?aan">Batterij opladen AAN</a></li>
        <li><a href="/battery?uit">Batterij opladen UIT</a></li>
    </ul>
    </font>
</body>
</html>
)"""";
