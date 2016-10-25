var junctionDataTable;
var junctionTableChart;
var exonDataTable;
var exonTableChart;

function getDistubutionTable(groups,property){


  var dataTable = new google.visualization.DataTable();
  dataTable.addColumn("string","Sample");
  dataTable.addColumn("number",property);



  for(var groupName in groups){ //for each group //should only be one

     
     for(var i = 0; i < groups[groupName].length; i++){
       dataTable.addRow([  groups[groupName][i].SampleName,  parseInt( groups[groupName][i][property]) ]);
    }
  
  }

  return dataTable;
}

function plotDistrubution(junctionId,samples_junctions,distrubutionId,property){ //this is called in junctionPlots.js

    var groups = samples_junctions[junctionId][0]; //groups object result is global
    
   distubutionTable = getDistubutionTable(groups,property); //only 1 group

    
    var options = {
          title: property + ' Distrubution Among Samples',
          curveType: 'function',
          width: 1000,
          height: 500,
          legend: { position: 'bottom' }
    };

    var chart = new google.visualization.LineChart(document.getElementById(distrubutionId));

    chart.draw(distubutionTable, options);

}

function getExonDataTable(data){


   var dataTable = new google.visualization.DataTable();
   dataTable.addColumn('string', 'Chromosome');
   dataTable.addColumn('number', 'StartPos');
   dataTable.addColumn('number', 'EndPos');
   dataTable.addColumn('string', 'Strand');

   for(var i = 0; i < data.length; i++){

          dataTable.addRow([data[i].Chr,data[i].StartPos,data[i].EndPos,data[i].Strand]);

   }

   return dataTable;


}


function getJunctionDataTable(data){


   var dataTable = new google.visualization.DataTable();
   dataTable.addColumn('string', 'Chromosome');
   dataTable.addColumn('number', 'StartPos');
   dataTable.addColumn('number', 'EndPos');

   for(var i = 0; i < data.length; i++){

          dataTable.addRow([data[i].Chr,parseInt(data[i].StartPos),parseInt(data[i].EndPos)]);
          dataTable.JunctionId = data[i].JunctionId;

   }

   return dataTable;


}

function createExonTable(exons){

   exonDataTable = getExonDataTable(exons);
   exonTableChart= new google.visualization.Table(document.getElementById('ExonTable'));
   exonTableChart.draw(exonDataTable ,{showRowNumber: true, width: '100%', height: '100%'});
  

}

function selectJunctionHandler(){

      var selection = junctionTableChart.getSelection();
      
      if(selection.length != 1)
          return;

     // console.log(selection[0].row);
      var junctionId = result.junctions[selection[0].row].JunctionId;

      plotBoxPlot(junctionId);
}

function createJunctionTable(junctions){

   junctionDataTable = getJunctionDataTable(junctions);
   junctionTableChart = new google.visualization.Table(document.getElementById('JunctionTable'));
   junctionTableChart.draw(junctionDataTable,{showRowNumber: true, width: '100%', height: '100%'});

   // Every time the table fires the "select" event, it should call your
   // selectHandler() function.
   google.visualization.events.addListener(junctionTableChart, 'select', selectJunctionHandler);

}
