<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">PROGRAMACIÓN | {{action}}</h3>
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
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Ajuste > Programación > Editar</p>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Editar punto de programación</h4>
      <v-divider></v-divider>
      <div v-if="loading">
        <v-skeleton-loader
          type="article"
          class="mx-auto"
      ></v-skeleton-loader>
      </div>
      <v-row>
        <v-col cols="12" lg="6">
          <canvas id="espectro-chart"></canvas>
        </v-col>
        <v-col cols="12" lg="6">
          <v-row>
            <v-col cols="12" md="4">
              <v-dialog
              ref="dialog"
              v-model="modal_time"
              :return-value.sync="time"
              persistent
              width="290px"
              >
                <template v-slot:activator="{ on }">
                    <v-text-field
                    v-model="time"
                    label="Hora"
                    readonly
                    v-on="on"
                    ></v-text-field>
                </template>
                <v-time-picker
                    v-if="modal_time"
                    v-model="time"
                    full-width
                >
                    <v-spacer></v-spacer>
                    <v-btn text color="primary" @click="modal_time = false">Cancel</v-btn>
                    <v-btn text color="primary" @click="$refs.dialog.save(time)">OK</v-btn>
                </v-time-picker>
              </v-dialog>
            </v-col>
            <v-col cols="12" md="7" class="offset-md-1">
              <v-switch v-model="schedulePoint.fade" label="Progresivo"></v-switch>
            </v-col>
          </v-row>
          <v-row>
            <v-col cols="12" md="6" lg="6" v-for="(canal, i) in canales" :key="i">
              <v-row>
                <v-col cols="12" class="d-flex justify-space-between">
                  <p class="mx-0 my-0" >Canal: {{i+1}}</p>
                  <span
                    class=""
                    v-text="porcentaje[i]+'%'"
                  ></span>
                </v-col>
                <v-col cols="12 py-0">
                  <v-slider
                    v-model="porcentaje[i]"
                    track-color="grey"
                    min="0"
                    max="100"
                    :disabled="!canal.enabled"
                    @change="updateChart()"
                    always-dirty
                  >
                    <template v-slot:prepend>
                      <v-icon
                      >
                        mdi-minus
                      </v-icon>
                    </template>
            
                    <template v-slot:append>
                      <v-icon
                      >
                        mdi-plus
                      </v-icon>
                    </template>
                  </v-slider>
                </v-col>
              </v-row>
            </v-col>
          </v-row>
          <v-divider class="py-4"></v-divider>
          <v-row class="px-3">
            <v-btn text color="error" class="mx-4" @click="goBack()">Cancelar</v-btn>
            <v-spacer></v-spacer>
            <v-btn text color="primary" class="mx-4" @click="sendTest()">Test</v-btn>
            <v-btn color="primary" @click="save()">Guardar</v-btn>
          </v-row>
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
  name: 'Adj_Programacion_editar',
  props: ['json'],
  data: () => ({
    loading: true,
    canales: [],
    schedulePoint: {},
    num_canales: 0,
    modal_time: false,
    time: '00:00',
    porcentaje: [],
    action: ""
  }),
  methods: {
    goBack()
    {
      this.$router.push({ name: "Programacion"});
    },
    save()
    {
      for(var i = 0; i < this.num_canales; i++)
      {
        this.schedulePoint["canal"+(i+1)] = this.porcentaje[i];
      }
      this.schedulePoint["fechaHora"] = this.time;

      var str = JSON.stringify(this.schedulePoint);
      this.$router.push({ name: "Programacion", params: { modifiedPoint: str }});
    },
    sendTest()
    {
      var tmpCanales = [];
      for(var i = 0; i < this.canales.length; i++)
      {
        var rango = this.canales[i].max_pwm - this.canales[i].min_pwm;
        var valor = this.canales[i].min_pwm + (rango * this.porcentaje[i] / 100);
        this.canales[i].current_pwm = valor;

        this.canales[i].porcentaje = this.porcentaje[i];
        tmpCanales.push(this.canales[i]);
      }

      var obj = {
        canales: tmpCanales
      }

      this.$http.post(this.$remoteServer + 'test', JSON.stringify(obj), {
        headers: {"Content-Type": "text/plain"}
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
    },
    request()
    {
      var self = this;
      
      self.porcentaje = [];

      this.$http.get(this.$remoteServer + 'canales').then(function(response)
      {
        self.canales = response.body["canales"];

        for (var i = 0; i < self.canales.length; i++)
        {
          self.canales[i].porcentaje = self.schedulePoint["canal"+(i+1)];
          self.porcentaje.push(self.schedulePoint["canal"+(i+1)]);
        }

        this.updateChart();
        
        self.loading = false;

      }, function(){
          self.error = true;
          self.loading = false;
      });
    },
    createChart(chartId, chartData) {
      const ctx = document.getElementById(chartId);
      new Chart(ctx, {
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
          responsive: true,
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
                display: true,
                labelString: ""
              }
            }]
          }
        }
      });
    },
    updateChart()
    {
      var blancos = [];
      var color = [];

      for (var i = 0; i < this.canales.length; i++)
      {
        for (var j = 0; j < this.canales[i].leds.length; j++)
        {
          if (this.canales[i].leds[j].K > 0)
          {
            blancos.push({
              temperatura: this.canales[i].leds[j].K,
              potencia: this.canales[i].leds[j].W * this.porcentaje[i] / 100
            });
          }
          else
          {
            color.push({
              nm: this.canales[i].leds[j].nm,
              potencia: this.canales[i].leds[j].W * this.porcentaje[i] / 100
            });
          }
        }
      }

      if (blancos.length > 0 || color.length > 0)
      {
        
        var canvas = document.getElementById("espectro-chart");
        const ctx = canvas.getContext("2d");
        var gradient = ctx.createLinearGradient(0, 0, canvas.parentElement.offsetWidth, 0);
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
        var obj = {
          type: 'line',
          data: {
            labels: [],
            datasets: [
              { // one line graph
                data: [],
                lineTension: 0.3,  
                backgroundColor: gradient,
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

        const p = this.polinomio(blancos,color,0);
        const f = parse(p);
        const simplified = simplify(f);
        for (var z = 0; z <= 10; z += 0.1)
        {
          obj.data.labels.push(z);
          var onda =simplified.evaluate({ x: z });
          obj.data.datasets[0].data.push(onda);
        }

        this.createChart('espectro-chart', obj);
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
  created: function(){

    var obj = JSON.parse(this.json);
    this.num_canales = obj["num_canales"];

    if (obj["schedulePoint"] != undefined)
    {
      this.schedulePoint = obj["schedulePoint"];
      this.time = this.schedulePoint["fechaHora"];
      this.action = "EDITAR";
    }
    else
    {
      this.action = "NUEVO";

      this.schedulePoint = {
        id: 0,
        fechaHora: "00:00",
        fade: true
      };

      for(var i = 0; i < this.num_canales; i++)
      {
        this.schedulePoint["canal"+(i+1)] = 0;
      }
      this.time = this.schedulePoint["fechaHora"];
    }

    this.request();
  }
}
</script>
