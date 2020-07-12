<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">PROGRAMACIÓN</h3>
    </v-col>
  </v-row>
  <v-row align="center" justify="center" >
    <v-col cols="12" md="10" lg="10"
      style="
        padding-top: 0px;
        margin-top: -50px;
        background-color: white;
        border-radius: 5px;
        border: 1px solid transparent;
        border-image: linear-gradient(to bottom, rgb(51,51,51) 0%, rgba(255,255,255,0) 50%);
        border-image-slice: 1;
      "
    >
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Ajuste > Programación</p>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Ajuste de la programación</h4>
      <v-divider></v-divider>
      <div v-if="loading">
        <v-skeleton-loader
          type="article"
          class="mx-auto"
      ></v-skeleton-loader>
      </div>
      <v-row>
        <v-col cols="12" md="8" class="offset-md-2">
          <canvas id="prog-chart"></canvas>
        </v-col>
      </v-row>
      <v-row>
        <v-col cols="12">
          <v-data-table
            :headers="headers"
            :items="items"
          >
            <template v-slot:top>
              <v-toolbar flat color="white">
                <v-toolbar-title>Programación</v-toolbar-title>
                <v-spacer></v-spacer>
                <v-btn color="primary" dark class="mb-2" @click="newItem()">Añadir</v-btn>
              </v-toolbar>
            </template>
            <template v-slot:item.actions="{ item }">
              <v-icon
                small
                class="mr-2"
                @click="editItem(item)"
                color="primary"
              >
                mdi-pencil
              </v-icon>
              <v-icon
                small
                @click="deleteItem(item)"
                color="error"
              >
                mdi-delete
              </v-icon>
            </template>
            <template v-slot:item.fade="{ item }">
              <v-simple-checkbox readonly disable v-model="item.fade"></v-simple-checkbox>
            </template>
          </v-data-table>
        </v-col>
      </v-row>
    </v-col>
  </v-row>
</v-container>
</template>
<script>
import Chart from "chart.js";

