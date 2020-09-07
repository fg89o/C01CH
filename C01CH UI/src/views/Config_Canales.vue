<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">CANALES</h3>
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
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Configuración > Canales</p>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Opciones relacionadas con la configuración de canales</h4>
      <v-divider></v-divider>
      <v-alert
          outlined
          type="success"
          text
          transition="scale-transition"
          :value="success"
          dismissible
          v-model="success"
      >
      Cambios realizados correctamente!
      </v-alert>
      <v-alert
          outlined
          type="error"
          text
          transition="scale-transition"
          :value="error"
          dismissible
          v-model="error"
      >
      Error!.No se recibió respuesta.
      </v-alert>
      <div v-if="loading">
        <v-skeleton-loader v-for="(sk, i) in 5" :key="i"
          type="list-item-two-line"
          class="mx-auto"
      ></v-skeleton-loader>
      </div>
      <v-container v-for="(item,i) in canales"
          :key="i">
        <v-row>
          <v-col cols="8">
            <v-input
                :messages="['Si el canal está deshabilitdo los leds estarán apagados y no se usará en la programación']"
            >
                Habilitar canal
            </v-input>
          </v-col>
          <v-col cols="4" class="d-flex justify-end" >
              <v-switch v-model="item.enabled" flat inset></v-switch>
          </v-col>
        </v-row>
        <v-row>
          <v-col cols="8">
            <v-input
                :messages="['Opción para resetear los valores máximos detectados de voltios, corriente y potencia']"
            >
                Valores máximos
            </v-input>
          </v-col>
          <v-col cols="4" class="d-flex justify-end" >
              <v-btn outlined color="primary" v-on:click="resetMax()">Reset</v-btn>
          </v-col>
        </v-row>
        <v-row>
          <v-col cols="6" sm="3">
            <v-text-field
                v-model="item.target_mA"
                label="mA objetivo actuales"
                readonly
                dense
            ></v-text-field>
          </v-col>
        </v-row>
        <v-row>
          <v-col cols="12">
            <v-text-field
                v-model="item.max_volts"
                label="Voltaje Máximo"
                hint = "Voltaje máximo al que se limitará el canal"
                type="number"
                suffix="V"
                persistent-hint
                required
            ></v-text-field>
          </v-col>
        </v-row>
        <v-row>
          <v-col cols="12">
            <v-text-field
                v-model="item.max_mA"
                label="Miliamperios Máximos"
                hint = "Miliamperios máximos al que se limitará el canal"
                type="number"
                suffix="mA"
                persistent-hint
                required
            ></v-text-field>
          </v-col>
        </v-row>
        <v-row>
          <v-col cols="12">
            <v-text-field
                v-model="item.min_mA"
                label="Miliamperios Mínimos"
                hint = "Miliamperios mínimos al que se limitará el canal"
                type="number"
                suffix="mA"
                persistent-hint
                required
            ></v-text-field>
          </v-col>
        </v-row>
        <v-row>
          <v-col cols="12" class="py-0">
            <v-row>
              <v-subheader>Config. Color</v-subheader>
              <v-spacer></v-spacer>
              <v-btn class="primary text">
                <v-icon
                  @click="nuevo(i)"
                >
                  mdi-plus
                </v-icon>
              </v-btn>
            </v-row>
            <v-row v-for="(led, j) in item.leds" :key="j">
              <v-col cols="4">
                <v-text-field
                  v-model="led.K"
                  label="Temperatura (K)"
                  dense
                ></v-text-field>
              </v-col>
              <v-col cols="3">
                <v-text-field
                  v-model="led.nm"
                  label="Long. de Onda (nm)"
                  dense
                ></v-text-field>
              </v-col>
              <v-col cols="3">
                <v-text-field
                  v-model="led.W"
                  label="Potencia (W)"
                  dense
                ></v-text-field>
              </v-col>
              <v-col cols="2">
                <v-btn outlined color="error" @click="borrar(i, j)">
                  <v-icon>
                    mdi-delete
                  </v-icon>
                </v-btn>
              </v-col>
            <v-divider></v-divider>
            </v-row>
          </v-col>
        </v-row>
      </v-container>
      <v-spacer></v-spacer>
      <v-btn large color="primary" v-on:click="save()">Guardar todo</v-btn>
    </v-col>
  </v-row>
</v-container>
</template>
<script>
export default {
  name: 'Config_Canales',
  data: () => ({
        canales: [],
        loading: true,
        success: false,
        error: false
    }),
    methods: {
      resetMax()
      {
        var self = this;
        var obj = {
          reset: true
        }
        this.$http.post(process.env.VUE_APP_REMOTESERVER + 'resetMaximos', JSON.stringify(obj), { headers: {"Content-Type": "text/plain"}})
        .then(function(/* response */){
          self.success = true;
        }, function(){
            self.error = true;
        });
      },
      borrar( num, led)
      {
        console.log(num + " " + led);
        this.canales[num].leds.splice(led,1);
      },
      nuevo( num )
      {
        var obj = {
          K: 0,
          nm: 0,
          W: 0
        };

        this.canales[num].leds.push(obj);
      },
      getInfo()
      {
        var self = this;

        this.$http.get(process.env.VUE_APP_REMOTESERVER + 'canales').then(function(response){
          self.canales = response.body["canales"];
          self.loading = false;
        }, function(){
            self.error = true;
            self.loading = false;
        });
      },
      save()
      {
        var self = this;
        self.success = false;
        self.error = false;

        for (var i = 0; i < self.canales.length; i++)
        {
          self.canales[i].max_mA = parseFloat(self.canales[i].max_mA);
          self.canales[i].min_mA = parseFloat(self.canales[i].min_mA);
          self.canales[i].max_volts = parseFloat(self.canales[i].max_volts);
        }

        var obj = {
          canales: self.canales
        }
        
        this.$http.post(process.env.VUE_APP_REMOTESERVER + 'canales', JSON.stringify(obj),{ headers: {"Content-Type": "text/plain"}}).then(function(/* response */){
          self.success = true;
        }, function(){
            self.error = true;
        });
      }
    },
    created: function(){
      this.getInfo();
    }
}
</script>
