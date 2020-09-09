<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">VENTILACIÓN</h3>
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
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Ajuste > Ventilador</p>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Configuración actual del ventilador en base a la temperatura</h4>
      <v-divider></v-divider>
      <div v-if="loading">
        <v-skeleton-loader
          type="article"
          class="mx-auto"
      ></v-skeleton-loader>
      </div>
      <v-row>
        <v-col cols="12" sm="8" lg="6" class="d-flex justify-center offset-lg-1">
          <canvas id="fan-chart"></canvas>
        </v-col>
        <v-col cols="12" sm="4" lg="4">
          <v-form>
            <v-col cols="12" sm="6">
              <v-text-field
                  readonly
                  v-model="potencia"
                  label="Potencia actual"
              ></v-text-field>
            </v-col>
            <!-- <v-col cols="12" sm="6">
              <v-text-field
                  readonly
                  v-model="temperatura"
                  label="Temperatura"
              ></v-text-field>
            </v-col>
            <v-col cols="12" sm="6">
              <v-text-field
                  readonly
                  v-model="voltaje"
                  label="Voltaje"
              ></v-text-field>
            </v-col> -->
          </v-form>
        </v-col>
      </v-row>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Cambiar la configuración actual.</h4>
      <v-divider></v-divider>
      <v-card-text>
        <v-form>
            <v-row align="center">
                <v-col cols="8">
                    <v-input
                        :messages="['Comprueba la temperatura periódicamente y modifica la potencia del ventilador en función de esta']"
                    >
                        Control automático
                    </v-input>
                </v-col>
                <v-col cols="4" class="d-flex justify-end" >
                    <v-switch v-model="modo_automatico" flat inset @change="save()"></v-switch>
                </v-col>
            </v-row>
            <v-row class="my-6">
              <v-slider
                v-model="nuevo_porcentaje"
                track-color="grey"
                always-dirty
                min="0"
                max="100"
                thumb-label="always"
                @change="save()"
                v-if="!modo_automatico"
              >
                <template v-slot:prepend>
                  <v-icon
                    @click="decrement()"
                  >
                    mdi-minus
                  </v-icon>
                </template>
        
                <template v-slot:append>
                  <v-icon
                    @click="increment()"
                  >
                    mdi-plus
                  </v-icon>
                </template>
              </v-slider>
            </v-row>
            <v-row v-bind:class="{ 'd-none': !modo_automatico }">
              <v-col cols="12" sm="6">
                  <v-text-field
                      v-model="nuevo_max_p"
                      label="Potencia máxima:"
                      hint = "expresada en porcentaje"
                      persistent-hint
                      required
                  ></v-text-field>
              </v-col>
              <v-col cols="12" sm="6">
                  <v-text-field
                      v-model="nuevo_min_p"
                      label="Potencia mínima:"
                      hint = "expresada en porcentaje"
                      persistent-hint
                      required
                  ></v-text-field>
              </v-col>
              <v-col cols="12" sm="6">
                  <v-text-field
                      v-model="nuevo_max_t"
                      label="Valor del canal para 100%:"
                      hint = "expresado en porcentaje"
                      persistent-hint
                      required
                  ></v-text-field>
              </v-col>
              <v-col cols="12" sm="6">
                  <v-text-field
                      v-model="nuevo_min_t"
                      label="Valor de canal para 0%:"
                      hint = "expresado en porcentaje"
                      persistent-hint
                      required
                  ></v-text-field>
              </v-col>
              <v-col cols="12" class="d-flex justify-end px-4">
                <v-btn large color="primary" v-on:click="save()">Guardar</v-btn>
              </v-col>
            </v-row>
        </v-form>
      </v-card-text>
    </v-col>
  </v-row>
</v-container>
</template>
<script>
import Chart from 'chart.js';

