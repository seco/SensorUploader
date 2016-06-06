<html>
	<head>
		<link href="//fonts.googleapis.com/css?family=Roboto+Slab|Roboto+Mono|Roboto+Condensed|Roboto" rel="stylesheet" type="text/css"></link>
		<link href="/css/template.css" rel="stylesheet" type="text/css"></link>

		<script src="//ajax.googleapis.com/ajax/libs/jquery/2.2.4/jquery.min.js"></script>
		<script src="//code.highcharts.com/highcharts.js"></script>
	</head>
<body>
	<div id="container" style="width:800px; height:400px;"></div>

<script type="text/javascript">		
<?php


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

$i = 0;

foreach($res as $value) {
	$xvalue = (array)$value;
	
	$xkey = (array)($xvalue['_id']);
	
	foreach($data as $key => $val) {
		$vl = $xvalue[$key];
		if ($vl == 0)
			$vl = null;
		array_push($data[$key], round($vl * 100) / 100);
	}
	
	array_push($categories, $xkey['y'].'-'.$xkey['m'].'-'.$xkey['d'].' '.$xkey['h']);
	
	$i++;
}
?>
	
var data = <?=(json_encode($data)); ?>;

var xdata = [];

for(var k in data) {
	xdata.push({
            name: k,
            data: data[k]
        });
}


$(function () { 
    $('#container').highcharts({
        chart: {
            type: 'line'
        },
        title: {
            text: 'Weather'
        },
        xAxis: {
            categories: <?=(json_encode($categories)); ?>
        },
        yAxis: {
        },
        series: xdata
    });
});
 
</script>
</body>