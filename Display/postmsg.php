<?php

@include_once('inc/config.inc');

$key = @$_REQUEST["key"];
$topic = @$_REQUEST["topic"];
$message = @$_REQUEST["message"];


if ($_SERVER['REQUEST_METHOD'] === 'POST' && $key == $config["mqtt"]["key"] && isset($topic) && isset($message)) {

    $c = new Mosquitto\Client;
    $c->onConnect(function() use ($c, $topic, $message) {
        $c->publish($topic, $message);
    });
    $c->setCredentials($config["mqtt"]["user"], $config["mqtt"]["pass"]);
    $c->connect($config["mqtt"]["host"], $config["mqtt"]["port"]);
    try{
    for ($i = 0; $i < 100; $i++) {
        // Loop around to permit the library to do its work
        $c->loop(1);

        sleep(5);
    }} catch (Mosquitto\Exception $x)
    {
        //
    }
    echo("OK");
} else {
   header($_SERVER["SERVER_PROTOCOL"]." 404 Not Found", true, 404);
   echo("Method Not Found");
}?>
