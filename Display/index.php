<html>
    <head>
        <link href="//fonts.googleapis.com/css?family=Roboto+Slab|Roboto+Mono|Roboto+Condensed|Roboto" rel="stylesheet" type="text/css"></link>
        <link href="/css/template.css" rel="stylesheet" type="text/css"></link>

        <script src="//ajax.googleapis.com/ajax/libs/jquery/2.2.4/jquery.min.js"></script>
        <script src="//code.highcharts.com/highcharts.js"></script>
    </head>
<body>
    <div id="container" style="width:1000px; height:600px;margin: 0 auto;"></div>

<script type="text/javascript">
<?php

const OVERSEALEVEL = 300;//meters

$manager = new MongoDB\Driver\Manager('mongodb://localhost:27017');



$mongo_date = new MongoDB\BSON\UTCDateTime((new DateTime())->getTimestamp() * 1000 - 1000 * 60 * 60 * 24 * 7);


$cursor = $manager->executeCommand('mqtt', new MongoDB\Driver\Command(['aggregate' => 'message', 'pipeline' => [
    [ '$match' => ['topic' => 'hu/data', 'date' => ['$gte' => $mongo_date ]]],
    [ '$project' =>  [
       'y' => ['$year' => '$date'],
       'm' => ['$month' => '$date'],
       'd' => ['$dayOfMonth' => '$date'],
       'h' => ['$hour' => '$date'],
       'date' =>  '$date',
       'message'  => '$message',
       'topic' =>  '$topic']
   ],
    [ '$group' => [
       '_id' =>  [ 'y' => '$y','m' => '$m','d' => '$d','h' => '$h'],
       'total' => [ '$sum' =>  1 ],
       'date' => [ '$max' =>  '$date' ],
       'temperature1' => [ '$avg' =>  '$message.temperature1' ],
       'pressure1' => [ '$avg' =>  '$message.pressure1' ],
   ]],
   [ '$sort'  =>  [ 'date'  =>  -1 ] ],

]]));

$res = (array)($cursor->toArray()[0]);
$res = (array)($res['result']);

$data = ['temperature1' => [], 'pressure1' => []];

$categories = [];


foreach($res as $value) {
    $xvalue = (array)$value;

    $xkey = (array)($xvalue['_id']);

    foreach($data as $key => $val) {
        $vl = $xvalue[$key];
        if ($vl == 0) {
            $vl = null;
        }
        else {
            if (strrpos($key, 'pressure', -strlen($key)) !== false)
                $vl = ($vl / pow(1.0-OVERSEALEVEL/44330.0, 5.255));
            $vl = round($vl * 100) / 100;
        }
        array_push($data[$key], $vl);
    }

        $date = new DateTime();
        $date->setDate($xkey['y'], $xkey['m'], $xkey['d']);
        $date->setTime($xkey['h'], 0, 0);
        if (date('Z') > 0)
            $date->add(new DateInterval('PT'.abs(date('Z')).'S'));
        else
            $date->sub(new DateInterval('PT'.abs(date('Z')).'S'));

    array_push($categories, $date->format('Y-m-d H:00:00') );

}
?>

var data = <?=(json_encode($data)); ?>;

var xdata = [];
var ydata = [];


for(var k in data) {
    data[k].reverse();
    xdata.push({
            name: k,
            data: data[k],
            yAxis: ydata.length,
            type: 'spline'
        });
    ydata.push({
            labels: {
                style: {
                    color: Highcharts.getOptions().colors[ydata.length],
                    'font-weight':'bold'
                },
            },
            title: {
                text: null
            }
        });

}

var categories = <?=(json_encode($categories)); ?>;
categories.reverse();

$(function () {
    $('#container').highcharts({
        chart: {
            type: 'line'
        },
        title: {
            text: 'Weather'
        },
        tooltip: {
            shared: true
        },
        xAxis: {
            categories: categories
        },
        yAxis: ydata,
        series: xdata
    });
});

</script>
</body>
