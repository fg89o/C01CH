<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">DASHBOARD</h3>
    </v-col>
  </v-row>
  <v-row align="center" justify="center">
    <v-col cols="12" md="10" >
      <v-row 
          align="start"
          justify="space-between">
        <v-col cols="12" sm="6" lg="7"
          style="
            padding-top: 0px;
            margin-top: -50px;
            background-color: white;
            border-radius: 5px;
            border: 1px solid #aaa;
            margin-bottom: 50px;
          "
        >
          <v-row>
            <v-col cols="3" sm="4" md="4" lg="3" class="d-none d-md-block">
              <v-icon class="display-4">mdi-flash-circle</v-icon>
            </v-col>
            <v-col cols="12" sm="8" md="8" lg="9">
              <v-row>
                <v-col cols="12" lg="4">
                  <div class="d-flex flex-column px-4">
                    <span class="subheading font-weight-light mr-1">Voltaje:</span>
                    <span
                      class="display-1 font-weight-bold"
                      v-text="volts"
                    ></span>
                  </div>
                </v-col>
                <v-col cols="12" lg="4">
                  <div class="d-flex flex-column px-4">
                    <span class="subheading font-weight-light mr-1">Amperios:</span>
                    <span
                      class="display-1 font-weight-bold"
                      v-text="amps"
                    ></span>
                  </div>
                </v-col>
                <v-col cols="12" lg="4">
                  <div class="d-flex flex-column px-4">
                    <span class="subheading font-weight-light mr-1">Potencia:</span>
                    <span
                      class="display-1 font-weight-bold"
                      v-text="power"
                    ></span>
                  </div>
                </v-col>
              </v-row>
            </v-col>
          </v-row>
        </v-col>
        <v-col cols="12" sm="5" lg="4" class="d-fex flex-row"
          style="
            padding-top: 0px;
            margin-top: -50px;
            background-color: white;
            border-radius: 5px;
            border: 1px solid #aaa;
            margin-bottom: 50px;
          "
        >
        <v-row>
          <v-col cols="6">
            <v-icon class="display-4">mdi-wifi</v-icon>
          </v-col>
          <v-col cols="6" class="d-flex flex-column">
            <span class="subheading font-weight-light mr-1">Modo:</span>
            <span
              class="display-2 font-weight-bold"
              v-text="modo_wifi"
            ></span>
            <span
              class="subheading font-weight-light"
              v-text="potencia_wifi"
            ></span>
          </v-col>
        </v-row>
        </v-col>
      </v-row>
  </v-col>
  </v-row>
  <v-row align="center" justify="center">
    <v-col cols="12" md="10" lg="10"
      style="
        padding-top: 0px;
        margin-top: -50px;
        background-color: white;
        border-radius: 5px;
        border: 1px solid #aaa;
        margin-bottom: 50px;
      "
    >
      <v-row>
        <v-col cols="12" md="8">
          <canvas id="canales-chart"></canvas>
        </v-col>
        <v-col md="4" cols="12">
          <v-row>
            <v-col offset-sm="2" offset-md="0" cols="4" md="12">
              <div class="d-flex flex-column">
                <span class="subheading font-weight-light mr-1">Modo:</span>
                <span
                  class="title font-weight-bold"
                  v-text="modo"
                ></span>
              </div>
            </v-col>
            <v-col cols="4" md="12">
              <v-row>
                <v-col cols="6" md="12">
                  <span class="subheading font-weight-light mr-1">Siguiente cambio:</span>
                </v-col>
                <v-col cols="6" md="12">
                  <span
                    class="display-2 font-weight-light py-4"
                    v-text="siguiente_hora"
                  ></span>
                </v-col>
              </v-row>
            </v-col>
          </v-row>
        </v-col>
      </v-row>
    </v-col>
  </v-row>
  
</v-container>
</template>

<script>
import Chart from 'chart.js';
export default {
  name: 'Home',
  data: () => ({
    modo: "",
    siguiente_hora: "",
    modo_wifi: "",
    potencia_wifi: "",
    interval:null,
    volts: "0V",
    amps: "0A",
    dac: 0,
    power: "0W"
  }),
  methods: {
    createChart(chartId, chartData) {
      const ctx = document.getElementById(chartId);
      new Chart(ctx, {
        type: "horizontalBar",
        data: chartData.data,
        options: {
          title: {
            display: true,
            text: 'Estado de canales'
          },
          legend:{
            display: false
          },
          responsive: true,
          scales: {
            xAxes: [{
              ticks: {
                beginAtZero: true,
                min: 0,
                max: 100
              },
              scaleLabel: {
                display: true,
                labelString: "% Potencia"
              }
            }],
            // yAxes: [{
            //   ticks: {
            //     beginAtZero: true,
            //     padding: 25,
            //   },
            //   scaleLabel: {
            //     display: true,
            //     labelString: "Potencia %"
            //   }
            // }]
          }
        }
      });
    },
    requestCanales()
    {
      var self = this;

      this.$http.get(this.$remoteServer + 'canales').then(function(response){

        self.modo = response.body["modo_programado"] ? "Auto" : "Manual";
        self.siguiente_hora = response.body["modo_programado"] 
          ? response.body["siguiente_punto_hora"].toString().padStart(2,"0") + ":" + response.body["siguiente_punto_minuto"].toString().padStart(2,"0")
          : "__:__";
        self.volts = response.body["canales"][0]["bus_volts"].toFixed(2) + "V";
        self.amps = (response.body["canales"][0]["bus_miliamps"] / 1000).toFixed(3) + "A";
        self.power = ((response.body["canales"][0]["bus_miliamps"] / 1000) * response.body["canales"][0]["bus_volts"]).toFixed(2)+"W";

        self.fillChartData( response.body["canales"] );

      }, function(){
          self.error = true;
      });
    },
    fillChartData( canales )
    {
      var obj = {
        data: {
          labels: [],
          datasets: [{
            data: [],
            backgroundColor: "#b3d4fc"
          }]
        },
      }

      for (let i = 0; i < canales.length; i++)
      {
        var rango = canales[i].max_pwm - canales[i].min_pwm;
        var valor = canales[i].current_pwm - canales[i].min_pwm;

        let nombre = "Canal #" + (i + 1);
        obj.data.labels.push(nombre);
        obj.data.datasets[0].data.push(Math.round(valor * 100 / rango));
      }

      this.createChart("canales-chart", obj)
    },
    requestNetwork()
    {
        var self = this;

        this.$http.get(this.$remoteServer + 'red').then(function(response){
            self.modo_wifi = response.body["mode"];
            self.potencia_wifi = response.body["rssi"] + "dbm"; 
        }, function(){

            self.error = true;
        });
    },
    initialize()
    {
      this.requestCanales();
      this.requestNetwork();
    }
  },
  created(){
    this.initialize();

    this.interval = setInterval(() => {
      this.initialize();
    }, 60000);
  },
  beforeDestroy() {
    clearInterval(this.interval);
  }
}
</script>
