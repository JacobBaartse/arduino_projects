
const char *HTML_CONTENT_IFRAMES = R""""(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="icon" href="data:,">
    <title>Radio</title>
</head>
<body>

  <div style="display: flex; flex-direction: column; gap: 20px; width: 100%; max-width: 600px; margin: 0 auto;">
  
  <!-- Eerste iframe -->
  <div style="position: relative; width: 100%; padding-bottom: 10%; height: 0; overflow: hidden; border-radius: 8px;">
    <iframe 
      src="/radio" 
      title="Eerste ingesloten pagina" 
      style="position: absolute; top: 0; left: 0; width: 100%; height: 100%; border: none;" 
      loading="lazy" 
      allowfullscreen>
    </iframe>
  </div>

  <!-- Tweede iframe -->
  <div style="position: relative; width: 100%; padding-bottom: 200%; height: 0; overflow: hidden; border-radius: 8px;">
    <iframe 
      src="http://192.168.2.8" 
      title="Tweede ingesloten pagina" 
      style="position: absolute; top: 0; left: 0; width: 100%; height: 100%; border: none;" 
      loading="lazy" 
      allowfullscreen>
    </iframe>
  </div>

</div>
</body>
</html>)"""";
