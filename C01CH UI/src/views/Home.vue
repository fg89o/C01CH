<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">DASHBOARD</h3>
    </v-col>
  </v-row>
  <v-row align="center" justify="center">
    <v-col cols="12" sm="10" lg="8"
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
      <v-row>
        <v-col>
          <div class="text-right">Ult. actualización: {{lastUpdate}}</div>
        </v-col>
      </v-row>
      <v-row>
        <v-col cols="6" md="4">
          <v-card >
            <v-card-text>
              <v-row >
                <v-col class="py-0 my-0">
                  <div class="overline">Voltaje</div>
                </v-col>
              </v-row>
              <v-row align="center">
                <v-col class="display-1 text--primary text-h5 text-md-primary py-0 my-0" cols="8">
                  {{volts}}
                </v-col>
                <v-col cols="4" class="py-0 my-0">
                  <v-list-item-icon>
                    <v-icon>mdi-current-dc</v-icon>
                  </v-list-item-icon>
                </v-col>
              </v-row>
              <v-row>
                <v-col class="py-0 my-0">
                 <p class="text-caption my-0 d-none d-md-flex">Máximo detectado: <span class="blue--text">{{volts_peak}}</span></p>
                  <p class="text-caption my-0 d-md-none">Valor máximo: <span class="blue--text">{{volts_peak}}</span></p>
                </v-col>
              </v-row>
            </v-card-text>
          </v-card>
        </v-col>
        <v-col cols="6" md="4">
          <v-card>
            <v-card-text>
              <v-row >
                <v-col class="py-0 my-0">
                  <div class="overline">Corriente</div>
                </v-col>
              </v-row>
              <v-row align="center">
                <v-col class="display-1 text--primary text-h5 text-md-primarypy-0 my-0" cols="8">
                  {{amps}}
                </v-col>
                <v-col cols="4" class="py-0 my-0">
                  <v-list-item-icon>
                    <v-icon>mdi-current-dc</v-icon>
                  </v-list-item-icon>
                </v-col>
              </v-row>
              <v-row>
                <v-col class="py-0 my-0">
                  <p class="text-caption my-0 d-none d-md-flex">Máximo detectado: <span class="blue--text">{{amps_peak}}</span></p>
                  <p class="text-caption my-0 d-md-none">Valor máximo: <span class="blue--text">{{amps_peak}}</span></p>
                </v-col>
              </v-row>
            </v-card-text>
          </v-card>
        </v-col>
        <v-col cols="12" md="4">
          <v-card >
            <v-card-text>
              <v-row >
                <v-col class="py-0 my-0">
                  <div class="overline">Potencia</div>
                </v-col>
              </v-row>
              <v-row align="center">
                <v-col class="display-1 text--primary text-h5 text-md-primary py-0 my-0" cols="8">
                  {{power}}
                </v-col>
                <v-col cols="4" class="py-0 my-0">
                  <v-list-item-icon>
                    <v-icon>mdi-flash-circle</v-icon>
                  </v-list-item-icon>
                </v-col>
              </v-row>
              <v-row>
                <v-col class="py-0 my-0">
                  <p class="text-caption my-0 d-none d-md-flex">Máximo detectado: <span class="blue--text">{{power_peak}}</span></p>
                  <p class="text-caption my-0 d-md-none">Valor máximo: <span class="blue--text">{{power_peak}}</span></p>
                </v-col>
              </v-row>
            </v-card-text>
          </v-card>
        </v-col>
        
      </v-row>

      <v-row>
        <v-col cols="12" md="4">
          <v-card>
            <v-card-text>
              <div class="overline">Funcionamiento</div>
            </v-card-text>
            <v-card-text v-if="modo=='Manual'" class="text-center">
              <div class="display-1 text--primary text-primary py-0 my-0">
                Manual
              </div>
            </v-card-text>
            <v-card-text v-if="modo=='Auto'" class="text-center py-0">
              <v-list>
                <v-list-item-content three-line>
                  <v-list-item-title>
                    <div class="display-1 text--primary text-primary py-0 my-0">Auto</div>
                  </v-list-item-title>
                  <v-list-item-subtitle>
                    <div class="text-caption">Siguiente cambio:</div>
                  </v-list-item-subtitle>
                  <v-list-item-subtitle>
                    <div class="text-caption">{{siguiente_hora}}H</div>
                  </v-list-item-subtitle>
                </v-list-item-content>
              </v-list>
            </v-card-text>
            <v-card-actions class="text-center">
              <v-btn class="mx-auto ma-2" outlined color="blue" v-on:click="changeMode()">Cambiar</v-btn>
            </v-card-actions>
          </v-card>
        </v-col>
        <v-col cols="12" md="8">
          <v-card>
            <v-card-text>
               <div class="text-overline">Estado de los canales</div>
            </v-card-text>
            <v-card-text>
              <v-list class="transparent">
                <v-list-item
                  v-for="(canal, i) in canales"
                  :key="i"
                >
                  <v-list-item-content three-line>
                    <v-list-item-title>
                      <div class="d-inline-block">Canal {{ i + 1 }}</div>
                      <div class="d-inline-block float-right text-right">{{ canal.potencia }} %</div>
                    </v-list-item-title>
                    <v-list-item-subtitle>
                    <v-progress-linear :value="canal.potencia"></v-progress-linear>
                    </v-list-item-subtitle>
                    <v-list-item-subtitle class="d-flex flex-row justify-space-between">
                      <div class="text-caption">{{canal.bus_volts.toFixed(1)}}V | {{(canal.bus_miliamps / 1000).toFixed(2)}}A | {{(canal.bus_miliamps / 1000 * canal.bus_volts).toFixed(0)}}W</div>
                      <div class="text-caption align-right">PWM: {{canal.pwm_value}}</div>
                    </v-list-item-subtitle>
                  </v-list-item-content>
                </v-list-item>
              </v-list>
            </v-card-text>
          </v-card>
        </v-col>
      </v-row>
    </v-col>
  </v-row>