export default {
  props: ["modifiedPoint"],
  name: 'Adj_Programacion',
  data: () => ({
    loading: true,
    headers: [],
    items: [],
    dialog: false,
    editedItemReceived: false,
    editedItem: {},
    num_canales: 0,
  }),
  watch: {
    dialog (val) {
      val || this.close()
    },
  },

  methods: {
    createChart(chartId, chartData)
    {
      const ctx = document.getElementById(chartId);
      new Chart(ctx, {
        type: chartData.type,
        data: chartData.data,
        options: {
          title: {
            display: true,
            text: 'Programacion'
          },
          legend:{
            display: true
          },
          responsive: true,
          scales: {
            xAxes: [{
              type: 'time',
              time: {
                  parser: 'HH:mm',
                  unit: 'hour',
                  unitStepSize: 1,
                  
              },
              ticks:{
                min: "00:00", //calculated above in my implementation
                max: "23:59"  //same as above
              }
            }],
            yAxes: [{
              ticks: {
                beginAtZero: true,
                padding: 25,
                min: 0,
              },
              scaleLabel: {
                display: false,
                labelString: "Porcentaje %"
              }
            }]
          }
        }
      });
    },
    updateChart()
    {
      var colors = [
        "rgb(244, 67, 54, 0.7)",
        "rgb(233, 30, 99, 0.7)",
        "rgb(156, 39, 176, 0.7)",
        "rgb(103, 58, 183, 0.7)",
        "rgb(63, 81, 181, 0.7)",
        "rgb(76, 175, 80, 0.7)",
        "rgb(255, 152, 0, 0.7)",
        "rgb(0, 150, 136, 0.7)"
      ];

      var obj = {
        type: 'line',
        data: {
          labels: [],
          datasets: []
        },
      }

      for(let i = 0; i < this.num_canales; i++)
      {
        obj.data.datasets.push({
          label: "#"+(i+1),
          data: [],
          lineTension: 0,
          borderColor: colors[i],
          backgroundColor: "transparent"
        });
      }

      var now = new Date(
        Date.now()
      );

      var initTime = new Date(
        now.getFullYear(),
        now.getMonth(),
        now.getDate(), 
        23, 59, 0, 0
      );


      var lastIndex = -1;
      for(let i = 0; i < 1380; i++)
      {
        initTime.setSeconds(initTime.getSeconds() + 60);
        var hora = initTime.getHours().toString().padStart(2,"0") + ":" + initTime.getMinutes().toString().padStart(2,"0");
        for(let j = 0; j < this.items.length; j++)
        {
          if (this.items[j].fechaHora == hora)
          {
            lastIndex = j;

            for(let z = 0; z < this.num_canales; z++)
            {
              obj.data.datasets[z].data.push({
                x: hora,
                y: this.items[j]["canal"+(z+1)]
              });
            }
          }
        }
      }

      // Primer y ultimo indice para completar el grafico
      if (obj.data.datasets[0].data.length > 0)
      {
        for(let i = 0; i < this.num_canales; i++)
        {
          if (obj.data.datasets[0].data[0].x != "00:00")
          {
            console.log("añadido primer punto");
            obj.data.datasets[i].data.splice(0, 0, {
              x: "00:00",
              y: this.items[lastIndex]["canal"+(i+1)]
            });
          }
          
          if (obj.data.datasets[0].data[obj.data.datasets[0].data.length - 1].x != "23:59")
          {
            console.log("añadido ultimo punto");
            obj.data.datasets[i].data.push({
              x: "23:59",
              y: this.items[lastIndex]["canal"+(i+1)]
            });
          }
        }
      }

      this.createChart("prog-chart", obj);

    },
    saveChanges()
    {

      var self = this;
      var obj = [];

      for(var i = 0; i < this.items.length; i++)
      {
        obj.push({
          hour: this.items[i].fechaHora.split(":")[0],
          minute: this.items[i].fechaHora.split(":")[1],
          fade: this.items[i].fade,
          values: []
        });

        for(var j = 0; j < this.num_canales; j++)
        {
          obj[i].values.push(this.items[i]["canal"+(j+1)]);
        }
      }

      this.$http.post(this.$remoteServer + 'schedule', JSON.stringify(obj), {
        headers: {
            "Content-Type": 'text/plain'
        }
      }).then(function(/* response */)
      {

      }, function(){
        self.error = true;
        self.loading = false;
      });
    },
    request(){
      var self = this;
      
      this.$http.get(this.$remoteServer + 'schedule').then(function(response)
      {
        self.items = [];
        self.headers = [];

        self.headers.push({ text: 'Nº', value: 'id' });
        self.headers.push({
            text: 'Fecha / Hora',
            align: 'start',
            sortable: false,
            value: 'fechaHora',
        }),
        self.headers.push({ text: 'Progresivo', value: 'fade', sortable: false });
        var i;
        for ( i = 0; i < response.body["channel_size"]; i++)
        {
          self.headers.push({ text: '#' + (i+1), value: 'canal'  + (i+1), sortable: false});
        }
        self.headers.push({ text: 'Acciones', value: 'actions', sortable: false });

        self.num_canales = response.body["channel_size"];

        for( i = 0; i < response.body["schedule"].length; i++)
        {
          var obj = {
            id: (i+1),
            fechaHora: response.body["schedule"][i].hour.toString().padStart(2,"0") + ":" + response.body["schedule"][i].minute.toString().padStart(2,"0"),
            fade: response.body["schedule"][i].fade,
          }

          for (var j = 0; j < response.body["schedule"][i].values.length; j++)
          {
            obj["canal"+(j+1)] = response.body["schedule"][i].values[j];
          }

          self.items.push(obj);
        }

        if (self.editedItemReceived)
        {
          self.CheckModifiedValue();
        }
        
        self.updateChart();

        self.loading = false;

      }, function(){
          self.error = true;
          self.loading = false;
      });
    },
    CheckModifiedValue()
    {
      if (this.editedItemReceived)
      {
        var i;
        var point = JSON.parse(this.modifiedPoint);
        if (point.id == 0)
        {
          for(i = 0; i < this.items.length; i++)
          {
            if (this.items[i].id >= point.id)
            {
              point.id = (this.items[i].id + 1);
            }
          }

          this.items.push(point);
        }
        else
        {
          for (i = 0; i < this.items.length; i++)
          {
            if (this.items[i].id == point.id)
            {
              this.items[i] = point;
            }
          }
        }

        this.saveChanges();
      }
    },
    editItem( item )
    {
      var obj = {
        num_canales: this.num_canales,
        schedulePoint: item
      }

      var str = JSON.stringify(obj);
      this.$router.push({ name: "prog_editar", params: { json: str }});
    },
    deleteItem (item) {
      const index = this.items.indexOf(item)
      if (confirm('¿Seguro que desea eliminar este elemento?')){
        this.items.splice(index, 1);
        this.saveChanges();
      }
    },
    newItem()
    {
      var obj = {
        num_canales: this.num_canales
      }

      var str = JSON.stringify(obj);
      this.$router.push({ name: "prog_editar", params: { json: str }});
    },
    close () {
      this.dialog = false
      setTimeout(() => {
        this.editedItem = Object.assign({}, this.defaultItem)
        this.editedIndex = -1
      }, 300)
    },
  }, 
  created: function()
  {
    if (this.modifiedPoint !== undefined)
    {
      this.editedItem = JSON.parse(this.modifiedPoint);
      this.editedItemReceived = true;
    }

    this.request();
  }
}
</script>
