/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

var gateway = `ws://${window.location.hostname}:8008/ws`;
var websocket;
// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
  //  initButton();
}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
   // let data = JSON.parse(event.data);
  //  document.getElementById('led').className = data.status;

  console.log("data.status: " + event.data);

  switch (event.data){

    case "on":
        $('.thumbnailButtonItem.thumbnailSelected').removeClass('thumbnailSelected');
        $('#on').addClass('thumbnailSelected');
        break;
    case "off":
        $('.thumbnailButtonItem.thumbnailSelected').removeClass('thumbnailSelected');
        $('#off').addClass('thumbnailSelected');
        break;
    case "blink":
        $('.thumbnailButtonItem.thumbnailSelected').removeClass('thumbnailSelected');
        $('#blink').addClass('thumbnailSelected');
        break;
    case "loop-one":
            $('.thumbnailButtonItem.thumbnailSelected').removeClass('thumbnailSelected');
            $('#loop-one').addClass('thumbnailSelected');
            break;
    case "loop-two":
        $('.thumbnailButtonItem.thumbnailSelected').removeClass('thumbnailSelected');
        $('#loop-two').addClass('thumbnailSelected');
        break;
    default:
        $('.thumbnailButtonItem.thumbnailSelected').removeClass('thumbnailSelected');
        $('#on').addClass('thumbnailSelected');
        break;

  }

}

$(document).ready(function(){

    $(function() {
        $('.thumbnailButtonItem').on('click', function(e) {
            $('.thumbnailButtonItem.thumbnailSelected').removeClass('thumbnailSelected');
            $(this).addClass('thumbnailSelected');
            e.preventDefault();

            websocket.send($(this).data("action"));
        });
    });
});