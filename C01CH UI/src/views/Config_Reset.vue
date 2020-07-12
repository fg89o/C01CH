<template>
<v-container style="max-width: 100%">
  <v-row dark style="background-color: rgb(51,51,51); margin-top: -12px; padding-bottom: 60px">
    <v-col cols="12"
    >
      <h3 dark style="color: white">RESET</h3>
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
      <p class="my-4 px-4 text-uppercase font-weight-light overline">Configuración > Reset</p>
      <v-divider></v-divider>
      <h4 class="my-2 px-4">Reinicia el equipo y carga la configuración guardada</h4>
      <v-divider></v-divider>
      <v-alert
          outlined
          type="success"
          text
          transition="scale-transition"
          :value="success"
          v-model="success"
      >
      Equipo en proceso de reinicio. Espere unos segundo y actualice la página.
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
      Error al procesar el reinicio. No se recibió respuesta.
      </v-alert>
      <v-alert v-if="!success"
          outlined
          type="warning"
          text
          transition="scale-transition"
      >
      Al reiniciar el equipo se cargará la configuración guardada en memoria. Todos los cambios no guardados se perderán. ¿ Está seguro que desea reiniciar el equipo?
      </v-alert>
      <div class="d-flex justify-center">
        <v-btn v-if="!success" large color="error" v-on:click="reset()">Reiniciar ahora</v-btn>
      </div>
    </v-col>
  </v-row>
</v-container>
</template>
<script>
export default {
  name: 'Config_Reset',
  data: () => ({
      success: false,
      error: false
    }),
    methods: {
      reset()
      {
        var self = this;
        var obj = {
          reset: true
        }
        this.$http.post(this.$remoteServer + 'reset', JSON.stringify(obj), { headers: {"Content-Type": "text/plain"}})
        .then(function(/* response */){
          self.success = true;
        }, function(){
            self.error = true;
            self.loading = false;
        });
      }
    }
}
</script>
