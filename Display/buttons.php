<?php
@include_once('inc/auth.inc');

@include_once('inc/config.inc');

if ($_SERVER['REQUEST_METHOD'] === 'GET') {

    @require_once('inc/head.inc');


    $manager = new MongoDB\Driver\Manager($config["mongo"]["url"]);

    $cursor = $manager->executeCommand('mqtt', new MongoDB\Driver\Command(['aggregate' => 'message', 'pipeline' => [
        [ '$match' => ['topic' => ['$regex' => '.*/switch']]],
        [ '$project' =>  [
           'date' =>  '$date',
           'message'  => '$message',
           'topic' =>  '$topic']
       ],
       [ '$sort'  =>  [ 'date'  =>  -1 ] ],
       [ '$group' => [
           '_id' =>  '$topic',
           'date' => [ '$first' =>  '$date' ],
           'message' => [ '$first' =>  '$message' ],
           'topic' => [ '$first' =>  '$topic' ],
       ]],


    ]]));

    $res = (array)($cursor->toArray()[0]);
    $res = (array)($res['result']);

    foreach ($res as $value) {
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