export default {
  name: 'Adj_Fan',
  data: () => ({
      temperatura: 0,
      potencia: 0,
      voltaje: 0,
      loading: true,
      modo_automatico: true,
      nuevo_max_t: 0,
      nuevo_min_t: 0,
      nuevo_min_p: 0,
      nuevo_max_p: 0,
      nuevo_porcentaje: 0,
    }),
  methods: {
    createChart(chartId, chartData) {
      const ctx = document.getElementById(chartId);
      new Chart(ctx, {
        type: chartData.type,
        data: chartData.data,
        options: {
          title: {
            display: true,
            text: 'Potencia ventilador'
          },
          legend:{
            display: false
          },
          responsive: true,
          aspectRatio: 1,
          scales: {
            xAxes: [{
              scaleLabel: {
                display: true,
                labelString: "Canal"
              }
            }],
            yAxes: [{
              ticks: {
                beginAtZero: true,
                padding: 25,
              },
              scaleLabel: {
                display: true,
                labelString: "Ventilador"
              }
            }]
          }
        }
      });
    },
    request()
    {
      var self = this;
      this.$http.get(process.env.VUE_APP_REMOTESERVER + 'fansettings').then(function(response)
      {

        self.modo_automatico = response.body["enabled"];
        self.potencia = Math.round(response.body["curr_pwm"] * 100 / 1024 ) + "%";
        self.nuevo_porcentaje = response.body["curr_pwm"] * 100 / 1024;

        self.nuevo_max_t = response.body["max_channel_value"];
        self.nuevo_min_t = response.body["min_channel_value"];
        self.nuevo_max_p = Math.round(response.body["max_pwm"] * 100 / 1024 );
        self.nuevo_min_p = Math.round(response.body["min_pwm"] * 100 / 1024 );

        var obj = {
          type: 'line',
          data: {
            labels: [],
            datasets: [
              { // one line graph
                data: [],
                backgroundColor: 'rgba(179,212,252,0.3)',
                lineTension: 0,  
                borderWidth: 3,
                borderColor: [
                  '#1976d2',
                ],
              }
            ]
          },
        }

        // var valor_minimo = response.body["min_channel_value"] - 10;
        // var valor_maximo = response.body["max_temp"] + 10;
        var valor_minimo = 0
        var valor_maximo = 100
        for (var i = valor_minimo; i <= valor_maximo; i++)
        {
          obj.data.labels.push(i);
          var value = 0;
          if (response.body["enabled"])
          {
            if (i < response.body["min_channel_value"])
            {
              value = response.body["min_pwm"] * 100 / 1024 ;
            }
            else if ( i >= response.body["min_channel_value"] && i < response.body["max_channel_value"])
            {
              var rango = response.body["max_channel_value"] - response.body["min_channel_value"];
              var porcentaje = (i-response.body["min_channel_value"]) * 100 / rango / 100;

              var v = response.body["min_pwm"] + (response.body["max_pwm"] - response.body["min_pwm"]) * porcentaje;
              value = v * 100 / 1024 ;
            }
            else
            {
              value = response.body["max_pwm"] * 100 / 1024 ;
            }
          }
          else
          {
            value = response.body["curr_pwm"] * 100 / 1024;
          }
          obj.data.datasets[0].data.push(value);
        }

        this.createChart('fan-chart', obj);
        self.loading = false;

      }, function(){
          self.error = true;
          self.loading = false;
      });
    },
    save()
    {
      var self = this;

      var obj = {}
      if (self.modo_automatico)
      {
        obj = {
          enabled: self.modo_automatico,
          min_channel_value: self.nuevo_min_t,
          max_channel_value: self.nuevo_max_t,
          min_pwm: Math.round(1024 * self.nuevo_min_p / 100),
          max_pwm: Math.round(1024 * self.nuevo_max_p / 100)
        }
      }
      else
      {
        obj = {
          enabled: self.modo_automatico,
          curr_pwm: Math.round(1024 * self.nuevo_porcentaje / 100)
        }
      }

      this.$http.post(process.env.VUE_APP_REMOTESERVER + 'fansettings', JSON.stringify(obj), { headers: {"Content-Type": "text/plain"}}).then(function( /* response */)
      {
        self.request();
      }, function(){
          self.error = true;
          self.loading = false;
      });

    },
    decrement()
    {
      if (this.nuevo_porcentaje > 0)
      {
        this.nuevo_porcentaje--;
        save();
      }
    },
    increment()
    {
      if (this.nuevo_porcentaje < 100)
      {
        this.nuevo_porcentaje++;
        save();
      }
    }
  },
  created: function(){
    this.request();
  }
}
</script>
