// ==UserScript==
// @name          Gmail Auto Self BCC
// @namespace     http://www.kehlet.cx/
// @description   Automatically copy your From address into the bcc field (done upon hitting Send) so all mail you originate shows up in your Inbox.  For GMail v2.
// @include       http*://mail.google.com/*
// ==/UserScript==

window.addEventListener("load", loader, false);

function loader() {
   var api = typeof unsafeWindow != "undefined" && unsafeWindow.gmonkey ||
             (frames.js ? frames.js.gmonkey : null);
   if (api) api.load("1.0", init);
}

function init(gmail) {

   function findElement(root, name) {
      var elts = root.getElementsByName(name);
      return elts ? elts[0] : null;
   }

   function handleClicks(event) {
      var elt = event.target;
      if (elt.innerText == 'Send') {
         var fromElt = findElement(elt.ownerDocument, 'from');
         if (fromElt) {
            var bccElt = findElement(elt.ownerDocument, 'bcc');
            if (bccElt) {
               var re = new RegExp(fromElt.value);
               if (!bccElt.value.match(re)) {
                  bccElt.value = bccElt.value ? bccElt.value + ', ' + fromElt.value : fromElt.value;
               }
            }
         }
      }
   }
   
   function viewChanged() {
      var view = gmail.getActiveViewType();
      if (view == 'co' || view == 'cv') {
         var root = gmail.getNavPaneElement().ownerDocument;
         root.addEventListener('click', handleClicks, true);
      }
   }

   gmail.registerViewChangeCallback(viewChanged);
}
