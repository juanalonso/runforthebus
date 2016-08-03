<?php

  require 'config.php';

  //Bus stop ID
  $busStop = 5160;

  //Bus lines we are interested in
  $busLines = array(102, 152);


  //Time needed to get from the sofa to the bus stop
  $tmin = 260;

  //Amount of time we are willing to wait for the bus
  $tmax = $tmin + 60 * 5;

  //EMT timestamp when the bus is far away
  $NOBUS = 999999;

  //Max value for the digital pin.
  //255 for 5v
  //170 fro 3.3v
  //Less than 170 to allow some margin
  $maxVoltage = 120;


  /*

  Time arrow

                 Too late, wait for the next bus
                /
               /
              /     Get out now or you'll miss the bus
             /     /
            /     /
           /     /         You have some time left
          /     /         /
         /     /         /      
        /     /         /    The bus is far away, stay at home some more time
       /     /         /    /
  <---------+---------+---------<
  0        tmin      tmax     NOBUS

  */


  //Init the waiting time
  $currentWait = $NOBUS;

  //Connect to EMT servers and get the data for the bus stop
  $curl = curl_init();

  curl_setopt($curl, CURLOPT_URL, "https://openbus.emtmadrid.es:9443/emt-proxy-server/last/geo/GetArriveStop.php");
  curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
  curl_setopt($curl, CURLOPT_SSL_VERIFYPEER, false);
  curl_setopt($curl, CURLOPT_POST, 1);
  curl_setopt($curl, CURLOPT_POSTFIELDS, "idClient=$EMTUser&passKey=$EMTPass&idStop=$busStop");
  
  $result = curl_exec ($curl);

  //ERROR: no results, exit
  if ($result === false) {
    echo "{\"wait\":$NOBUS}\n";
    exit;
  }
  
  curl_close ($curl);

  //Convert to associative array
  $arrayResult = json_decode($result, true);

  //If there is no bus data, exit
  if (!isset($arrayResult["arrives"])) {
    echo "{\"wait\":$NOBUS}\n";
    exit;
  }

  //For each bus...
  foreach ($arrayResult["arrives"] as $key=>$arrival) {

    //...belonging to the bus lines we are interested in...
    if (in_array($arrival["lineId"],$busLines)) {

      //... remove some data
      unset($arrayResult["arrives"][$key]["isHead"]);
      unset($arrayResult["arrives"][$key]["destination"]);
      unset($arrayResult["arrives"][$key]["stopId"]);
      unset($arrayResult["arrives"][$key]["longitude"]);
      unset($arrayResult["arrives"][$key]["latitude"]);
      unset($arrayResult["arrives"][$key]["busPositionType"]);
      unset($arrayResult["arrives"][$key]["busDistance"]);

      //... and update the waiting time
      $busArrivesIn = $arrival["busTimeLeft"];
      if ($busArrivesIn >= $tmin && $busArrivesIn <= $tmax && $busArrivesIn<$currentWait) {
        $currentWait = $busArrivesIn;
      }

    } else {

      //Remove unwanted bus lines
      unset($arrayResult["arrives"][$key]);
    
    }
  }

  //Add the waiting time value and convert back to JSON
  $arrayResult["wait"] = returnValue($currentWait);
  print(json_encode($arrayResult));

  exit;

  //Scale the waiting time for arduino (0..255 and 999999)
  function returnValue($value) {

    global $tmin, $tmax, $NOBUS, $maxVoltage;

    if ($value == $NOBUS) {
      return $NOBUS;
    } 

    return floor($maxVoltage * ($value - $tmin) / ($tmax - $tmin));
  }


