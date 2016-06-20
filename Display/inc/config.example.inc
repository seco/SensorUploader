<?php

$config = [
    "sealevel" => 250,//meters
    "mongo" => [ 
        "url" => "mongodb://<>",
        "topic" => [ 
            "switch" => "<>",
            "data" => "<>"
        ]
    
    ],
    "mqtt" => [
        "host" => "<>",
        "port" => "<>",
        "user" => "<>",
        "pass" => "<>",
        "key" => "<>" //here goes the key to post postmsg.php
    ]
];

?>
