<?php
@include_once('inc/auth.inc');

@include_once('inc/config.inc');

if ($_SERVER['REQUEST_METHOD'] === 'GET') {

    @require_once('inc/head.inc');


    $manager = new MongoDB\Driver\Manager($config["mongo"]["url"]);
    $topic = $config["mongo"]["topic"];
    if (!isset($topic))
        $topic = ['$regex' => '.*\/switch'];

    $filter =  ['topic' => $topic];
    $options = ['projection' => ['topic' => '$topic', 'message' => '$message' ],'sort' => ['topic' => 1]];

    $query = new MongoDB\Driver\Query($filter, $options);
    $cursor = $manager->executeQuery('mqtt.message', $query);

    foreach ($cursor as $value) {
        $id = str_replace('/', '_', $value->topic);
?>
<div>
    <label style="display:inline-block;" class="sliderTextLabel" for="<?=$id?>"><?=$value->topic?></label>
    <label style="display:inline-block;" class="sliderLabel">
        <input type="checkbox" id="<?=$id?>" onclick="$.ajax({type: 'POST', url: '<?=$_SERVER["SCRIPT_NAME"]?>', data: { id : '<?=$value->topic?>', checked: this.checked }});" <?=($value->message==="on" ? "checked" : "")?>/>
        <span class="slider">
            <span class="sliderOn">ON</span>
            <span class="sliderOff">OFF</span>
            <span class="sliderBlock"></span>
        </span>
    </label>
</div>
<?php
    }
    @require_once('inc/foot.inc');
} else {
    $c = new Mosquitto\Client;
    $c->onConnect(function() use ($c) {
        $c->publish($_POST["id"], $_POST["checked"] === "true" ? "on" : "off");
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
        //var_dump($x);
    }

}?>