</v-container>
</template>

<style scoped>
.pulse {

	border-radius: 50%;
	box-shadow: 0 0 0 0 rgba(0, 0, 0, 1);
	transform: scale(1);
	animation: pulse 2s infinite;
  
}

@keyframes pulse {
	0% {
		transform: scale(0.95);
		box-shadow: 0 0 0 0 rgba(0, 0, 0, 0.7);
	}

	70% {
		transform: scale(1);
		box-shadow: 0 0 0 10px rgba(0, 0, 0, 0);
	}

	100% {
		transform: scale(0.95);
		box-shadow: 0 0 0 0 rgba(0, 0, 0, 0);
	}
}
</style>

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
    volts_peak: "0",
    amps: "0A",
    amps_peak: "0",
    dac: 0,
    power: "0W",
    power_peak: "0",
    canales: [],
    lastUpdate: ""
  }),
  methods: {
    changeMode()
    {
      var self = this;

      self.error = false;

      var obj = {
        modo_programado: (self.modo != "Auto"),
        canales: self.canales
      }

      this.$http.post(process.env.VUE_APP_REMOTESERVER + 'canales', JSON.stringify(obj), {
        headers: {
            "Content-Type": 'text/plain'
        }
      }).then(function(/* response */)
      {
        self.success = true;
        self.requestCanales();
      }, function(){
          self.error = true;
      });
    },

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

      this.$http.get(process.env.VUE_APP_REMOTESERVER + 'canales').then(function(response){


        self.modo = response.body["modo_programado"] ? "Auto" : "Manual";
        self.siguiente_hora = response.body["modo_programado"] 
          ? response.body["siguiente_punto_hora"].toString().padStart(2,"0") + ":" + response.body["siguiente_punto_minuto"].toString().padStart(2,"0")
          : "__:__";

        self.volts = response.body["canales"][0]["bus_volts"].toFixed(1) + "V";
        self.amps = (response.body["canales"][0]["bus_miliamps"] / 1000).toFixed(2) + "A";
        self.amps = response.body["canales"][0]["bus_miliamps"] > 0 ? self.amps : "0.00A";
        self.power = ((response.body["canales"][0]["bus_miliamps"] / 1000) * response.body["canales"][0]["bus_volts"]).toFixed(0)+"W";
        self.power = response.body["canales"][0]["bus_miliamps"] > 0 ? self.power : "0W";

        self.volts_peak = response.body["canales"][0]["bus_volts_peak"].toFixed(1) + "V";
        self.amps_peak = (response.body["canales"][0]["bus_miliamps_peak"] / 1000).toFixed(2) + "A";
        self.power_peak = (response.body["canales"][0]["bus_power_peak"] / 1000).toFixed(0) + "W";

        self.canales = response.body["canales"];
        for (let i = 0; i < self.canales.length; i++)
        {
          var rango = self.canales[i].pwm_max - self.canales[i].pwm_min;
          var valor = self.canales[i].pwm_value - self.canales[i].pwm_min;

          self.canales[i].potencia =  Math.round(valor * 100 / rango);
        }

        var now = new Date();
        self.lastUpdate = now.getHours() + ":" + now.getMinutes().toString().padStart(2,"0") + ":" + now.getSeconds().toString().padStart(2,"0");

        //self.fillChartData( response.body["canales"] );

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

        this.$http.get(process.env.VUE_APP_REMOTESERVER + 'red').then(function(response){
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
    }, 10000);
  },
  beforeDestroy() {
    clearInterval(this.interval);
  }
}
</script>
