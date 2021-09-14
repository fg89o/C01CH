<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">CANALES</h3>
    </v-col>
  </v-row>
  <v-row align="center" justify="center">
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
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Ajuste > Canales</p>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Ajuste individual de cada canal</h4>
      <v-divider></v-divider>
      <div v-if="loading">
        <v-skeleton-loader
          type="article"
          class="mx-auto"
      ></v-skeleton-loader>
      </div>
      <v-alert
        border="bottom"
        colored-border
        type="warning"
        elevation="2"
        v-if="!loading && ledsCount == 0"
      >
      Rellene la información de los led para mostrar la gráfica del espectro
      <v-btn link to="/canales" class="float-right" text color="primary">Ir a configuración</v-btn>
      </v-alert>
      <v-row>
        <v-col :class="ledsCount == 0 ? 'd-none' : ''" cols="12" md="6" lg="4">
          <div class="chart-container" style="position: relative;">
            <canvas id="espectro-chart"></canvas>
        </div>
          <!-- <canvas style="max-height: 300px" id="espectro-chart"></canvas> -->
        </v-col>
        <v-col cols="12" md="6" class="py-0">
          <v-form v-if="!loading" class="mx-4">
            <v-row align="center">
              <v-col cols="12" class="d-flex justify-end py-0">
                <v-btn large text color="primary" class="caption" v-on:click="request()">Actualizar</v-btn>
              </v-col>
              <v-col cols="8">
                  <v-input
                      :messages="['Habilitar este modo deshabilitará el modo programado.']"
                      dense
                  >
                      Habilitar ajuste manual
                  </v-input>
              </v-col>
              <v-col cols="4" class="d-flex justify-end" >
                  <v-spacer></v-spacer>
                  <v-switch v-model="modo_manual" flat inset dense @change="send()"></v-switch>
              </v-col>
            </v-row>
            <v-row v-if="modo_manual">
              <v-col cols="12" v-for="(canal, i) in canales" :key="i">
                <v-row>
                  <v-col cols="12" class="d-flex justify-space-between">
                    <p class="mx-0 my-0" >Canal: {{i+1}}</p>
                    <span>
                      {{porcentaje[i]}}%
                    </span>
                  </v-col>
                  <v-col cols="12 py-0">
                    <v-slider
                      v-model="porcentaje[i]"
                      track-color="grey"
                      min="0"
                      max="100"
                      :disabled="!canal.enabled"
                      @change="send()"
                      always-dirty
                    >
                      <template v-slot:prepend>
                        <v-icon @click="decrement(i)">
                          mdi-minus
                        </v-icon>
                      </template>
              
                      <template v-slot:append>
                        <v-icon @click="increment(i)">
                          mdi-plus
                        </v-icon>
                      </template>
                    </v-slider>
                  </v-col>
                </v-row>
              </v-col>
            </v-row>
          </v-form>
        </v-col>
      </v-row>
    </v-col>
  </v-row>
</v-container>
</template>
<script>

import { simplify, parse } from "mathjs";
import Chart from "chart.js";

