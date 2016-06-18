/**
 *
 * This NodeJS application listens to MQTT messages and records them to MongoDB
 *
 * @author  Dennis de Greef <github@link0.net>, Daniil Bystrukhin <dannsk@mail.ru>
 * @license MIT
 *
 */

require('utils');

var mongodb  = require('mongodb');
var mqtt     = require('mqtt');
var config   = require('./config');

var https = require('https');
var http = require('http');



var mqttUri  = 'mqtt://' + config.mqtt.hostname + ':' + config.mqtt.port;
var client   = mqtt.connect(mqttUri, { 'username' : config.mqtt.username,  'password': config.mqtt.password });
var mongoIgnore = config.mongodb.ignorematch ? new RegExp(config.mongodb.ignorematch) : null;
var mongoUri = 'mongodb://' + config.mongodb.hostname + ':' + config.mongodb.port + '/' + config.mongodb.database;

client.on('connect', function () {
    client.subscribe(config.mqtt.namespace);
});

mongodb.MongoClient.connect(mongoUri, function(error, database) {
    if(error != null) {
        console.error("ERROR: " + error.toString());
        return;
    }

    //console.log('connected');
    var collection = database.collection(config.mongodb.collection);
    collection.createIndex( { "topic" : 1 } );
    collection.createIndex( { "date" : -1 } );


    client.on('message', function (topic, message) {
        //console.log('message ' + topic);
        var messageObject = {
            topic: topic,
            message: message.toString(),
            date: new Date()
        };

        try {
            messageObject.message = JSON.parse(message.toString());
        } catch (e) {
            messageObject.message = message.toString();
        }

        try {
            if (config.replstatus && config.replstatus.length) {
                for (var cond of config.replstatus ) {
                    var matches = topic.matchGetObject(cond.match || '', messageObject);
                    if (matches && cond.topic) {
                        var mtopic = cond.topic.formatUsingObject(matches);

                        if (mtopic.startsWith('/') && mtopic.endsWith('/') && mtopic.length > 2)
                            mtopic = { $regex: mtopic.substr(1, mtopic.length - 2) };

                        collection.aggregate([
                            { $match: { topic: mtopic }},
                            { $sort: { date: -1 } },
                            {
                                $group:
                                {
                                    _id: "$topic",
                                    topic: { $first: "$topic" },
                                    message: { $first: "$message" }
                                }
                            }
                        ]).toArray(function(err, result) {
                            if (result) {
                                result.forEach(function(x) {
                                    if (x.topic && x.message)
                                        client.publish(x.topic, x.message);
                                })

                            }
                        });
                    }
                }
            }
        } catch(e) {
            console.error(e);
        }

        try {
            if (config.httpforward && config.httpforward.length) {
                for (var cond of config.httpforward ) {
                    var matches = topic.matchGetObject(cond.match || '', messageObject);

                    if (matches && cond.url) {
                        var url = cond.url.formatUsingObject(matches);

                        if (url.startsWith('https://'))
                            https.get(url);
                        else if (url.startsWith('http://'))
                            http.get(url);
                    }
                }
            }
        } catch(e) {
            console.error(e);
        }
        if (!mongoIgnore || !mongoIgnore.test(topic)) {
            collection.insert(messageObject, function(error, result) {
                if(error != null) {
                    console.error("ERROR: " + error.toString());
                }
            });
        }
    });
});
