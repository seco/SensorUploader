// Create a client instance
client = new Paho.MQTT.Client("<>", 8881, "clientId");

// set callback handlers
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

// connect the client
client.connect({onSuccess:onConnect, useSSL: true, userName:"<>", password:"<>" });


// called when the client connects
function onConnect() {
  // Once a connection has been made, make a subscription and send a message.
  alert("onConnect");
  client.subscribe("public/data");
  message = new Paho.MQTT.Message("Hello");
  message.destinationName = "public/data";
  client.send(message);
}

// called when the client loses its connection
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    alert("onConnectionLost:"+responseObject.errorMessage);
  }
}

// called when a message arrives
function onMessageArrived(message) {
  alert("onMessageArrived:"+message.payloadString);
}