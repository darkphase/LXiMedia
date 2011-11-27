// Loads a player based on the window size.
function loadStretchedImage(id, imageUrl)
{
  var width = 630, height = 460;
  if (window.innerWidth && window.innerHeight) 
  {
    width = window.innerWidth;
    height = window.innerHeight;
  }
  
  document.getElementById(id).innerHTML = 
      "<img src=\"" + imageUrl + "&resolution=" + width + "x" + height + "\" alt=\"...\" />";
}
