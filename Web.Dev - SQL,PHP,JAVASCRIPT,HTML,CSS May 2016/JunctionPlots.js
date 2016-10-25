//this is where I genrates graphs from data provided by the server, I used google chart to display the results. 


//handle clicled of junction
function junctionClickedHanlder(junctionId, samples_junctions,boxPlotId,distrubutionId,ratioPlot1,ratioPlot2){ //when junction is clicked
      //console.log(this);
    //console.log("junction clicked handler in junctionplots.js =)");
    //console.log(document.getElementById(boxPlotId));
    //console.log(ratioPlot1 + "  " + ratioPlot2);

    plotBoxPlot(junctionId,samples_junctions,boxPlotId);
    plotDistrubution(junctionId,samples_junctions,distrubutionId,"SupportNum");
    plotDistrubution(junctionId,samples_junctions,ratioPlot1,"SpliceRatioStartPos");
    plotDistrubution(junctionId,samples_junctions,ratioPlot2,"SpliceRatioEndPos");
}

//"result" is stored as global in ajaxQuery, which is the return result from the server
function plotBoxPlot(junctionId,samples_junctions,boxPlotId){

   //li = $("#" + boxPlotId).parent().parent().find("#a2").parent();
   

    var groups = samples_junctions[junctionId]; //groups object

    var data = new google.visualization.DataTable();
    data.addColumn('string','GroupName');
    data.addColumn("number","median");
    data.addColumn({id:'max', type:'number', role:'interval'});
    data.addColumn({id:'min', type:'number', role:'interval'});
    data.addColumn({id:'firstQuartile', type:'number', role:'interval'});
    data.addColumn({id:'median', type:'number', role:'interval'});
    data.addColumn({id:'thirdQuartile', type:'number', role:'interval'});

    var groupsData = [];

   // console.log("result group");
   // console.log(groups);
   // console.log("group length" + groups.length);


    var row = 0;

    for(var i = 0; i < groups.length; i++){ //get object, each group of object of array, size 1


      for(var groupName in groups[i]){ //should only be size 1
          var sinlgeGroupSupport = []; //new rows          
          var samplesSupport = groups[i][groupName];
          sinlgeGroupSupport.push(groupName);
           
          for(var j = 0; j < samplesSupport.length; j++){
              sinlgeGroupSupport.push(parseInt(samplesSupport[j].SupportNum)); //push back each support number
          }

          groupsData[row]=(sinlgeGroupSupport);
        
          row++;
      }

    }

    // console.log("groupDats");
    // console.log(groupsData);
     var finalArray = getBoxPlotValues(groupsData);
    // console.log("final array");
    // console.log(finalArray);
     data.addRows(finalArray);

     var options = {
          title:'Box Plot',
          width: 1000,
          height: 500,
          //height: 500,
          legend: {position: 'none'},
          hAxis: {
            gridlines: {color: '#fff'}
          },
          vAxis: {
             title:'Support Number'
          },
          lineWidth: 0,
          series: [{'color': '#D3362D'}],
          intervals: {
            barWidth: 1,
            boxWidth: 1,
            lineWidth: 2,
            style: 'boxes'
          },
          interval: {
            max: {
              style: 'bars',
              fillOpacity: 1,
              color: '#777'
            },
            min: {
              style: 'bars',
              fillOpacity: 1,
              color: '#777'
            }
          }
      };

       var chart = new google.visualization.LineChart(document.getElementById(boxPlotId));
       chart.draw(data, options);

   // console.log("dataTable");
   // console.log(groupsData);
}

function getBoxPlotValues(array){

        var newArray = [];
      //  console.log("length of array" + array.length);
        for (var i = 0; i < array.length; i++) {

          var arr = array[i].slice(1).sort(function (a, b) {
            return a - b;
          });

          var max = arr[arr.length - 1];
          var min = arr[0];
         // console.log("arr");
         // console.log(arr);
          var median = getMedian(arr);

          // First Quartile is the median from lowest to overall median.
          var firstQuartile = getMedian(arr.slice(0, Math.ceil(arr.length/2)));

          // Third Quartile is the median from the overall median to the highest.
          var thirdQuartile = getMedian(arr.slice(Math.floor(arr.length/2)));

          var tempArray = [];
          newArray.push(tempArray);
          newArray[i][0] = array[i][0];
          newArray[i][1] = median;
          newArray[i][2] = max;
          newArray[i][3] = min
          newArray[i][4] = firstQuartile;
          newArray[i][5] = median;
          newArray[i][6] = thirdQuartile;
        }

        return newArray;
}

 function getMedian(array) {
        var length = array.length;

        /* If the array is an even length the
         * median is the average of the two
         * middle-most values. Otherwise the
         * median is the middle-most value.
         */
        if (length % 2 === 0) {
          var midUpper = length / 2;
          var midLower = midUpper - 1;

          return (array[midUpper] + array[midLower]) / 2;
        } else {
          return array[Math.floor(length / 2)];
        }

}
