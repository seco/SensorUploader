/**
 * This NodeJS application listens to MQTT messages and records them to MongoDB
 * @author  Dennis de Greef <github@link0.net>, Daniil Bystrukhin <dannsk@mail.ru>
 * @license MIT
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

require('./utils');
var config   = require('./config');

var service  = new (require('./service'))(config);

console.log(service);

/*

mongodb.MongoClient.connect(mongoUri, function(error, database) {
    if(error != null) {
        console.error("ERROR: " + error.toString());
        return;
    }

    var collection = database.collection(config.mongodb.collection);
    collection.createIndex( { "topic" : 1 } );
    collection.createIndex( { "date" : -1 } );

    client.on('message', function (topic, message) {
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

        var insertDb = function(messageObject) {

            if (!mongoIgnore || !mongoIgnore.test(messageObject.topic)) {
                var insertOneDb = function (error1, result1) {
                    if(error1 != null) {
                        console.error("ERROR: " + error1.toString());
                    } else if (messageObject.message !== null && messageObject.message !== '') {

                        delete messageObject.unique;

                        collection.insert(messageObject, function(error, result) {
                            if(error != null) {
                                console.error("ERROR: " + error.toString());
                            }
                        });
                    }
                };

                if (messageObject.unique) {
                    collection.deleteMany({'topic': messageObject.topic}, insertOneDb);
                } else {
                    insertOneDb();
                }
            }
        };

        try {
            var handled = false;

            if (config.rules && config.rules.length) {
                for (var cond of config.rules ) {

                    var matches = topic.matchGetObject(cond.match || '', messageObject);
                    if (matches) {
                        if (typeof cond.handled !== 'undefined')
                            handled |= cond.handled;

                        messageObject.unique = (typeof cond.unique !== 'undefined') && cond.unique;

                        if (cond.url) {

                            var url = cond.url.formatUsingObject(matches);

                            request('GET', url).done(function (res) {
                                if (res.statusCode < 300) {
                                    var data = res.getBody().toString('utf8');

                                    try {
                                        matches['data'] = JSON.parse(data);
                                    } catch (e) {
                                        matches['data'] = data;
                                    }

                                    var intMessageObject = {};

                                    for (var i in messageObject)
                                        intMessageObject[i] = messageObject[i];

                                    if (cond.formatmsg) {
                                        message = cond.formatmsg.formatUsingObject(matches);

                                        try {
                                            intMessageObject.message = JSON.parse(message.toString());
                                        } catch (e) {
                                            intMessageObject.message = message.toString();
                                        }
                                    }

                                    if (cond.formattopic) {
                                        intMessageObject.topic = cond.formattopic.formatUsingObject(matches);
                                    }

                                    insertDb(intMessageObject);
                                }
                            });
                        }

                        if (cond.topic){
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
            }

            if (!handled)
                insertDb(messageObject);
        } catch(e) {
            console.error(e);
        }
    });
});
*/