export default {
  name: 'Adj_Manual',
  data: () => ({
      canales: [],
      modo_manual: false,
      loading: true,
      porcentaje: {},
      ledsCount: 0,
      espectroChart: {}
  }),
  methods: {
    createChart(chartId, chartData) {
      const ctx = document.getElementById(chartId);
      this.espectroChart = new Chart(ctx, {
        type: chartData.type,
        data: chartData.data,
        options: {
          title: {
            display: true,
            text: 'Espectro'
          },
          legend:{
            display: false
          },
          responsive: false,
          aspectRatio: 1,
          scales: {
            xAxes: [{
              ticks: {
                callback: function(value) {
                    return parseFloat(value).toFixed(2);
                },
                maxTicksLimit: 10,
                display: false
              },
              gridLines: {
                display: false
              },
              scaleLabel: {
                display: true,
                labelString: "Longitud de onda"
              }
            }],
            yAxes: [{
              ticks: {
                beginAtZero: true,
                padding: 25,
                min: 0,
                stepSize: 1
              },
              scaleLabel: {
                display: false,
                labelString: ""
              }
            }]
          }
        }
      });
    },
    decrement(index)
    {
      if (this.porcentaje[index] > 0)
      {
        this.$set(this.porcentaje,index, this.porcentaje[index]-1);
        this.send();
      }
    },
    increment(index)
    {
      if (this.porcentaje[index] < 100)
      {
        this.$set(this.porcentaje,index, this.porcentaje[index]+1);
        this.send();
      }
    },
    send()
    {
      var self = this;

      self.success = false;
      self.error = false;

      var tmpCanales = [];
      for(var i = 0; i < self.canales.length; i++)
      {

        if (self.porcentaje[i] != self.canales[i].porcentaje)
        {
          var rango = self.canales[i].pwm_max - self.canales[i].pwm_min;
          var valor = self.canales[i].pwm_min + (rango * self.porcentaje[i] / 100);
          self.canales[i].pwm_value = valor;

          self.canales[i].porcentaje = self.porcentaje[i];
          tmpCanales.push(self.canales[i]);
        }
      }

      var obj = {
        modo_programado: !self.modo_manual,
        canales: tmpCanales
      }

      this.$http.post(process.env.VUE_APP_REMOTESERVER + 'canales', JSON.stringify(obj), {
        headers: {
            "Content-Type": 'text/plain'
        }
      }).then(function(/* response */)
      {
        self.success = true;
        setTimeout(()=>{
          self.success = false;
        }, 5000);
      }, function(){
          self.error = true;
          self.loading = false;
      });

      this.updateChart();
    },
    request()
    {
      var self = this;
      
      this.$http.get(process.env.VUE_APP_REMOTESERVER + 'canales').then(function(response)
      {
        self.modo_manual = !response.body["modo_programado"];
        self.canales = response.body["canales"];

        for (var i = 0; i < self.canales.length; i++)
        {
          var rango = self.canales[i].pwm_max - self.canales[i].pwm_min;
          var valor = self.canales[i].pwm_value - self.canales[i].pwm_min;

          self.canales[i].porcentaje = Math.round(valor * 100 / rango);

          this.$set(self.porcentaje,i,self.canales[i].porcentaje);
        }

        this.updateChart();

        self.loading = false;

      }, function(){
          self.error = true;
          self.loading = false;
      });
    },
    updateChart()
    {
      this.ledsCount = 0;
      var blancos = [];
      var color = [];

      var potencia_es_cero = true;
      for (var i = 0; i< this.canales.length; i++){
        if (this.canales[i].porcentaje > 0)
        {
          potencia_es_cero = false;
          break;
        }
      }

      for ( i = 0; i < this.canales.length; i++)
      {
        for (var j = 0; j < this.canales[i].leds.length; j++)
        {
          if (this.canales[i].leds[j].K > 0)
          {
            this.ledsCount++;
            blancos.push({
              temperatura: this.canales[i].leds[j].K,
              potencia: potencia_es_cero ? this.canales[i].leds[j].W : this.canales[i].leds[j].W * this.canales[i].porcentaje / 100
            });
          }
          else
          {
            this.ledsCount++;
            color.push({
              nm: this.canales[i].leds[j].nm,
              potencia: potencia_es_cero ? this.canales[i].leds[j].W : this.canales[i].leds[j].W * this.canales[i].porcentaje / 100
            });
          }
        }
      }

      if (blancos.length > 0 || color.length > 0)
      {
        this.espectroChart.data.datasets[0].data = [];
        this.espectroChart.data.labels = [];

        const p = this.polinomio(blancos,color,0);
        const f = parse(p);
        const simplified = simplify(f);
        for (var z = 0; z <= 10; z += 0.1)
        {
          this.espectroChart.data.labels.push(z);
          var onda =simplified.evaluate({ x: z });
          this.espectroChart.data.datasets[0].data.push(onda);
        }

        this.espectroChart.update();
      }
    },
    polinomio(blancos, color, fullespectro)
		{
      //Variables
			var w = 0;
			var i = 0;
			var k = ''; //si varia entre rangos
			var l = '';
			var n = 4.87298334620742;
			var p = ''; //(color[i].nm-400)/30; //solo aplica a color
			var m = '('; //son las que sumará al final

      //FullSpectrum
      if(fullespectro > 0)
      {
        m = m + '((' + fullespectro + '(0.408689018624721((3*e^(-(x-9.6)^2/7))+(8.2*e^(-(x-1.7)^2/0.3))+(7.7*e^(-(x-8.2)^2/2.5))+(1.1*e^(-(x-9.6)^2/0.06)+(2*e^(-(x-2)^2/0.8)))))';
        w += fullespectro;
      }
      
      //Blancos
      for(i=0; i < blancos.length; i++){
      
        if (m != '(') {
          m = m + '+';
        }

        if (blancos[i].temperatura < 3500){

          k = 8+(((2600-blancos[i].temperatura)/100 )* 0.125);
          l = 10.896322/(2.23606798+1.73205081 * k);
          m = m + '(' + blancos[i].potencia + '*(' + l + '*((5*e^(-(x-2)^2/0.2))+(' + k + '*e^(-(x-6.2)^2/3)))))';

        } else if (blancos[i].temperatura > 3500 && blancos[i].temperatura < 5000){

          k = 5+(((4000-blancos[i].temperatura)/100 )* ((100*0.7)/300));
          l = 10.896322/(2.23606798+1.73205081 * k);
          m = m + '(' + blancos[i].potencia + '*(' + l + '*((5*e^(-(x-2)^2/0.2))+(' + k + '*e^(-(x-5.7)^2/3)))))';

        } else if (blancos[i].temperatura > 5000 && blancos[i].temperatura < 6500){	

          k = 2.5+(((5500-blancos[i].temperatura)/100 )* 0.125);
          l = 10.896322/(2.23606798+1.73205081 * k);
          m = m + '(' + blancos[i].potencia + '*(' + l + '*((5*e^(-(x-2)^2/0.2))+(' + k + '*e^(-(x-5.2)^2/3)))))';

        } else {

          k = 2.1+(((6500-blancos[i].temperatura)/100 )*(100*0.9)/8500);
          l = 10.896322/(2.23606798+1.73205081 * k);
          m = m + '(' + blancos[i].potencia + '*(' + l + '*((5*e^(-(x-2)^2/0.2))+(' + k + '*e^(-(x-5.2)^2/3)))))';
        }

        w += blancos[i].potencia;
        
      }

      //Color
      for(i=0; i < color.length; i++){
        p = (color[i].nm-400)/30;
        if(m == '('){
          m = m + '(' + color[i].potencia + '*(' + n + '*(5*e^(-(x- ' + p + ')^2/0.2))))';
        } else {
          m = m + '+' + '(' + color[i].potencia + '*(' + n + '*(5*e^(-(x- ' + p + ')^2/0.2))))';
        }
        
        w += color[i].potencia;
      }
      
      return m + ')/' + w ;
    }
  }, 
  mounted: function(){

    var canvas = document.getElementById("espectro-chart");
    const ctx = canvas.getContext("2d");
    var obj = {
      type: 'line',
      data: {
        labels: [],
        datasets: [
          { 
            data: [],
            lineTension: 0.3,  
            //backgroundColor: gradient,
            borderWidth: 1,
            borderColor: [
              'black',
            ],
            pointBackgroundColor: 'rgba(0,0,0,0)',
            pointBorderColor: 'rgba(0,0,0,0)',
            pointBorderWidth: 0
          }
        ]
      },
    }
    this.createChart('espectro-chart', obj);

    let width = parseInt(canvas.style.width, 10)
    if (isNaN(width))
    {
      width = 0;
    }

    var gradient = ctx.createLinearGradient(0, 0, width, 0);
    gradient.addColorStop(0, 'black');
    gradient.addColorStop(.1, 'blue');
    gradient.addColorStop(.3, 'blue');
    gradient.addColorStop(.4, 'rgb(0,206,209)');
    gradient.addColorStop(.45, 'rgb(0,255,0)');
    gradient.addColorStop(.6, 'rgb(0,255,0)');
    gradient.addColorStop(.7, 'yellow');
    gradient.addColorStop(.75, 'orange');
    gradient.addColorStop(.85, 'red');
    gradient.addColorStop(1, 'black');

    this.espectroChart.data.datasets[0]["backgroundColor"] = gradient;

    this.request();
  }
}
</script>
