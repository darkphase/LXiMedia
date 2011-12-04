function createScriptArea(id)
{
  var width = 630, height = 460;
  if (window.innerWidth && window.innerHeight)
  {
    width = window.innerWidth;
    height = window.innerHeight;
  }

  var lnb = document.createElement("TEXTAREA");
  var txt = document.getElementById(id);
  var string = '';

  for (var no=1;no<300;no++)
  {
    if (string.length>0) string += '\n';
    string += no;
  }

  var lnwidth = 25, tbheight = 25;

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
    lnb.style.left  = (txt.offsetLeft - 27) + "px";
  }

  setLine();
  txt.focus();
  txt.onkeydown    = function() { setLine(); }
  txt.onmousedown  = function() { setLine(); }
  txt.onscroll     = function() { setLine(); }
  txt.onblur       = function() { setLine(); }
  txt.onfocus      = function() { setLine(); }
  txt.onmousewheel = function() { setLine(); }
  txt.onmouseover  = function() { setLine(); }
  txt.onmouseup    = function() { setLine(); }
}
