const MongoDB  = require('mongodb');
const EventEmitter = require('events');
const MQTT     = require('mqtt');
const request = require('then-request');

class Service extends EventEmitter {
    constructor(config){
        super();

        this.config = config;

        this.mqttUri  = 'mqtt://' + config.mqtt.hostname + ':' + config.mqtt.port;
        this.mongoUri = 'mongodb://' + config.mongodb.hostname + ':' + config.mongodb.port + '/' + config.mongodb.database;

        this.rules = config.rules;

        this.on('mqtt-connect', function() { console.log('mqtt-connect');} )
            .on('mongo-connect', function() { console.log('mongo-connect');} )
            .on('mqtt-message', this.readRules )
            .on('mongo-rules', this.applyRules )
            .on('mongo-data', this.saveData )
            .on('mongo-inserted', function(data) { console.log('mongo-inserted ' + JSON.stringify(data));} )
    }

    saveData(data) {
        var self = this;

        if (this.mongoDatabase) {
            var insertOneDb = function (error1, result1) {
                if(error1 != null) {
                    console.error("ERROR: " + error1.toString());
                    self.emit('mongo-error', error1.toString());
                } else if (data.message !== null && data.message !== '') {

                    delete data.unique;

                    self.mongoCollection.insert(data, function(error, result) {
                        if(error != null) {
                            console.error("ERROR: " + error.toString());
                            self.emit('mongo-error', error.toString());
                        }
                    });

                    self.emit('mongo-inserted', data);
                }
            };

            if (data.unique) {
                self.mongoCollection.deleteMany({'topic': data.topic }, insertOneDb);
            } else {
                insertOneDb();
            }
        }

        return this;
    }


    applyRules(rules, data) {
        var self = this;

        if (this.mongoDatabase) {
            var handled = false;

            try{

                for (var cond of (rules  || []) ) {
                    var matches = data.topic.matchGetObject(cond.match || '', data);

                    if (matches) {
                        if (typeof cond.handled !== 'undefined')
                            handled |= cond.handled;

                        if (typeof cond.func  === 'string' ){
                            try{
                                cond.func = new Function('return ' + cond.func)();
                            } catch(x) {
                                delete cond.func;
                            }
                        }

                        if (typeof cond.func === 'function') {
                            try {
                                var res = cond.func.call(self, data, cond);

                                if (typeof res !== 'undefined')
                                    handled |= res;

                            } catch(x) {
                                //
                            }
                        }

                        data.unique = (typeof cond.unique !== 'undefined') && cond.unique;

                        if (cond.url) {

                            var url = cond.url.formatUsingObject(matches);

                            request('GET', url).done(function (res) {
                                if (res.statusCode < 300) {
                                    var gdata = res.getBody().toString('utf8');

                                    try {
                                        matches['data'] = JSON.parse(gdata);
                                    } catch (e) {
                                        matches['data'] = gdata;
                                    }

                                    var idata = {};

                                    for (var i in data)
                                        idata[i] = data[i];

                                    if (cond.formatmsg) {
                                        var message = cond.formatmsg.formatUsingObject(matches);

                                        try {
                                            idata.message = JSON.parse(message);
                                        } catch (e) {
                                            idata.message = message;
                                        }
                                    }

                                    if (cond.formattopic) {
                                        idata.topic = cond.formattopic.formatUsingObject(matches);
                                    }

                                    self.emit('mongo-data', idata);
                                }
                            });
                        }

                        if (cond.topic){
                            var mtopic = cond.topic.formatUsingObject(matches);

                            if (mtopic.startsWith('/') && mtopic.endsWith('/') && mtopic.length > 2)
                                mtopic = { $regex: mtopic.substr(1, mtopic.length - 2) };

                            self.mongoCollection.aggregate([
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
                                if(err != null) {
                                    console.error("ERROR: " + err.toString());
                                    self.emit('mongo-error', err.toString());
                                } else if (result) {
                                    result.forEach(function(x) {
                                        if (x.topic && x.message && self.client)
                                            self.client.publish(x.topic, x.message);
                                    })

                                }
                            });
                        }
                    }
                }
            } catch (error) {
                console.error("ERROR: " + error.toString());
                self.emit('mongo-error', error.toString());
            }
            if (!handled)
                self.emit('mongo-data', data);
        }

        return this;
    }

    readRules(data) {
        var self = this;

        if (this.mongoDatabase && this.config.mongodb.rulecollection) {
            this.mongoDatabase.collection(this.config.mongodb.rulecollection)
                .find()
                .toArray(function(err, docs) {
                    if (docs) self.rules = [].concat(docs, self.config.rules);

                    self.emit('mongo-rules', self.rules, data);
                });
        } else {
            self.emit('mongo-rules', self.rules, data);
        }

        return this;
    }


    start() {
        var self = this;

        this.client = MQTT.connect(this.mqttUri, { 'username' : this.config.mqtt.username,  'password': this.config.mqtt.password })
            .on('connect', function () {
                self.client.subscribe(self.config.mqtt.namespace);
                self.emit('mqtt-connect')
            }).on('message', function (topic, message) {
                try {
                    message = JSON.parse(message.toString());
                } catch (e) {
                    message = message.toString();
                }

                self.emit('mqtt-message', {
                    topic: topic,
                    message: message,
                    date: new Date()
                });
            }).on('error', function (error) {
                console.error("ERROR: " + error.toString());
                self.emit('mqtt-error', error.toString());
            });

        MongoDB.MongoClient.connect(this.mongoUri, function(error, database) {
            if(error != null) {
                console.error("ERROR: " + error.toString());
                self.emit('mongo-error', error.toString());
            } else {
                self.mongoDatabase = database;

                self.mongoCollection = database.collection(self.config.mongodb.collection);
                self.mongoCollection.createIndex( { "topic" : 1 } );
                self.mongoCollection.createIndex( { "date" : -1 } );

                self.emit('mongo-connect');
            }
        });

        return this;
    }
}

module.exports = Service;
