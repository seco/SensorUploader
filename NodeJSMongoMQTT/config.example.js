var config = {};

config.debug = process.env.DEBUG || false;

config.mqtt  = {};
config.mqtt.namespace = process.env.MQTT_NAMESPACE || '#';
config.mqtt.hostname  = process.env.MQTT_HOSTNAME  || 'localhost';
config.mqtt.port      = process.env.MQTT_PORT      || 1883;
config.mqtt.username  = process.env.MQTT_USERNAME  || '';
config.mqtt.password  = process.env.MQTT_PASSWORD  || '';


config.mongodb = {};
config.mongodb.hostname   = process.env.MONGODB_HOSTNAME   || 'localhost';
config.mongodb.port       = process.env.MONGODB_PORT       || 27017;
config.mongodb.database   = process.env.MONGODB_DATABASE   || 'mqtt';
config.mongodb.collection = process.env.MONGODB_COLLECTION || 'message';

config.mongodb.ignorematch = process.env.MONGODB_IGNOREMATCH || '(^public)';


config.rules = [];


config.rules.push({
    'match': process.env.REPLSTATUS_MATCH_RESEND || 'sender\/status',
    'topic': process.env.REPLSTATUS_TOPIC || 'sender/switch',
    'handled' : true
});

config.rules.push({
    'match': process.env.REPLSTATUS_MATCH_UNIQUE || 'hu\/switch',
    'unique' : true
});

config.rules.push( {
    'match': process.env.REPLSTATUS_MATCH || 'sender\/(data)',
    'url': process.env.HTTPFORWARD_URL || 'https://maker.ifttt.com/trigger/{1}/with/key/blablabla?value1={message.temperature1}&value2={message.pressure1}&value3=&#7b;date&#7d;';,
    'handled' : true
});

module.exports = config;
