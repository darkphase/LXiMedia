// Show and hide controls.
var controlsLocked, timer;

function showControls()
{
  if (!controlsLocked)
  {
    clearTimeout(timer);
    timer = setTimeout("hideControls()", 1000);
  
    document.getElementById("controls").style.visibility = "visible";
  }
}

function hideControls()
{
  if (!controlsLocked)
  {
    document.getElementById("controls").style.visibility = "hidden";
  }
}

function lockControls()
{
  clearTimeout(timer);
  controlsLocked = true;
  
  document.getElementById("controls").style.visibility = "visible";
}

function unlockControls()
{
  timer = setTimeout("hideControls()", 1000);
  controlsLocked = false;
}

// Player
var width, height, lastImageUrl;

function resizeWindow()
{
  width = 630;
  height = 460;
  
  if (window.innerWidth && window.innerHeight) 
  {
    width = window.innerWidth;
    height = window.innerHeight;
  }
  else if (document.body && document.body.offsetWidth) 
  {
    width = document.body.offsetWidth;
    height = document.body.offsetHeight;
  }
  
  var browser = document.getElementById("browser");
  if (browser)
  {
    browser.style.width = width + "px";
    browser.style.height = (height - document.getElementById("audioplayer").offsetHeight) + "px";
  }
  
  if (lastImageUrl)
    loadImage(lastImageUrl);
}

function closePlayer()
{
  top.location.href = document.getElementById("browser").contentWindow.location.href;
}

function loadThumbnailBar(path, start, count)
{
  var request = new XMLHttpRequest();
  request.onreadystatechange = function()
    {
      if ((request.readyState == 4) && (request.status == 200) &&
          (request.responseText.length > 0))
      {
        document.getElementById("items").innerHTML += request.responseText;
        if (count > 0)
          loadThumbnailBar(path, start + count, count);
      }
    }

  request.open("GET", path + "?items=" + start + "," + count + "&func=2,loadVideo,3,loadImage", true);
  request.send();
}

function loadAudio(audioUrl, title)
{
  lastImageUrl = false;
  
  if (top.location.href != window.location.href)
  {
    top.location.href = window.location.href;
  }
  else
  {
    document.getElementById("player").innerHTML = 
        "<audio autoplay=\"autoplay\" id=\"player\">" +
        "<source src=\"" + audioUrl + "?format=oga\" type=\"audio/ogg\" />" +
        "<source src=\"" + audioUrl + "?format=mp2\" type=\"audio/mpeg\" />" +
        "<source src=\"" + audioUrl + "?format=wav\" type=\"audio/wave\" />" +
        "</audio>" +
        title;

    resizeWindow();
  }
}

function loadVideo(videoUrl)
{
  lastImageUrl = false;
  
  document.getElementById("player").innerHTML = 
      "<video autoplay=\"autoplay\" id=\"player\">" +
      "<source src=\"" + videoUrl + "?format=ogv\" type=\"video/ogg\" />" +
      "<source src=\"" + videoUrl + "?format=mpeg\" type=\"video/mpeg\" />" +
      "</video>";
}

function loadImage(imageUrl)
{
  if (!lastImageUrl)
    resizeWindow();

  lastImageUrl = imageUrl;

  document.getElementById("player").innerHTML = 
      "<img src=\"" + imageUrl + "?format=jpeg&resolution=" + width + "x" + height + "&bgcolor=000000\" alt=\"...\" />";
}
