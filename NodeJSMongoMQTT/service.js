var mongodb  = require('mongodb');
var mqtt     = require('mqtt');
var request = require('then-request');

class Service extends EventEmitter {
    constructor(length) (config){
        this.config = config;

        this.mqttUri  = 'mqtt://' + config.mqtt.hostname + ':' + config.mqtt.port;
        this.mongoUri = 'mongodb://' + config.mongodb.hostname + ':' + config.mongodb.port + '/' + config.mongodb.database;

        this.client = mqtt.connect(mqttUri, { 'username' : config.mqtt.username,  'password': config.mqtt.password });
    }
}
/*
var service = function Service(config){
    this.config = config;

    var mqttUri  = 'mqtt://' + config.mqtt.hostname + ':' + config.mqtt.port;
    var mongoUri = 'mongodb://' + config.mongodb.hostname + ':' + config.mongodb.port + '/' + config.mongodb.database;

    this.client = mqtt.connect(mqttUri, { 'username' : config.mqtt.username,  'password': config.mqtt.password });
}


    client.on('connect', function () {
        client.subscribe(config.mqtt.namespace);
    });

    var mongoClient = this.mongoClient = mongodb.MongoClient;

    mongoClient.connect(mongoUri,
};
*/

modile.exports = Service;
