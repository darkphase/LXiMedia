var txt, msg, path, timer;

function createScriptArea(scriptid, messageid, serverpath)
{
  txt = document.getElementById(scriptid);
  msg = document.getElementById(messageid);
  path = serverpath;

  var width = 630, height = 460;
  if (window.innerWidth && window.innerHeight)
  {
    width = window.innerWidth;
    height = window.innerHeight;
  }

  var lnb = document.createElement("TEXTAREA");

  var string = "";
  for (var i=1; i<9999; i++)
  {
    if (string.length > 0) string += '\n';
    string += i;
  }

  var lnwidth = 30, tbheight = 25;

  lnb.className             = "linenumbers";
  lnb.style.width           = lnwidth + "px";
  lnb.style.height          = (height - (tbheight + 8)) + "px";
  lnb.style.position        = "absolute";
  lnb.style.overflow        = "hidden";
  lnb.style.textAlign       = "right";
  lnb.style.paddingRight    = "0.2em";
  lnb.style.zIndex          = 0;
  lnb.innerHTML             = string;
  lnb.innerText             = string;
  
  txt.style.zIndex          = 1;
  txt.style.position        = "relative";
  txt.style.marginLeft      = (lnwidth + 2) + "px";
  txt.style.width           = (width - (lnwidth + 4)) + "px";
  txt.style.height          = (height - (tbheight + 6)) + "px";
  txt.parentNode.insertBefore(lnb, txt.nextSibling);

  function setLine()
  {
    lnb.scrollTop   = txt.scrollTop;
    lnb.style.top   = (txt.offsetTop) + "px";
    lnb.style.left  = (txt.offsetLeft - (lnwidth + 2)) + "px";
  }

  setLine();
  txt.focus();
  txt.onkeydown    = function() { setLine(); clearTimeout(timer); timer=setTimeout("parseCode()", 1000); }
  txt.onmousedown  = function() { setLine(); }
  txt.onscroll     = function() { setLine(); }
  txt.onblur       = function() { setLine(); }
  txt.onfocus      = function() { setLine(); }
  txt.onmousewheel = function() { setLine(); }
  txt.onmouseover  = function() { setLine(); }
  txt.onmouseup    = function() { setLine(); }
}

function parseCode()
{
  var request = new XMLHttpRequest();
  request.onreadystatechange = function()
  {
    if ((request.readyState == 4) && (request.status == 200))
      msg.innerHTML = request.responseText;
  }

  var code = txt.innerText;
  request.open("POST", path + "?parse=", true);
  request.setRequestHeader("Content-length", code.length);
  request.send(code);
}

function closeWindow()
{
  window.opener="dummy"; // Prevents warning.
  window.close();
}
